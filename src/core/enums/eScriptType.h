#pragma once

#include <string_view>
#include "math/utils/Types.h"
#include "core/utils/ThrowWrappers.h"

namespace zzz::core
{
	/**
	 * @enum eScriptType
	 * @brief Типы пользовательских и движковых скриптов (C++ код, компилируемый в библиотеку zzz_user_scripts).
	 *
	 * @details Скрипты не упаковываются в .dat архивы и управляются собственной инфраструктурой
	 *          (ScriptRegistry, ScriptFactory, ScriptStorage).
	 */
	enum class eScriptType : zU8
	{
		Unknown = 0,

		// --- Базовая иерархия движка и жизненного цикла ---
		Game,             ///< Глобальный скрипт игры (GameScript) — живёт на протяжении всей сессии игры
		Scene,            ///< Скрипт сцены (SceneScript) — управляет логикой и объектами конкретной сцены
		Layer,            ///< Скрипт слоя (LayerScript) — логика отдельного GameLayer
		GameObject,       ///< Компонентный скрипт игрового объекта (Script) — крепится к GameObject
		ECS,              ///< ECS-система / логика сущности в парадигме Entity-Component-System
		Common,           ///< Обычный / автономный скрипт без строгой привязки к контексту

		// --- Данные и расширения инструментов ---
		ScriptableObject, ///< Контейнер данных / статическая база данных (конфиги предметов, характеристики)
		Editor,           ///< Скрипт расширения редактора (кастомные инспекторы, тулзы)

		// --- Паттерн MVVM и UI Поведение ---
		Model,            ///< Бизнес-логика и чистые данные предметной области (без знания о UI)
		ViewModel,        ///< Модель представления (состояние экрана, команды, реактивные свойства для биндинга)
		View,             ///< Представление (ViewScript) — связывает окно/вьюху со слоем
		UIBehavior        ///< Поведение UI (Attached Behavior / анимации разметки, интерактивность виджетов)
	};

	[[nodiscard]] constexpr std::string_view ToString(eScriptType type)
	{
		switch (type)
		{
		case eScriptType::Unknown:          return "Unknown";
		case eScriptType::Game:             return "Game";
		case eScriptType::Scene:            return "Scene";
		case eScriptType::Layer:            return "Layer";
		case eScriptType::GameObject:       return "GameObject";
		case eScriptType::ECS:              return "ECS";
		case eScriptType::Common:           return "Common";
		case eScriptType::ScriptableObject: return "ScriptableObject";
		case eScriptType::Editor:           return "Editor";
		case eScriptType::Model:            return "Model";
		case eScriptType::ViewModel:        return "ViewModel";
		case eScriptType::View:             return "View";
		case eScriptType::UIBehavior:       return "UIBehavior";
		}
		THROW_RUNTIME("Необработанный eScriptType");
	}
}
