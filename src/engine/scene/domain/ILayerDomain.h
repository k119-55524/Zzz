#pragma once

#include "core/utility/NonCopyable.h"

namespace zzz::engine
{
	/**
	 * @class ILayerDomain
	 * @brief Базовый полиморфный интерфейс домена слоя сцены.
	 */
	class ILayerDomain
	{
	public:
		virtual ~ILayerDomain() = default;

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
