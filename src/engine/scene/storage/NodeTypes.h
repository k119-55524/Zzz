#pragma once

#include "math/quat/Quat.h"
#include "math/utils/Types.h"
#include "math/vector/Vec3.h"

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

	/// @brief Битовые флаги состояния узла сцены.
	namespace NodeFlags
	{
		inline constexpr zU8 None    = 0;
		inline constexpr zU8 Active  = 1 << 0;
		inline constexpr zU8 Visible = 1 << 1;
	}

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
}

