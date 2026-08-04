#pragma once

#include <core/time/Time.h>
#include <core/events/Event.h>

namespace zzz::script
{
	class GameScript;
	class SceneScript;
	class ViewScript;
	class Script;
}

namespace zzz::engine
{
	/**
	 * @brief Шина событий глобального уровня (уровня проекта).
	 * Используется для глобальных скриптов (GameScript), которые живут от старта и до остановки игры.
	 */
	class ProjectEventBus
	{
	public:
		void InvokeStart() { OnStart(); }
		void InvokeDestroy() { OnDestroy(); }
		void InvokeEnable() { OnEnable(); }
		void InvokeDisable() { OnDisable(); }
		void InvokeUpdate(const zzz::engine::Time& t) { OnUpdate(t); }

		void UnsubscribeAll(const std::shared_ptr<void>& context)
		{
			OnStart.Unsubscribe(context);
			OnDestroy.Unsubscribe(context);
			OnEnable.Unsubscribe(context);
			OnDisable.Unsubscribe(context);
			OnUpdate.Unsubscribe(context);
		}

		void ClearAll()
		{
			OnStart.Clear();
			OnDestroy.Clear();
			OnEnable.Clear();
			OnDisable.Clear();
			OnUpdate.Clear();
		}

	private:
		friend class zzz::script::GameScript;
		Event<> OnStart;
		Event<> OnDestroy;
		Event<> OnEnable;
		Event<> OnDisable;
		Event<const zzz::engine::Time&> OnUpdate;
	};

	/**
	 * @brief Шина событий уровня отдельной сцены.
	 * Используется для скриптов сцены (SceneScript). Жизненный цикл ограничен загрузкой и выгрузкой конкретной сцены.
	 */
	class SceneEventBus
	{
	public:
		void InvokeStart() { OnStart(); }
		void InvokeDestroy() { OnDestroy(); }
		void InvokeEnable() { OnEnable(); }
		void InvokeDisable() { OnDisable(); }
		void InvokeUpdate(const zzz::engine::Time& t) { OnUpdate(t); }

		void UnsubscribeAll(const std::shared_ptr<void>& context)
		{
			OnStart.Unsubscribe(context);
			OnDestroy.Unsubscribe(context);
			OnEnable.Unsubscribe(context);
			OnDisable.Unsubscribe(context);
			OnUpdate.Unsubscribe(context);
		}

	private:
		friend class zzz::script::SceneScript;
		Event<> OnStart;
		Event<> OnDestroy;
		Event<> OnEnable;
		Event<> OnDisable;
		Event<const zzz::engine::Time&> OnUpdate;
	};

	/**
	 * @brief Шина событий уровня игрового объекта (GameObject).
	 * Используется для компонентов и скриптов (Script), прикрепленных к конкретным объектам на сцене.
	 * События не имеют строгой гарантии порядка вызова (используется UnorderedEvent).
	 */
	class GameObjectEventBus
	{
	public:
		void InvokeStart() { OnStart(); }
		void InvokeDestroy() { OnDestroy(); }
		void InvokeEnable() { OnEnable(); }
		void InvokeDisable() { OnDisable(); }
		void InvokeUpdate(const zzz::engine::Time& t) { OnUpdate(t); }

		void UnsubscribeAll(const std::shared_ptr<void>& context)
		{
			OnStart.Unsubscribe(context);
			OnDestroy.Unsubscribe(context);
			OnEnable.Unsubscribe(context);
			OnDisable.Unsubscribe(context);
			OnUpdate.Unsubscribe(context);
		}

	private:
		friend class zzz::script::Script;
		UnorderedEvent<> OnStart;
		UnorderedEvent<> OnDestroy;
		UnorderedEvent<> OnEnable;
		UnorderedEvent<> OnDisable;
		UnorderedEvent<const zzz::engine::Time&> OnUpdate;
	};

	/**
	 * @brief Шина событий уровня представления (View / Окна).
	 * Используется для скриптов, привязанных к конкретному View (ViewScript).
	 */
	class ViewEventBus
	{
	public:
		void InvokeStart() { OnStart(); }
		void InvokeDestroy() { OnDestroy(); }
		void InvokeEnable() { OnEnable(); }
		void InvokeDisable() { OnDisable(); }
		void InvokeUpdate(const zzz::engine::Time& t) { OnUpdate(t); }

		void UnsubscribeAll(const std::shared_ptr<void>& context)
		{
			OnStart.Unsubscribe(context);
			OnDestroy.Unsubscribe(context);
			OnEnable.Unsubscribe(context);
			OnDisable.Unsubscribe(context);
			OnUpdate.Unsubscribe(context);
		}

	private:
		friend class zzz::script::ViewScript;
		Event<> OnStart;
		Event<> OnDestroy;
		Event<> OnEnable;
		Event<> OnDisable;
		Event<const zzz::engine::Time&> OnUpdate;
	};
}
