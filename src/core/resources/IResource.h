#pragma once

#include <string_view>
#include "core/utils/Guid.h"
#include "core/enums/eResourceType.h"
#include "core/enums/eResourceState.h"
namespace zzz::core
{
	/**
	 * @class IResource
	 * @brief Базовый абстрактный интерфейс любого ресурса движка.
	 */
	class IResource
	{
	public:
		virtual ~IResource() = default;

		[[nodiscard]] virtual const Guid& GetGuid() const noexcept = 0;
		[[nodiscard]] virtual eResourceType GetResourceType() const noexcept = 0;
		[[nodiscard]] virtual std::string_view GetName() const noexcept = 0;
		[[nodiscard]] virtual eResourceState GetState() const noexcept = 0;
		virtual void SetState(eResourceState state) noexcept = 0;
	};
}
