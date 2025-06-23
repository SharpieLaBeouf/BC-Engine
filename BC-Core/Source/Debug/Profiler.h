#pragma once

// Core Headers

// C++ Standard Library Headers
#include <map>
#include <memory>
#include <chrono>

// External Vendor Library Headers

namespace BC {

	struct ProfileResult 
    {
		std::string Name;
		float Time = 0.0f;
		bool Accumulative = false;
	};

	class Profiler 
    {

	public:

		static Profiler& Get() 
        {
			static Profiler s_Instance;
			return s_Instance;
		}

		std::map<std::string, ProfileResult>& GetResults()
        {
            return m_Results;
        }

		void AddResult(const ProfileResult& result) 
        {
            m_Results[result.Name] = result; 
        }

		void AddAccumResult(const ProfileResult& result) 
        { 
			if(m_Results.count(result.Name) == 0)
				m_Results[result.Name] = result;
			else
				m_Results[result.Name].Time += result.Time; 
		}

		void NewFrame() 
		{
			for (auto it = m_Results.begin(); it != m_Results.end(); ++it) 
			{
				if (it->second.Accumulative) 
				{
					it->second.Time = 0.0f;
				}
			}
		}

		// Delete copy assignment and move assignment constructors
		Profiler(const Profiler&) = delete;
		Profiler(Profiler&&) = delete;

		// Delete copy assignment and move assignment operators
		Profiler& operator=(const Profiler&) = delete;
		Profiler& operator=(Profiler&&) = delete;

	private:

		Profiler() { }

		std::map<std::string, ProfileResult> m_Results;
	};

	class ProfileTimer 
    {

	public:

		ProfileTimer(const std::string& name, bool accumulative = false) : 
            m_Name(name), 
            m_Stopped(false), 
            m_Accumulative(accumulative),
            m_TimerStart(std::chrono::high_resolution_clock::now()) { }
		
        ~ProfileTimer() 
        {
			if (!m_Stopped)
				StopTimer();
		}

		void StopTimer() 
        {
			auto currentTime = std::chrono::high_resolution_clock::now();

			long long timerStart = std::chrono::time_point_cast<std::chrono::microseconds>(m_TimerStart).time_since_epoch().count();
			long long timerEnd = std::chrono::time_point_cast<std::chrono::microseconds>(currentTime).time_since_epoch().count();
			
			m_Stopped = true;

			float elapsedTime = (timerEnd - timerStart) * 0.001f;
			if(!m_Accumulative)
				Profiler::Get().AddResult({ m_Name, elapsedTime });
			else
				Profiler::Get().AddAccumResult({ m_Name, elapsedTime, true });
		}

	private:

		std::string m_Name;
		std::chrono::time_point<std::chrono::high_resolution_clock> m_TimerStart;
		bool m_Stopped;
		bool m_Accumulative;
	};

}

// Determine the Function Signature Macro depending on the compiler. Sourced and modified slightly from boost/current_function.hpp.
#if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
	# define BC_CURRENT_FUNCTION __PRETTY_FUNCTION__
#elif defined(__DMC__) && (__DMC__ >= 0x810)
	# define BC_CURRENT_FUNCTION __PRETTY_FUNCTION__
#elif defined(__FUNCSIG__) || (_MSC_VER)
	# define BC_CURRENT_FUNCTION __FUNCSIG__
#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
	# define BC_CURRENT_FUNCTION __FUNCTION__
#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
	# define BC_CURRENT_FUNCTION __FUNC__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
	# define BC_CURRENT_FUNCTION __func__
#elif defined(__cplusplus) && (__cplusplus >= 201103)
	# define BC_CURRENT_FUNCTION __func__
#else
	# define BC_CURRENT_FUNCTION "Unknown Compiler"
#endif

#define BC_PROFILE_SCOPE(name) ::BC::ProfileTimer time##__LINE__(name)
#define BC_PROFILE_SCOPE_ACCUMULATIVE(name) ::BC::ProfileTimer time##__LINE__(name, true)
#define BC_PROFILE_FUNCTION() BC_PROFILE_SCOPE(BC_CURRENT_FUNCTION)
