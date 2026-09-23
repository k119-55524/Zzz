#pragma once

#include <memory>
#include <utility>
#include <type_traits>

#include "engine/resources/ResourceBase.h"

namespace zzz::engine
{
	/**
	 * @class ResourceRef
	 * @brief Легковесная типизированная RAII-обёртка над std::shared_ptr ресурса движка.
	 * @tparam T Конкретный тип ресурса (должен наследоваться от ResourceBase).
	 */
	template<typename T>
	class ResourceRef final
	{
		static_assert(std::is_base_of_v<ResourceBase, T>, "ResourceRef может быть инстанцирован только для типов, наследующих ResourceBase!");

	public:
		ResourceRef() noexcept
			: m_Resource(nullptr)
		{
		}

		explicit ResourceRef(std::shared_ptr<T> resource)
			: m_Resource(std::move(resource))
		{}

		~ResourceRef() = default;

		ResourceRef(const ResourceRef&) = default;
		ResourceRef(ResourceRef&&) noexcept = default;
		ResourceRef& operator=(const ResourceRef&) = default;
		ResourceRef& operator=(ResourceRef&&) noexcept = default;

		ResourceRef& operator=(std::shared_ptr<T> resource)
		{
			m_Resource = std::move(resource);
			return *this;
		}

		void Reset() noexcept
		{
			m_Resource.reset();
		}

		[[nodiscard]] T* Get() const noexcept { return m_Resource.get(); }
		[[nodiscard]] T* operator->() const noexcept { return m_Resource.get(); }
		[[nodiscard]] T& operator*() const noexcept { return *m_Resource; }

		[[nodiscard]] explicit operator bool() const noexcept { return m_Resource != nullptr; }

		[[nodiscard]] bool operator==(const ResourceRef&) const noexcept = default;
		[[nodiscard]] bool operator==(std::nullptr_t) const noexcept { return m_Resource == nullptr; }

	private:
		std::shared_ptr<T> m_Resource;
	};
}
