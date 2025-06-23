#pragma once

// Core Headers

// C++ Standard Library Headers
#include <cstdint>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <chrono>
#include <vector>
#include <string>

// External Vendor Library Headers

namespace BC
{

    enum class JobPriority : uint8_t
    {
        Low = 0,
        Medium = 1,
        High = 2 
    };

    using JobFunction = std::function<void()>;

    class JobCounter
    {

    public:

        JobCounter() = default;
        ~JobCounter() = default;
        
        JobCounter(const JobCounter& other) = delete;
        JobCounter(JobCounter&& other)
        {
            m_Count.store(other.m_Count.load()); other.m_Count.store(0);
        }
        
        JobCounter& operator=(const JobCounter& other) = delete;
        JobCounter& operator=(JobCounter&& other)
        {
            if (this == &other)
                return *this;

            m_Count.store(other.m_Count.load()); other.m_Count.store(0);
            
            return *this;
        }

        void Increment(uint32_t value = 1);
        void Decrement();
        void Wait();

        uint32_t GetCount() const { return m_Count.load(); }

    private:

        std::atomic<uint32_t> m_Count = 0;
        std::mutex m_Mutex;
        std::condition_variable m_Condition;

    };
    
    struct JobProfileEvent
    {
        std::string     Name;
        double          StartTime;
        double          EndTime;
        JobPriority     Priority;
    };

    struct FrameProfile
    {
        std::chrono::high_resolution_clock::time_point frame_start;
        double frame_duration; // in milliseconds
        std::vector<std::vector<JobProfileEvent>> thread_events;
    };

    struct Job
    {
        JobFunction Func;
        JobCounter* Counter = nullptr;
        JobPriority Priority = JobPriority::Medium;
        bool IsPersistent = false;
        std::string Name = "";

        Job() = default;
        Job(const std::string& n, const JobFunction& f, JobCounter* c, JobPriority p, bool pers)
            : Name(n), Func(f), Counter(c), Priority(p), IsPersistent(pers) { }
    };

}