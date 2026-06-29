#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <common/templates/Event.h>

#include "../EngineExport.h"

namespace zzz
{
	class GameObject;
}

namespace zzz::script
{
	struct ScriptEvents
	{
		zzz::engine::Event<> OnStart;
		zzz::engine::Event<float> OnUpdate;
		zzz::engine::Event<> OnDestroy;
	};

	class Z_ENGINE_API Script : public std::enable_shared_from_this<Script>
	{
	public:
		explicit Script(GameObject* owner);
		virtual ~Script();

		GameObject* GetOwner() const { return m_Owner; }
		virtual std::string_view GetScriptTypeName() const = 0;

		virtual std::string SerializeState() { return ""; }
		virtual void DeserializeState(const std::string& /*data*/) {}

		ScriptEvents Events;

	private:
		GameObject* m_Owner;
	};
}
