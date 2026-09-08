#pragma once

#include "core/utils/macros/MiscMacros.h"
#include "core/enums/eObjectDomain.h"

namespace zzz::engine
{
	/**
	 * @class ILayerDomain
	 * @brief Базовый полиморфный интерфейс домена слоя сцены.
	 */
	class ILayerDomain
	{
	public:
		ILayerDomain() = default;
		virtual ~ILayerDomain() = default;

		Z_NO_COPY_MOVE(ILayerDomain);

		/**
		 * @brief Тип домена сущностей слоя.
		 */
		[[nodiscard]] virtual ::zzz::core::eObjectDomain GetDomainType() const noexcept = 0;

		/**
		 * @brief Кадровое обновление домена.
		 * @param dt Дельта времени текущего тика.
		 */
		virtual void Update(float dt) = 0;

		/**
		 * @brief Очистка всех данных и состояния домена.
		 */
		virtual void Clear() = 0;
	};
}
