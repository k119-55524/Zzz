#pragma once

#include <string_view>
#include "core/utils/Types.h"
#include "core/utils/ThrowWrappers.h"
#include <math/Math.h>

namespace zzz::core
{
	/**
	 * @brief Тип визуального перехода между сценами.
	 */
	enum class eTransitionType : zU8
	{
		Instant = 0,    // Мгновенная смена сцены без эффектов
		FadeColor = 1,  // Затемнение/высветление через цвет (Fade Out -> Fade In)
		CrossFade = 2   // Плавное растворение старой сцены в новую
	};

	constexpr std::string_view ToString(eTransitionType type)
	{
		switch (type)
		{
		case eTransitionType::Instant:   return "Instant";
		case eTransitionType::FadeColor: return "FadeColor";
		case eTransitionType::CrossFade: return "CrossFade";
		}
		THROW_RUNTIME("Необработанный eTransitionType");
	}

	/**
	 * @brief Источник настроек перехода сцены.
	 */
	enum class eTransitionSource : zU8
	{
		UseGlobal = 0,  // Использовать глобальные настройки переходов из SceneManager
		Custom = 1      // Использовать параметры перехода, заданные для этой сцены
	};

	constexpr std::string_view ToString(eTransitionSource source)
	{
		switch (source)
		{
		case eTransitionSource::UseGlobal: return "UseGlobal";
		case eTransitionSource::Custom:    return "Custom";
		}
		THROW_RUNTIME("Необработанный eTransitionSource");
	}

	/**
	 * @brief Параметры конвейера смены сцен.
	 * 
	 * Представляет собой структуру параметров и флагов поведения переходов сцен.
	 */
	struct SceneTransitionParams
	{
		eTransitionType type{ eTransitionType::Instant };
		zF32 durationSeconds{ 0.3f };
		zzz::math::Color4<zF32> fadeColor{ 0.0f, 0.0f, 0.0f, 1.0f }; // Черный по умолчанию

		// Флаги управления поведением перехода (каркас)
		bool blockUserInput{ true };        // Блокировать доставку пользовательского ввода в скрипты на время перехода
		bool pauseOldSceneUpdate{ true };   // Приостанавливать Update() уходящей сцены
		bool renderLoadingSpinner{ false }; // Отображать индикатор ожидания при задержке I/O

		bool operator==(const SceneTransitionParams&) const = default;
	};
}
