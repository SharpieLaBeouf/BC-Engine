#pragma once

// Core Headers
#include "Debug/Logging.h"

// C++ Standard Library Headers
#include <chrono>

// External Vendor Library Headers

namespace BC
{

	class Time {

	public:

		static void Init();
		static void Shutdown();

		static void UpdateTime();

		static float GetUnscaledDeltaTime();
		static float GetDeltaTime();

		static float GetUnscaledFixedDeltaTime();
		static float GetFixedDeltaTime();

		static void SetFixedDeltaTime(float fixedDeltaTime);

		static uint16_t GetUnscaledFixedUpdatesHz();
		static uint16_t GetFixedUpdatesHz();

		static float GetTimeScale();
		static void SetTimeScale(float timeScale);
		
		static uint16_t GetFrameRate();

		static double GetCurrTime();

	private:

		double m_CurrentTime	{ 0.0 };
		double m_DeltaTime		{ 0.0 };
		float m_FixedDeltaTime	{ 1.0f / 60.0f };
		float m_TimeScale		{ 1.0f };

		uint16_t m_FrameRateEstimate	{ 0 };
		float m_FrameRateTimer	{ 1.0f };

		std::chrono::time_point<std::chrono::high_resolution_clock> m_LastTimeClock;
		std::chrono::time_point<std::chrono::high_resolution_clock> m_CurrentTimeClock;
		
		static Time* s_Instance;
		
	private:

		Time();

		// Delete copy assignment and move assignment constructors
		Time(const Time&) = delete;
		Time(Time&&) = delete;

		// Delete copy assignment and move assignment operators
		Time& operator=(const Time&) = delete;
		Time& operator=(Time&&) = delete;

	};

}