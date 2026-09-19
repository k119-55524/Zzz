#pragma once

#include <string>
#include <string_view>
#include <atomic>
#include "core/utils/Guid.h"
#include "core/enums/eResourceType.h"
#include "core/enums/eResourceState.h"

namespace zzz::engine
{
	/**
	 * @class ResourceBase
	 * @brief Каноническая базовая реализация метаданных ресурса.
	 */
	class ResourceBase
	{
	public:
		ResourceBase(const ::zzz::core::Guid& guid, ::zzz::core::eResourceType type, std::string name)
			: m_Guid(guid)
			, m_Type(type)
			, m_Name(std::move(name))
			, m_State(::zzz::core::eResourceState::Ready)
		{
		}

		virtual ~ResourceBase() = default;

		[[nodiscard]] virtual const ::zzz::core::Guid& GetGuid() const noexcept { return m_Guid; }
		[[nodiscard]] virtual ::zzz::core::eResourceType GetResourceType() const noexcept { return m_Type; }
		[[nodiscard]] virtual std::string_view GetName() const noexcept { return m_Name; }
		[[nodiscard]] virtual ::zzz::core::eResourceState GetState() const noexcept { return m_State.load(std::memory_order_relaxed); }
		virtual void SetState(::zzz::core::eResourceState state) noexcept { m_State.store(state, std::memory_order_relaxed); }

	protected:
		::zzz::core::Guid m_Guid;
		::zzz::core::eResourceType m_Type;
		std::string m_Name;
		std::atomic<::zzz::core::eResourceState> m_State;
	};
}
