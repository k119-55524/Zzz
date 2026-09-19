#pragma once

#include <utility>
#include <cstddef>

#include "core/enums/eResourceType.h"
#include "engine/resources/cpu/CpuMesh.h"
#include "engine/resources/cpu/CpuMaterial.h"
#include "engine/resources/cpu/CpuTexture2D.h"
#include "engine/resources/cpu/CpuShader.h"
#include "engine/resources/gpu/GpuMesh.h"
#include "engine/resources/gpu/GpuMaterial.h"
#include "engine/resources/gpu/GpuTexture2D.h"
#include "engine/resources/gpu/GpuShader.h"

using namespace zzz::core;

namespace zzz::engine
{
	template<typename... SupportedResources>
	class CpuResourceManager;

	template<typename... SupportedResources>
	class GpuResourceManager;

	/**
	 * @enum eEngineResourceType
	 * @brief Перечисление типов ресурсов, находящихся под управлением централизованных менеджеров ресурсов.
	 * @note Добавление значения в данный enum требует обязательной специализации ResourceBinding<Type>,
	 *       иначе при компиляции возникнет ошибка неполного типа (incomplete type).
	 */
	enum class eEngineResourceType : zU8
	{
		Mesh,
		Material,
		Texture2D,
		Shader,
		_Count // Маркер общего количества типов
	};

	/**
	 * @struct ResourceBinding
	 * @brief Декларативная привязка eEngineResourceType к конкретным типам ресурсов ядра.
	 * @note Базовый шаблон не определен намеренно для гарантии ошибок компиляции при расширении enum.
	 */
	template<eEngineResourceType Type>
	struct ResourceBinding;

	template<>
	struct ResourceBinding<eEngineResourceType::Mesh>
	{
		static constexpr eResourceType CoreType = eResourceType::Mesh;
		using Cpu = CpuMesh;
		using Gpu = GpuMesh;
	};

	template<>
	struct ResourceBinding<eEngineResourceType::Material>
	{
		static constexpr eResourceType CoreType = eResourceType::Material;
		using Cpu = CpuMaterial;
		using Gpu = GpuMaterial;
	};

	template<>
	struct ResourceBinding<eEngineResourceType::Texture2D>
	{
		static constexpr eResourceType CoreType = eResourceType::Texture2D;
		using Cpu = CpuTexture2D;
		using Gpu = GpuTexture2D;
	};

	template<>
	struct ResourceBinding<eEngineResourceType::Shader>
	{
		static constexpr eResourceType CoreType = eResourceType::Shader;
		using Cpu = CpuShader;
		using Gpu = GpuShader;
	};

	template<size_t... Is>
	auto BuildCpuManager(std::index_sequence<Is...>)
		-> CpuResourceManager<typename ResourceBinding<static_cast<eEngineResourceType>(Is)>::Cpu...>;

	template<size_t... Is>
	auto BuildGpuManager(std::index_sequence<Is...>)
		-> GpuResourceManager<typename ResourceBinding<static_cast<eEngineResourceType>(Is)>::Gpu...>;

	using CoreCpuResourceManager = decltype(BuildCpuManager(std::make_index_sequence<static_cast<size_t>(eEngineResourceType::_Count)>{}));
	using CoreGpuResourceManager = decltype(BuildGpuManager(std::make_index_sequence<static_cast<size_t>(eEngineResourceType::_Count)>{}));
}
