#include "BC_PCH.h"
#include "Jobs.h"

namespace BC
{
    void JobCounter::Increment(uint32_t value)
    {
        m_Count.fetch_add(value);
    }
    
    void JobCounter::Decrement() 
    {
        if (m_Count.fetch_sub(1) == 1)
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            m_Condition.notify_all();
        }
    }

    void JobCounter::Wait()
    {
        std::unique_lock<std::mutex> lock(m_Mutex);
        m_Condition.wait(lock, [&]() { return m_Count.load() == 0; });
    }
}