#pragma once
#include <Game.h>

class StartGame : public zzz::script::Game
{
public:
	StartGame();

	std::string_view GetScriptTypeName() const override { return "StartGame"; }
};
