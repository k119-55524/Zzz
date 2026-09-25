#pragma once

#include <string_view>

#include "math/utils/Types.h"
#include "core/utils/ThrowWrappers.h"

namespace zzz::core
{
	/**
	 * @enum ePackageDatType
	 * @brief Физические типы записей в оглавлении архива package.dat (контейнер структуры игры).
	 *
	 * @details Определяет тип ресурса верхнего уровня (манифест, сцены, вьюхи, префабы),
	 *          упакованного в бинарный архив c_GamePackageRelativePath.
	 */
	enum class ePackageDatType : zU32
	{
		ProjectManifest = 1, ///< Манифест проекта (ProjectManifestData)
		Scene           = 2, ///< Игровая сцена (SceneData)
		PrimaryView     = 3, ///< Основное окно игры (PrimaryViewData)
		ChildView       = 4, ///< Дочернее представление (ChildViewData)
		IndependentView = 5, ///< Независимое окно (IndependentViewData)
		Prefab          = 6, ///< Иерархия префаба (PrefabData)

		/// @brief Граничные маркеры диапазона для compile-time вычисления размера массива таблиц
		///        (c_TypeCount), проверки валидности типа и индексации в ArchiveTraitsBase.
		First           = ProjectManifest,
		Last            = Prefab
	};

	[[nodiscard]] constexpr std::string_view ToString(ePackageDatType type)
	{
		switch (type)
		{
		case ePackageDatType::ProjectManifest: return "ProjectManifest";
		case ePackageDatType::Scene:           return "Scene";
		case ePackageDatType::PrimaryView:     return "PrimaryView";
		case ePackageDatType::ChildView:       return "ChildView";
		case ePackageDatType::IndependentView: return "IndependentView";
		case ePackageDatType::Prefab:          return "Prefab";
		}
		THROW_RUNTIME("Необработанный ePackageDatType");
	}
}
