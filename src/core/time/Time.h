#pragma once

#include <chrono>
#include <algorithm>

#if Z_EDITOR
namespace zzz::editor
{
	class EditorEngine;
}
#endif

namespace zzz::engine
{
	class Engine;
}

namespace zzz::core
{
	class Time
	{
	public:
		Time();
		~Time() = default;

		/// @brief Возвращает время в секундах, прошедшее с предыдущего кадра,
		/// умноженное на масштаб времени (TimeScale).
		float GetDeltaTime() const noexcept { return m_DeltaTime; }

		/// @brief Возвращает реальное время в секундах с предыдущего кадра
		/// (без учета масштаба времени).
		float GetUnscaledDeltaTime() const noexcept { return m_UnscaledDeltaTime; }

		/// @brief Возвращает реальное время в секундах, прошедшее со старта приложения.
		float GetTimeSinceStartup() const noexcept { return m_TimeSinceStartup; }

		float GetTimeScale() const noexcept { return m_TimeScale; }
		void SetTimeScale(float scale) noexcept { m_TimeScale = (std::max)(scale, 0.0f); }

	private:
		friend class zzz::engine::Engine;
#if Z_EDITOR
		friend class zzz::editor::EditorEngine;
#endif
		void Update();
		void ResetFrameTimer() { m_LastFrameTime = std::chrono::high_resolution_clock::now(); }

		std::chrono::high_resolution_clock::time_point m_LastFrameTime;

		float m_DeltaTime = 0.0f;
		float m_UnscaledDeltaTime = 0.0f;
		float m_TimeSinceStartup = 0.0f;
		float m_TimeScale = 1.0f;
	};
}
