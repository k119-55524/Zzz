#pragma once

#include <memory>
#include <utility>
#include <type_traits>
#include "engine/resources/ResourceBase.h"

namespace zzz::engine
{
	/**
	 * @class ResourceRef
	 * @brief Легковесная RAII-обёртка для интрузивного подсчёта ссылок на ресурсы движка.
	 * @details Автоматически вызывает приватные AddRef() и Release() у ResourceBase,
	 *          обеспечивая безопасный подсчет ссылок и прозрачный доступ к ресурсу.
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
		{
			if (m_Resource)
			{
				static_cast<ResourceBase*>(m_Resource.get())->AddRef();
			}
		}

		~ResourceRef()
		{
			Reset();
		}

		ResourceRef(const ResourceRef& other)
			: m_Resource(other.m_Resource)
		{
			if (m_Resource)
			{
				static_cast<ResourceBase*>(m_Resource.get())->AddRef();
			}
		}

		ResourceRef& operator=(const ResourceRef& other)
		{
			if (this != &other)
			{
				Reset();
				m_Resource = other.m_Resource;
				if (m_Resource)
				{
					static_cast<ResourceBase*>(m_Resource.get())->AddRef();
				}
			}
			return *this;
		}

		ResourceRef(ResourceRef&& other) noexcept
			: m_Resource(std::move(other.m_Resource))
		{
		}

		ResourceRef& operator=(ResourceRef&& other) noexcept
		{
			if (this != &other)
			{
				Reset();
				m_Resource = std::move(other.m_Resource);
			}
			return *this;
		}

		ResourceRef& operator=(std::shared_ptr<T> resource)
		{
			if (m_Resource != resource)
			{
				Reset();
				m_Resource = std::move(resource);
				if (m_Resource)
				{
					static_cast<ResourceBase*>(m_Resource.get())->AddRef();
				}
			}
			return *this;
		}

		void Reset() noexcept
		{
			if (m_Resource)
			{
				static_cast<ResourceBase*>(m_Resource.get())->Release();
				m_Resource.reset();
			}
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
