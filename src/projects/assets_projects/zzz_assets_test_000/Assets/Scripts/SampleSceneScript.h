#pragma once
#include <SceneScript.h>

class SampleSceneScript : public zzz::script::SceneScript {
public:
    using SceneScript::SceneScript;
    const char* GetScriptTypeName() const override { return "SampleSceneScript"; }
    void OnBindEvents() override;

private:
    void OnInit();
    void OnStart();
    void OnEnable();
    void OnDisable();
    void OnUpdate(const zzz::engine::Time& time);
    void OnDestroy();
};
