#pragma once

#include <optional>
#include <string_view>

#include "math/utils/Types.h"
#include "core/utils/ThrowWrappers.h"
#include "core/enums/ePackageDatType.h"
#include "core/enums/eDataDatType.h"
#include "core/enums/eEngineResourceType.h"

namespace zzz::core
{
	/**
	 * @enum eArchiveLocation
	 * @brief Целевой физический архив, в который упаковывается данный ресурс.
	 */
	enum class eArchiveLocation : zU8
	{
		None = 0,   ///< Не упаковывается в архив (например, Unknown или исходный код)
		PackageDat, ///< Упаковывается в архив структуры игры (packages/package.dat)
		DataDat     ///< Упаковывается в архив сырых ресурсов игры (packages/data.dat)
	};

	[[nodiscard]] constexpr std::string_view ToString(eArchiveLocation loc)
	{
		switch (loc)
		{
		case eArchiveLocation::None:       return "None";
		case eArchiveLocation::PackageDat: return "PackageDat";
		case eArchiveLocation::DataDat:    return "DataDat";
		}
		THROW_RUNTIME("Необработанный eArchiveLocation");
	}

	/**
	 * @brief Определяет, в какой архив должен быть упакован данный тип движкового ресурса.
	 */
	[[nodiscard]] constexpr eArchiveLocation GetArchiveLocation(eEngineResourceType type) noexcept
	{
		switch (type)
		{
		case eEngineResourceType::ProjectManifest:
		case eEngineResourceType::View:
		case eEngineResourceType::PrimaryView:
		case eEngineResourceType::ChildView:
		case eEngineResourceType::IndependentView:
		case eEngineResourceType::Scene:
		case eEngineResourceType::Prefab:
			return eArchiveLocation::PackageDat;

		case eEngineResourceType::Mesh:
		case eEngineResourceType::Material:
		case eEngineResourceType::Shader:
		case eEngineResourceType::Animation:
		case eEngineResourceType::Texture2D:
		case eEngineResourceType::AudioClip:
		case eEngineResourceType::Video:
		case eEngineResourceType::Font:
		case eEngineResourceType::BinaryData:
			return eArchiveLocation::DataDat;

		case eEngineResourceType::Unknown:
		default:
			return eArchiveLocation::None;
		}
	}

	/// @brief Проверяет, предназначен ли ресурс для архива package.dat.
	[[nodiscard]] constexpr bool IsPackageDatResource(eEngineResourceType type) noexcept
	{
		return GetArchiveLocation(type) == eArchiveLocation::PackageDat;
	}

	/// @brief Проверяет, предназначен ли ресурс для архива data.dat.
	[[nodiscard]] constexpr bool IsDataDatResource(eEngineResourceType type) noexcept
	{
		return GetArchiveLocation(type) == eArchiveLocation::DataDat;
	}

	// -------------------------------------------------------------------------
	// Строгие конвертеры (бросают THROW_RUNTIME при невозможности конвертации)
	// -------------------------------------------------------------------------

	/**
	 * @brief Преобразует тип движкового ресурса в физический тип записи package.dat.
	 * @throws std::runtime_error если ресурс не предназначен для package.dat.
	 */
	[[nodiscard]] constexpr ePackageDatType ToPackageDatType(eEngineResourceType type)
	{
		switch (type)
		{
		case eEngineResourceType::ProjectManifest: return ePackageDatType::ProjectManifest;
		case eEngineResourceType::PrimaryView:     return ePackageDatType::PrimaryView;
		case eEngineResourceType::ChildView:       return ePackageDatType::ChildView;
		case eEngineResourceType::IndependentView: return ePackageDatType::IndependentView;
		case eEngineResourceType::View:            return ePackageDatType::PrimaryView;
		case eEngineResourceType::Scene:           return ePackageDatType::Scene;
		case eEngineResourceType::Prefab:          return ePackageDatType::Prefab;
		default:
			THROW_RUNTIME("Тип ресурса движка '{}' не может быть преобразован в ePackageDatType (не предназначен для package.dat).",
				ToString(type));
		}
	}

	/**
	 * @brief Преобразует тип движкового ресурса в физический тип записи data.dat.
	 * @throws std::runtime_error если ресурс не предназначен для data.dat.
	 */
	[[nodiscard]] constexpr eDataDatType ToDataDatType(eEngineResourceType type)
	{
		switch (type)
		{
		case eEngineResourceType::Mesh:       return eDataDatType::Mesh;
		case eEngineResourceType::Material:   return eDataDatType::Material;
		case eEngineResourceType::Shader:     return eDataDatType::Shader;
		case eEngineResourceType::Animation:  return eDataDatType::Animation;
		case eEngineResourceType::Texture2D:  return eDataDatType::Texture2D;
		case eEngineResourceType::AudioClip:  return eDataDatType::AudioClip;
		case eEngineResourceType::Video:      return eDataDatType::Video;
		case eEngineResourceType::Font:       return eDataDatType::Font;
		case eEngineResourceType::BinaryData: return eDataDatType::BinaryData;
		default:
			THROW_RUNTIME("Тип ресурса движка '{}' не может быть преобразован в eDataDatType (не предназначен для data.dat).",
				ToString(type));
		}
	}

