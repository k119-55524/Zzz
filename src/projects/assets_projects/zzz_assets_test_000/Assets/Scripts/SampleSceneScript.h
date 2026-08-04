#pragma once
#include <SceneScript.h>

class SampleSceneScript : public zzz::core::SceneScript {
public:
    using SceneScript::SceneScript;
    const char* GetScriptTypeName() const override { return "SampleSceneScript"; }
    void OnBindEvents() override;

private:
    void OnInit();
    void OnStart();
    void OnEnable();
    void OnDisable();
    void OnUpdate(const zzz::core::Time& time);
    void OnDestroy();
};
