
export module AppTime;

export namespace zzz
{
	/**
	 * @brief Класс для управления временем приложения.
	 *
	 * Класс `AppTime` предназначен для отслеживания общего времени работы приложения,
	 * и время между последовательными кадрами (дельта-время), с учетом пауз. 
	 * Класс является потоконебезопасным так как используется только основным потоком.
	 * Не допускает копирования или перемещения.
	 */
	export class AppTime final
	{
		using Clock = std::chrono::steady_clock;

	public:
		/**
		 * @brief Конструктор по умолчанию.
		 *
		 * Инициализирует объект `AppTime`, устанавливая начальное время запуска
		 * приложения и предыдущего тика, а также сбрасывая флаг паузы и время паузы.
		 */
		AppTime();

		/**
		 * @brief Конструктор копирования (запрещен).
		 */
		AppTime(AppTime&) = delete;

		/**
		 * @brief Конструктор перемещения (запрещен).
		 */
		AppTime(AppTime&&) = delete;

		/**
		 * @brief Деструктор по умолчанию.
		 *
		 * Освобождает ресурсы автоматически, так как класс не использует динамическую память.
		 */
		~AppTime() = default;

		/**
		 * @brief Получает общее время работы приложения.
		 *
		 * Возвращает время в секундах с момента создания объекта `AppTime`.
		 *
		 * @return double Общее время работы приложения в секундах. Возвращает 0.0, если время отрицательное.
		 */
		double GetAllAppTime() const;

		/**
		 * @brief Получает время между последовательными вызовами (дельта-время).
		 *
		 * Возвращает время в секундах между текущим вызовом и предыдущим вызовом метода,
		 * исключая время, проведенное в паузе. Обновляет время предыдущего тика.
		 *
		 * @return double Время между кадрами в секундах. Возвращает 0.0, если время отрицательное.
		 */
		double GetDeltaTime();

		/**
		 * @brief Устанавливает или снимает паузу.
		 *
		 * Включает или выключает режим паузы, накапливая время паузы для коррекции
		 * дельта-времени в `GetDeltaTime`.
		 *
		 * @param _isPause Если `true`, включает паузу; если `false`, выключает паузу.
		 */
		void Pause(bool _isPause);

		/**
		 * @brief Сбрасывает состояние таймера.
		 *
		 * Обновляет время предыдущего тика до текущего момента, сбрасывает флаг паузы
		 * и накопленное время паузы.
		 * Рестарт всего кроме общего времени работы AppTime(придожения).
		 * Полезно вызывать после инициализации приложения(загрузки рксурсов) чтобы
		 * первый кадр не имел слишком большую дельту.
		 */
		void Reset();

	private:
		/// @brief Флаг, указывающий, находится ли приложение в режиме паузы.
		bool isPause;

		/// @brief Время создания объекта `AppTime`.
		std::chrono::time_point<Clock> m_TimeAppStart;

		/// @brief Время предыдущего вызова `GetDeltaTime`.
		std::chrono::time_point<Clock> m_PrevTick;

		/// @brief Время начала текущей паузы.
		std::chrono::time_point<Clock> m_StartPause;

		/// @brief Накопленное время пауз в секундах.
		double m_DeltaPause;
	};

	AppTime::AppTime() :
		isPause{ false },
		m_DeltaPause{ 0.0 }
	{
		m_TimeAppStart = Clock::now();
		m_PrevTick = m_TimeAppStart;
	}

	void AppTime::Reset()
	{
		m_PrevTick = Clock::now();
		isPause = false;
		m_DeltaPause = 0.0;
	}

	double AppTime::GetAllAppTime() const
	{
		auto now = Clock::now();
		double deltaTime = std::chrono::duration<double>(now - m_TimeAppStart).count();

		return deltaTime < 0.0 ? 0.0 : deltaTime;
	}

	double AppTime::GetDeltaTime()
	{
		auto now = Clock::now();

		if (isPause)
		{
			m_PrevTick = now;
			return 0.0;
		}

		double deltaTime = std::chrono::duration<double>(now - m_PrevTick).count();
		m_PrevTick = now;

		deltaTime -= m_DeltaPause;
		m_DeltaPause = 0.0;

		return deltaTime < 0.0 ? 0.0 : deltaTime;
	}

	void AppTime::Pause(bool _isPause)
	{
		if (isPause == _isPause)
			return;

		isPause = _isPause;
		if (isPause)
			m_StartPause = Clock::now();
		else
		{
			auto now = Clock::now();
			m_DeltaPause += std::chrono::duration<double>(now - m_StartPause).count();
		}
	}
}