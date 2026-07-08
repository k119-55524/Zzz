#pragma once
#include <Game.h>

namespace zer
{
	class Test1 : public zzz::script::Game
	{
	public:
		Test1();
	
		void OnStart() override;
		void OnUpdate(float deltaTime) override;

	};
}

