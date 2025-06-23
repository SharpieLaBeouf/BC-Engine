#include "BC_PCH.h"
#include "Time.h"

namespace BC
{
	Time* Time::s_Instance = nullptr;

	Time::Time() 
	{
		m_LastTimeClock = std::chrono::high_resolution_clock::now();
	}

	void Time::Init()
	{
		BC_ASSERT(!s_Instance, "Time Already Initialised!");
		s_Instance = new Time();
	}

	void Time::Shutdown()
	{
		delete s_Instance;
		s_Instance = nullptr;
	}

	void Time::UpdateTime() 
	{
		BC_ASSERT(s_Instance, "Time::GetUnscaledDeltaTime: Time Not Initialised!");
        if (!s_Instance)
            return;

		s_Instance->m_CurrentTimeClock = std::chrono::high_resolution_clock::now();

		s_Instance->m_CurrentTime = std::chrono::duration_cast<std::chrono::duration<double>>(s_Instance->m_CurrentTimeClock.time_since_epoch()).count();
		s_Instance->m_DeltaTime = std::chrono::duration_cast<std::chrono::duration<double>>(s_Instance->m_CurrentTimeClock - s_Instance->m_LastTimeClock).count();

		s_Instance->m_LastTimeClock = s_Instance->m_CurrentTimeClock;

		s_Instance->m_FrameRateTimer -= static_cast<float>(s_Instance->m_DeltaTime);
		if (s_Instance->m_FrameRateTimer <= 0.0f) {
			s_Instance->m_FrameRateEstimate = static_cast<uint16_t>(1.0f / static_cast<float>(s_Instance->m_DeltaTime));
			s_Instance->m_FrameRateTimer = 1.0f;
		}
	}

	float Time::GetUnscaledDeltaTime() 
	{ 
		BC_ASSERT(s_Instance, "Time::GetUnscaledDeltaTime: Time Not Initialised!");
        if (!s_Instance)
            return 0.0f;

		return static_cast<float>(s_Instance->m_DeltaTime); 
	}

	float Time::GetDeltaTime() 
	{
		BC_ASSERT(s_Instance, "Time::GetDeltaTime: Time Not Initialised!");
        if (!s_Instance)
            return 0.0f;

		return static_cast<float>(s_Instance->m_DeltaTime) * s_Instance->m_TimeScale; 
	}

	float Time::GetUnscaledFixedDeltaTime() 
	{ 
		BC_ASSERT(s_Instance, "Time::GetUnscaledFixedDeltaTime: Time Not Initialised!");
        if (!s_Instance)
            return 0.0f;

		return s_Instance->m_FixedDeltaTime; 
	}

	float Time::GetFixedDeltaTime() 
	{
		BC_ASSERT(s_Instance, "Time::GetFixedDeltaTime: Time Not Initialised!");
        if (!s_Instance)
            return 0.0f;
			
		return s_Instance->m_FixedDeltaTime * s_Instance->m_TimeScale; 
	}

	void Time::SetFixedDeltaTime(const float& fixedDeltaTime) 
	{
		BC_ASSERT(s_Instance, "Time::SetFixedDeltaTime: Time Not Initialised!");
        if (!s_Instance)
            return;

		BC_CORE_TRACE("Fixed Update Frequency Interval Changed To: {0} seconds", fixedDeltaTime);
		s_Instance->m_FixedDeltaTime = fixedDeltaTime;
	}

	uint16_t Time::GetUnscaledFixedUpdatesHz() 
	{
		BC_ASSERT(s_Instance, "Time::GetUnscaledFixedUpdatesHz: Time Not Initialised!");
        if (!s_Instance)
            return -1;

		return static_cast<uint16_t>(1.0f / s_Instance->m_FixedDeltaTime); 
	}

	uint16_t Time::GetFixedUpdatesHz()
	{
		BC_ASSERT(s_Instance, "Time::GetFixedUpdatesHz: Time Not Initialised!");
        if (!s_Instance)
            return -1;

		return static_cast<uint16_t>(1.0f / s_Instance->m_FixedDeltaTime * s_Instance->m_TimeScale); 
	}

	float Time::GetTimeScale()
	{
		BC_ASSERT(s_Instance, "Time::GetTimeScale: Time Not Initialised!");
        if (!s_Instance)
            return 1.0f;

		return s_Instance->m_TimeScale; 
	}

	void Time::SetTimeScale(const float& timeScale)
	{
		BC_ASSERT(s_Instance, "Time::SetTimeScale: Time Not Initialised!");
        if (!s_Instance)
            return;

		s_Instance->m_TimeScale = timeScale;
	}

	// TODO: Average this from last two seconds of runtime for smoother and more accurate FPS
	uint16_t Time::GetFrameRate()
	{
		BC_ASSERT(s_Instance, "Time::GetFrameRate: Time Not Initialised!");
        if (!s_Instance)
            return 0;

		return s_Instance->m_FrameRateEstimate;
	}

    double Time::GetCurrTime()
	{
		BC_ASSERT(s_Instance, "Time::GetCurrTime: Time Not Initialised!");
        if (!s_Instance)
            return 0.0;

 		return s_Instance->m_CurrentTime; 
	}
}