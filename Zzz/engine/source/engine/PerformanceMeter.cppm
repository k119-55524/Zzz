#include "pch.h"
export module PerformanceMeter;

export namespace zzz
{
	/// @brief Класс для измерения времени выполнения кода с использованием высокоточного таймера.
	/// @details PerformanceMeter предоставляет функционал для замера времени выполнения
	/// участков кода. Он фиксирует время начала и вычисляет прошедшее время в секундах
	/// при остановке измерения. Использует std::chrono::high_resolution_clock для высокой точности.
	export class PerformanceMeter
	{
		using Clock = std::chrono::high_resolution_clock;

	public:
		/// @brief Конструктор по умолчанию.
		/// @details Инициализирует объект с текущим временем в качестве начальной точки.
		PerformanceMeter();

		/// @brief Запускает измерение времени.
		/// @details Фиксирует текущее время как начало измерения.
		void StartPerformance();

		/// @brief Останавливает измерение времени и возвращает прошедшее время.
		/// @return Время выполнения в секундах (тип double).
		double StopPerformance();

	private:
		std::chrono::time_point<Clock> m_TimeStart; ///< Точка времени начала измерения.
	};

	PerformanceMeter::PerformanceMeter() :
		m_TimeStart{ Clock::now() }
	{
	}

	void PerformanceMeter::StartPerformance()
	{
		m_TimeStart = Clock::now();
	}

	double PerformanceMeter::StopPerformance()
	{
		auto now = Clock::now();
		double deltaTime = std::chrono::duration<double>(now - m_TimeStart).count();
		return deltaTime;
	}
}