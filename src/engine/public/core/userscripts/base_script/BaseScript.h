#pragma once

#include <memory>
#include "../EngineExport.h"

namespace zzz::script
{
#pragma warning(push)
#pragma warning(disable: 4251)
	class Z_ENGINE_API BaseScript : public std::enable_shared_from_this<BaseScript>
	{
	public:
		BaseScript() = default;
		virtual ~BaseScript() = default;

		virtual const char* GetScriptTypeName() const = 0;

		bool IsActive() const { return m_IsActive; }

		/**
		 * @brief Включает или выключает скрипт.
		 * ВНИМАНИЕ: Деактивация и последующая активация скрипта ломает изначальный порядок 
		 * вызовов событий, так как скрипт будет помещен в конец очереди EventBus!
		 * TODO: Разработать механизм сохранения изначального порядка (например, флаг паузы/деактивации внутри CallbackEntry вместо отписки).
		 */
		void SetActive(bool active)
		{
			if (m_IsActive == active)
				return;

			m_IsActive = active;

			if (m_IsActive)
			{
				OnBindEvents();
			}
			else
			{
				OnUnbindEvents();
			}
		}

	protected:
		/**
		 * @brief Привязка событий. Вызывается при создании скрипта и при его активации.
		 * Поместите сюда вызовы SubscribeToUpdate, SubscribeToStart и т.д.
		 */
		virtual void OnBindEvents() = 0;

		/**
		 * @brief Отвязка событий. Вызывается при деактивации скрипта.
		 */
		virtual void OnUnbindEvents() = 0;

	private:
		bool m_IsActive = true;
	};
#pragma warning(pop)
}
