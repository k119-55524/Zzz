#pragma once

#include <string>
#include <string_view>
#include "core/utils/Guid.h"
#include "core/enums/eResourceType.h"
#include "math/utils/Types.h"

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
		{
		}

		virtual ~ResourceBase() = default;

		[[nodiscard]] virtual const ::zzz::core::Guid& GetGuid() const noexcept { return m_Guid; }
		[[nodiscard]] virtual ::zzz::core::eResourceType GetResourceType() const noexcept { return m_Type; }
		[[nodiscard]] virtual std::string_view GetName() const noexcept { return m_Name; }

	protected:
		::zzz::core::Guid m_Guid;
		::zzz::core::eResourceType m_Type;
		std::string m_Name;
	};
}