	/**
	 * @brief Преобразует физический тип записи package.dat в тип движкового ресурса.
	 */
	[[nodiscard]] constexpr eEngineResourceType ToEngineResourceType(ePackageDatType type) noexcept
	{
		switch (type)
		{
		case ePackageDatType::ProjectManifest: return eEngineResourceType::ProjectManifest;
		case ePackageDatType::Scene:           return eEngineResourceType::Scene;
		case ePackageDatType::PrimaryView:     return eEngineResourceType::PrimaryView;
		case ePackageDatType::ChildView:       return eEngineResourceType::ChildView;
		case ePackageDatType::IndependentView: return eEngineResourceType::IndependentView;
		case ePackageDatType::Prefab:          return eEngineResourceType::Prefab;
		}
		THROW_RUNTIME("Необработанный ePackageDatType");
	}

	/**
	 * @brief Преобразует физический тип записи data.dat в тип движкового ресурса.
	 */
	[[nodiscard]] constexpr eEngineResourceType ToEngineResourceType(eDataDatType type) noexcept
	{
		switch (type)
		{
		case eDataDatType::Mesh:       return eEngineResourceType::Mesh;
		case eDataDatType::Material:   return eEngineResourceType::Material;
		case eDataDatType::Shader:     return eEngineResourceType::Shader;
		case eDataDatType::Animation:  return eEngineResourceType::Animation;
		case eDataDatType::Texture2D:  return eEngineResourceType::Texture2D;
		case eDataDatType::AudioClip:  return eEngineResourceType::AudioClip;
		case eDataDatType::Video:      return eEngineResourceType::Video;
		case eDataDatType::Font:       return eEngineResourceType::Font;
		case eDataDatType::BinaryData: return eEngineResourceType::BinaryData;
		}
		THROW_RUNTIME("Необработанный eDataDatType");
	}

	// -------------------------------------------------------------------------
	// Безопасные небросающие обёртки для сборщика и валидаторов
	// -------------------------------------------------------------------------

	[[nodiscard]] constexpr std::optional<ePackageDatType> TryToPackageDatType(eEngineResourceType type) noexcept
	{
		switch (type)
		{
		case eEngineResourceType::ProjectManifest: return ePackageDatType::ProjectManifest;
		case eEngineResourceType::PrimaryView:     return ePackageDatType::PrimaryView;
		case eEngineResourceType::ChildView:       return ePackageDatType::ChildView;
		case eEngineResourceType::IndependentView: return ePackageDatType::IndependentView;
		case eEngineResourceType::View:            return ePackageDatType::PrimaryView;
		case eEngineResourceType::Scene:           return ePackageDatType::Scene;
		case eEngineResourceType::Prefab:          return ePackageDatType::Prefab;
		default:                                   return std::nullopt;
		}
	}

	[[nodiscard]] constexpr std::optional<eDataDatType> TryToDataDatType(eEngineResourceType type) noexcept
	{
		switch (type)
		{
		case eEngineResourceType::Mesh:       return eDataDatType::Mesh;
		case eEngineResourceType::Material:   return eDataDatType::Material;
		case eEngineResourceType::Shader:     return eDataDatType::Shader;
		case eEngineResourceType::Animation:  return eDataDatType::Animation;
		case eEngineResourceType::Texture2D:  return eDataDatType::Texture2D;
		case eEngineResourceType::AudioClip:  return eDataDatType::AudioClip;
		case eEngineResourceType::Video:      return eDataDatType::Video;
		case eEngineResourceType::Font:       return eDataDatType::Font;
		case eEngineResourceType::BinaryData: return eDataDatType::BinaryData;
		default:                              return std::nullopt;
		}
	}

	// -------------------------------------------------------------------------
	// Прямые операторы сравнения (равенства)
	// -------------------------------------------------------------------------

	[[nodiscard]] constexpr bool operator==(eEngineResourceType engineType, ePackageDatType pkgType) noexcept
	{
		const auto mapped = TryToPackageDatType(engineType);
		return mapped.has_value() && *mapped == pkgType;
	}

	[[nodiscard]] constexpr bool operator==(ePackageDatType pkgType, eEngineResourceType engineType) noexcept
	{
		return engineType == pkgType;
	}

	[[nodiscard]] constexpr bool operator==(eEngineResourceType engineType, eDataDatType dataDatType) noexcept
	{
		const auto mapped = TryToDataDatType(engineType);
		return mapped.has_value() && *mapped == dataDatType;
	}

	[[nodiscard]] constexpr bool operator==(eDataDatType dataDatType, eEngineResourceType engineType) noexcept
	{
		return engineType == dataDatType;
	}
}
