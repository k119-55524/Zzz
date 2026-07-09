
#include <algorithm>

#include "public/core/EngineTime.h"

namespace zzz::engine
{
	Time::Time()
	{
		m_LastFrameTime = std::chrono::high_resolution_clock::now();
	}

	void Time::Update()
	{
		auto currentFrameTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> duration = currentFrameTime - m_LastFrameTime;
		m_LastFrameTime = currentFrameTime;

		float unscaledDelta = duration.count();
			
		// Ограничение максимальной дельты для предотвращения скачков после "зависаний"
		unscaledDelta = std::min(unscaledDelta, 0.1f);

		m_UnscaledDeltaTime = unscaledDelta;
		m_DeltaTime = unscaledDelta * m_TimeScale;
		m_TimeSinceStartup += unscaledDelta;
	}
}
