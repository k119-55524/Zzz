#pragma once
#include <SceneScript.h>

class SampleSceneScript : public zzz::script::SceneScript {
public:
    using SceneScript::SceneScript;
    const char* GetScriptTypeName() const override { return "SampleSceneScript"; }
    void OnBindEvents() override {}
    void OnLoad();
};
