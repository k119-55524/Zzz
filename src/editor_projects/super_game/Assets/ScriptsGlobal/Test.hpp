#pragma once
#include <GameScript.h>

class Test : public zzz::script::GameScript
{
	public:
		Test();
		virtual ~Test() = default;

	private:
		void Init(std::shared_ptr<zzz::engine::ProjectEventBus> bus) override;

		void OnStart();
		void OnUpdate(const zzz::engine::Time& time);

		int i;
};
