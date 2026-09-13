#pragma once

#include "math/quat/Quat.h"
#include "math/utils/Types.h"
#include "math/vector/Vec3.h"

#include "core/utils/Guid.h"

using namespace zzz::math;
using namespace zzz::core;

/**
 * @file NodeTypes.h
 * @brief Базовые типы данных и структуры узлов графа сцены (NodeStorage).
 */
namespace zzz::engine
{
	using NodeHandle = zU32;
	using DomainHandle = zU32;
	using SpatialHandle = zU32;

	inline constexpr NodeHandle kInvalidNodeHandle = 0xFFFFFFFF;
	inline constexpr DomainHandle kInvalidDomainHandle = 0xFFFFFFFF;
	inline constexpr SpatialHandle kInvalidSpatialHandle = 0xFFFFFFFF;

	/// @brief Флаги состояния узла сцены.
	enum class eNodeFlags : zU8
	{
		None = 0,
		Active = 1 << 0,
		Visible = 1 << 1
	};

	[[nodiscard]] constexpr eNodeFlags operator|(eNodeFlags lhs, eNodeFlags rhs) noexcept
	{
		return static_cast<eNodeFlags>(static_cast<zU8>(lhs) | static_cast<zU8>(rhs));
	}

	[[nodiscard]] constexpr eNodeFlags operator&(eNodeFlags lhs, eNodeFlags rhs) noexcept
	{
		return static_cast<eNodeFlags>(static_cast<zU8>(lhs) & static_cast<zU8>(rhs));
	}

	[[nodiscard]] constexpr eNodeFlags operator~(eNodeFlags flag) noexcept
	{
		return static_cast<eNodeFlags>(~static_cast<zU8>(flag));
	}

	inline eNodeFlags& operator|=(eNodeFlags& lhs, eNodeFlags rhs) noexcept
	{
		lhs = lhs | rhs;
		return lhs;
	}

	inline eNodeFlags& operator&=(eNodeFlags& lhs, eNodeFlags rhs) noexcept
	{
		lhs = lhs & rhs;
		return lhs;
	}

	/**
	 * @struct VisualRange
	 * @brief Диапазон дескрипторов отрисовки узла в общем плоском массиве m_Draws.
	 */
	struct VisualRange
	{
		zU32 begin{ 0 };
		zU32 count{ 0 };

		[[nodiscard]] bool HasMesh() const noexcept { return count > 0; }
	};

	/**
	 * @struct DrawDescriptor
	 * @brief Непосредственный дескриптор одной отрисовки (связка геометрии и материала).
	 */
	struct DrawDescriptor
	{
		Guid meshGuid;
		Guid materialGuid;

		[[nodiscard]] bool HasMesh() const noexcept { return meshGuid.IsValid(); }
		[[nodiscard]] bool HasMaterial() const noexcept { return materialGuid.IsValid(); }
	};

	/// @brief Категория домена, к которому привязан узел.
	enum class eNodeDomainKind : zU8
	{
		None = 0,
		Object,
		Entity
	};

	/// @brief Компоненты пространственного положения (Rot, Pos, Scale).
	struct Transform
	{
		Quat<zF32> rotation{ 0.0f, 0.0f, 0.0f, 1.0f };
		Vec3<zF32> position{ 0.0f, 0.0f, 0.0f };
		Vec3<zF32> scale{ 1.0f, 1.0f, 1.0f };
	};

	/// @brief Внешние связи узла с доменным и пространственным хранилищами.
	struct NodeBindings
	{
		DomainHandle domainHandle{ kInvalidDomainHandle };
		SpatialHandle spatialHandle{ kInvalidSpatialHandle };
		eNodeDomainKind domainKind{ eNodeDomainKind::None };
	};

	/// @brief Непрерывный диапазон индексов узлов с обновлёнными мировыми матрицами.
	struct TransformChangeRange
	{
		zU32 begin{ 0 };
		zU32 end{ 0 }; // exclusive
	};
}
