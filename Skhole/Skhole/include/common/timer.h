#pragma once

#include <include.h>

namespace Skhole 
{
	class Timer 
	{
		Timer(){};
		~Timer(){};

		void Start()
		{
			m_StartTime = std::chrono::high_resolution_clock::now();
		}

		void End() {
			m_EndTime = std::chrono::high_resolution_clock::now();
		}

		float GetElapsedTime() {
			std::chrono::duration<float> duration = m_EndTime - m_StartTime;
			return duration.count();
		}

		float Stop() {
			End();
			return GetElapsedTime();
		}

	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTime;
		std::chrono::time_point<std::chrono::high_resolution_clock> m_EndTime;
	};

};
