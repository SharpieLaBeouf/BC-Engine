#pragma once

#include "Debug/Logging.h"
#include "Util/Platform.h"

#if defined(BC_PLATFORM_WINDOWS)
    #include <windows.h>
#elif defined(BC_PLATFORM_LINUX)
    #include <pthread.h>
    #include <sched.h>
#endif

namespace BC::Util
{
    static void SetThreadCoreAffinity(size_t index)
    {

    #if defined(BC_PLATFORM_WINDOWS)

        DWORD_PTR mask = 1ull << index;
        SetThreadAffinityMask(GetCurrentThread(), mask);

    #elif defined(BC_PLATFORM_LINUX)

        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(index, &cpuset);
        pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
        
    #else

        BC_CORE_WARN("SetThreadCore: Cannot Set Thread Core Affinity on Hardware.");

    #endif

        return;
    }

    static int GetThreadCoreIndex()
    {

    #if defined(BC_PLATFORM_WINDOWS)

        DWORD_PTR affinity = SetThreadAffinityMask(GetCurrentThread(), ~0ull); // temporarily set to all
        SetThreadAffinityMask(GetCurrentThread(), affinity); // restore
        for (int i = 0; i < 64; ++i)
        {
            if ((affinity >> i) & 1ull)
                return i;
        }

    #elif defined(BC_PLATFORM_LINUX)

        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        if (pthread_getaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) == 0)
        {
            for (int i = 0; i < CPU_SETSIZE; ++i)
            {
                if (CPU_ISSET(i, &cpuset))
                    return i;
            }
        }

    #else

        BC_CORE_WARN("GetThreadCoreIndex: Platform not supported.");

    #endif

        return -1;
    }
}