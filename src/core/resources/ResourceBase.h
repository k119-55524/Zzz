#pragma once

#include <string>
#include <atomic>
#include "core/resources/IResource.h"
namespace zzz::core
{
	/**
	 * @class ResourceBase
	 * @brief Каноническая базовая реализация метаданных ресурса.
	 */
	class ResourceBase : public IResource
	{
	public:
		ResourceBase(const Guid& guid, eResourceType type, std::string name)
			: m_Guid(guid)
			, m_Type(type)
			, m_Name(std::move(name))
			, m_State(eResourceState::Ready)
		{
		}

		~ResourceBase() override = default;

		[[nodiscard]] const Guid& GetGuid() const noexcept override { return m_Guid; }
		[[nodiscard]] eResourceType GetResourceType() const noexcept override { return m_Type; }
		[[nodiscard]] std::string_view GetName() const noexcept override { return m_Name; }
		[[nodiscard]] eResourceState GetState() const noexcept override { return m_State.load(std::memory_order_relaxed); }
		void SetState(eResourceState state) noexcept override { m_State.store(state, std::memory_order_relaxed); }

	protected:
		Guid m_Guid;
		eResourceType m_Type;
		std::string m_Name;
		std::atomic<eResourceState> m_State;
	};
}
