#pragma once

#include <memory>

#include "engine/scene/domain/IObjectDomain.h"
#include "engine/scene/domain/IEntityDomain.h"
#include "engine/scene/domain/IMVVMDomain.h"

namespace zzz::engine
{
	/**
	 * @class IDomainFactory
	 * @brief Абстрактная фабрика для создания доменов слоя сцены.
	 * 
	 * @note [TODO: Future Extensions] В будущем методы фабрики смогут принимать параметры конкретных
	 *       запросов / дескрипторы конфигурации домена (например, ObjectDomainDesc, EntityDomainDesc,
	 *       MVVMDomainDesc), включающие:
	 *       - Стартовую ёмкость и стратегию аллокации памяти (initial capacity, pool sizes).
	 *       - Настройки многопоточности и политики доступа к данным.
	 *       - Регистрацию систем/архетипов для EntityDomain или контекст диспетчера событий для MVVM.
	 */
	class IDomainFactory
	{
	public:
		virtual ~IDomainFactory() = default;

		/// @brief Создаёт домен объектов GameObject (в будущем: с передачей параметров/дескриптора запроса).
		[[nodiscard]] virtual std::unique_ptr<IObjectDomain> CreateObjectDomain() = 0;

		/// @brief Создаёт ECS-домен сущностей (в будущем: с передачей параметров/дескриптора запроса).
		[[nodiscard]] virtual std::unique_ptr<IEntityDomain> CreateEntityDomain() = 0;

		/// @brief Создаёт домен элементов UI/MVVM (в будущем: с передачей параметров/дескриптора запроса).
		[[nodiscard]] virtual std::unique_ptr<IMVVMDomain>   CreateMVVMDomain() = 0;
	};
}
