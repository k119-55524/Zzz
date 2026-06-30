#pragma once
#include <Game.h>

class StartGame1 : public zzz::script::Game
{
public:
    StartGame1();

    std::string_view GetScriptTypeName() const override { return "StartGame1"; }
};
