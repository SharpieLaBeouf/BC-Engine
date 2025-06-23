#include "BC_PCH.h"
#include "JobSystem.h"
#include "Util/ThreadUtil.h"

namespace BC
{

#pragma region Initialisation and General Helpers

    void JobSystem::Init()
    {
        // Lock main thread to core 0
        Util::SetThreadCoreAffinity(0);

        // Determine Worker Threads
        const uint32_t hardware_threads = std::thread::hardware_concurrency();
        m_NumWorkerThreads = hardware_threads - 2; // One dedicated for main thread, one dedicated for render thread

        BC_THROW(m_NumWorkerThreads > 0 , "JobSystem::Init: Insufficient Threads, CPU Not Supported.");

        // Resize Worker Thread Vector
        m_Workers.reserve(m_NumWorkerThreads);

        // Setup Frame Profiling
        m_CurrentFrameProfile.thread_events.resize(m_NumWorkerThreads);
        for (auto& events : m_CurrentFrameProfile.thread_events)
            events.reserve(128);

        // Spawn Worker Threads
        for (size_t i = 0; i < m_NumWorkerThreads; ++i)
        {
            m_Workers.push_back(std::make_unique<Worker>());
            m_Workers.back()->Thread = std::thread([this, i]() 
                { 
                    Util::SetThreadCoreAffinity(i + 2); 
                    WorkerThreadFunction(i); 
                }
            );
        }
    }

    void JobSystem::Shutdown()
    {
        m_Running = false;
        m_JobAvailable.notify_all();

        for (auto& worker : m_Workers)
        {
            if (worker && worker->Thread.joinable())
                worker->Thread.join();
        }
    }

    void JobSystem::SubmitJob
    (
        const std::string& name, 
        const JobFunction& func, 
        JobCounter* counter, 
        JobPriority priority, 
        bool persistent
    )
    {
        SubmitJobs({ {name, func} }, counter, priority, persistent);        
    }

    void JobSystem::SubmitJobs
    (
        const std::vector<std::pair<std::string, JobFunction>>& jobs, 
        JobCounter* counter, 
        JobPriority priority, 
        bool persistent
    )
    {
        {
            std::lock_guard<std::mutex> lock(m_GlobalQueueMutex);
            for (const auto& pair : jobs)
            {
                const auto& job_name = pair.first;
                const auto& func = pair.second;
                if (counter) counter->Increment();
                m_GlobalQueues[static_cast<int>(priority)].emplace_back(job_name, func, counter, priority, persistent);
            }
        }
        m_JobAvailable.notify_all();        
    }

    void JobSystem::FinishJobs()
    {
        while (true)
        {
            bool any_frame_jobs_remaining = false;

            {
                std::lock_guard<std::mutex> lock(m_GlobalQueueMutex);
                for (const auto& queue : m_GlobalQueues)
                {
                    for (const auto& job : queue)
                    {
                        if (!job.IsPersistent)
                        {
                            any_frame_jobs_remaining = true;
                            break;
                        }
                    }
                    if (any_frame_jobs_remaining) break;
                }
            }

            if (!any_frame_jobs_remaining)
            {
                for (const auto& context : m_Workers)
                {
                    if (!context) continue;

                    for (const auto& job : context->LocalQueue)
                    {
                        if (!job.IsPersistent)
                        {
                            any_frame_jobs_remaining = true;
                            break;
                        }
                    }
                    if (any_frame_jobs_remaining) break;
                }
            }

            if (!any_frame_jobs_remaining)
                break;

            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    }

    void JobSystem::BeginFrameProfile()
    {
        std::lock_guard<std::mutex> lock(m_ProfileMutex);
        m_ProfilingActive = true;
        m_CurrentFrameProfile.frame_start = std::chrono::high_resolution_clock::now();

        m_CurrentFrameProfile.thread_events.resize(m_NumWorkerThreads);
        for (auto& events : m_CurrentFrameProfile.thread_events)
        {
            events.clear();
            events.reserve(128);
        }
    }

    void JobSystem::EndFrameProfile()
    {
        auto frame_end = std::chrono::high_resolution_clock::now();
        {
            std::lock_guard<std::mutex> lock(m_ProfileMutex);
            m_ProfilingActive = false;
            m_CurrentFrameProfile.frame_duration = std::chrono::duration<double, std::milli>(frame_end - m_CurrentFrameProfile.frame_start).count();
            m_FrameProfiles.push_back(m_CurrentFrameProfile);

            if (m_FrameProfiles.size() > 100)
                m_FrameProfiles.pop_front();
        }
    }
    
    FrameProfile JobSystem::GetProfileResults(int how_many_frames_ago)
    {
        std::lock_guard<std::mutex> lock(m_ProfileMutex);
        if (m_FrameProfiles.empty())
            return FrameProfile();

        int index = static_cast<int>(m_FrameProfiles.size()) - how_many_frames_ago;
        if (index < 0)
            index = 0;

        return m_FrameProfiles[index];
    }

    bool JobSystem::GetJob(Job &out_job, Worker &context)
    {
        if (!context.LocalQueue.empty())
        {
            out_job = context.LocalQueue.back();
            context.LocalQueue.pop_back();
            return true;
        }

        std::lock_guard<std::mutex> lock(m_GlobalQueueMutex);
        for (int p = 0; p < 3; ++p)
        {
            auto& queue = m_GlobalQueues[p];
            if (!queue.empty())
            {
                out_job = queue.front();
                queue.pop_front();
                return true;
            }
        }
        return false;
    }

    bool JobSystem::StealJob(size_t thief_index, Job &out_job)
    {
        for (size_t i = 0; i < m_Workers.size(); ++i)
        {
            if (i == thief_index || !m_Workers[i])
                continue;

            auto& victim = *m_Workers[i];
            if (!victim.LocalQueue.empty())
            {
                out_job = victim.LocalQueue.front();
                victim.LocalQueue.pop_front();
                return true;
            }
        }
        return false;
    }

    bool JobSystem::HasJobs()
    {
        std::lock_guard<std::mutex> lock(m_GlobalQueueMutex);
        for (const auto& q : m_GlobalQueues)
        {
            if (!q.empty())
                return true;
        }
        return false;
    }

#pragma endregion

#pragma region Worker Thread Function

    void JobSystem::WorkerThreadFunction(size_t index)
    {
        auto& context = m_Workers[index];

        while (m_Running)
        {
            Job job;
            bool success = false;

            {
                std::unique_lock<std::mutex> lock(m_QueueMutex);
                m_JobAvailable.wait(lock, [this]() {
                    return !m_Running || HasJobs();
                });

                if (!m_Running)
                    break;

                success = GetJob(job, *context.get());
            }

            if (!success)
                success = StealJob(index, job);

            if (success)
            {
                auto job_start_time = std::chrono::high_resolution_clock::now();
                job.Func();
                auto job_end_time = std::chrono::high_resolution_clock::now();

                if (m_ProfilingActive)
                {
                    double start_ms = std::chrono::duration<double, std::milli>(job_start_time - m_CurrentFrameProfile.frame_start).count();
                    double end_ms = std::chrono::duration<double, std::milli>(job_end_time - m_CurrentFrameProfile.frame_start).count();

                    JobProfileEvent event{ job.Name, start_ms, end_ms, job.Priority };
                    m_CurrentFrameProfile.thread_events[index].push_back(event);
                }

                if (job.Counter)
                    job.Counter->Decrement();
            }
        }
    }

#pragma endregion

}