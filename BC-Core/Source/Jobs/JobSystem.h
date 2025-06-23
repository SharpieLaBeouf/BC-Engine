#pragma once

// Core Headers
#include "Jobs.h"

// C++ Standard Library Headers

// External Vendor Library Headers

namespace BC
{

    class JobSystem
    {

    public:

		JobSystem() = default;
        ~JobSystem() { if (m_Running.load()) { Shutdown(); } }

		JobSystem(const JobSystem&) = delete;
		JobSystem(JobSystem&&) = default;

		JobSystem& operator=(const JobSystem&) = delete;
		JobSystem& operator=(JobSystem&&) = default;

        /// @brief Initialise the Job System
        void Init();

        /// @brief Shutdown the Job System
        void Shutdown();

        /// @brief Submit a job to the global thread pool queue to be actioned by a worker thread
        /// @param name The name of the job
        /// @param func The function of the job, typically a lambda
        /// @param counter (Optional) The job counter associated with the particular job. Will increment by 1 and will decrement when complete
        /// @param priority The priority of the job. Highest jobs are prioritised over lower jobs
        /// @param persistent If the job is persistent across frames. If false, it will be finished on the frame it was dispatched, if true, it will run across multiple frames
        void SubmitJob(const std::string& name, const JobFunction& func, JobCounter* counter, JobPriority priority, bool persistent = false);
        
        /// @brief Submit a vector of job's to the global thread pool queue to be actioned by a worker thread
        /// @param jobs A vector holding pairs of job name's and the corresponding job function
        /// @param counter (Optional) The job counter associated with the job's submitted. E.g., counter will increment by N jobs submitted
        /// @param priority The priority of the job's. Highest jobs are prioritised over lower jobs
        /// @param persistent If the job's are persistent across frames. If false, it will be finished at the end of the current frame prior to proceeding with the next frame If true, it will run across multiple frames
        void SubmitJobs(const std::vector<std::pair<std::string, JobFunction>>& jobs, JobCounter* counter, JobPriority priority, bool persistent = false);

        /// @brief Helper called at end of Application::Run loop to wait for all
        /// non persistent jobs to complete in the current frame
        void FinishJobs();

        /// @brief This will be called at the start of the Application::Run loop
        /// to begin a new performance profile for non-persistent jobs, and will
        /// retain persistent jobs
        void BeginFrameProfile();

        /// @brief This will be called at the end of the Application::Run loop
        /// after JobSystem::FinishJobs to ensure frame profiles are caught for
        /// all jobs and stored into the frame profile deque
        void EndFrameProfile();

        /// @brief This will return the frame profile for a particular frame ago. This is clamped between 1->(MAX frame profiles to be stored at any time)
        /// @param how_many_frames_ago 
        FrameProfile GetProfileResults(int how_many_frames_ago = 1);

        uint8_t GetWorkerCount() const { return std::thread::hardware_concurrency() - 2; }
        
	private:
        
        struct Worker
        {
            /// @brief The worker thread instance
            std::thread Thread;

            /// @brief Local queue of a worker thread.
            std::deque<Job> LocalQueue;
        };

        /// @brief Vector of Worker Threads running
        std::vector<std::unique_ptr<Worker>> m_Workers;

        /// @brief Global job queue based on priority, 0 == low, 1 == medium, 2 == high, etc.
        std::array<std::deque<Job>, 3> m_GlobalQueues;

        /// @brief Mutex for Global job queue to ensure safe adding submission
        /// of jobs into global queue
        std::mutex m_GlobalQueueMutex;

        /// @brief Mutex for worker threads to check if a job is available
        std::mutex m_QueueMutex;

        /// @brief Condition variable to alert workers that a job is available
        /// to be taken
        std::condition_variable m_JobAvailable;

        /// @brief Atomic bool to flag whether the system is to continue
        /// running, or if worker threads should shutdown
        std::atomic<bool> m_Running = true;

        uint32_t m_NumWorkerThreads = -1;

        /// @brief The implementation of the worker thread that will run
        /// continuously and will take and execute jobs on the fly
        void WorkerThreadFunction(size_t index);

        /// @brief Helper function for a worker thread to get a job from the
        /// global queue
        bool GetJob(Job& out_job, Worker& context);
        
        /// @brief Helper function for a worker thread to steal a job from
        /// another worker thread queue, e.g., if thread queues are piling up,
        /// other threads can steal jobs to balance workload
        bool StealJob(size_t thief_index, Job& out_job);

        /// @brief Helper function to check if the global queue has jobs
        /// available to be taken  
        bool HasJobs();

        /// @brief Mutex to ensure thread safety when adding to the frame
        /// profiles
        std::mutex m_ProfileMutex;

        /// @brief A double ended queue to hold frame profiles of this and
        /// previous frames
        std::deque<FrameProfile> m_FrameProfiles;

        /// @brief A place holder for the current frame profile statistics to be
        /// copied into m_FrameProfiles once the frame is finalised
        FrameProfile m_CurrentFrameProfile;

        /// @brief Atomic bool to check if performance profiles should be taken
        /// or not
        std::atomic<bool> m_ProfilingActive{ false };

    };

}