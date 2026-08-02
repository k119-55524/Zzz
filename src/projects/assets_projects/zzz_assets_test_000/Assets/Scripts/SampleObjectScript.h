#pragma once
#include <Script.h>

class SampleObjectScript : public zzz::script::Script {
public:
    using Script::Script;
    const char* GetScriptTypeName() const override { return "SampleObjectScript"; }
    void OnBindEvents() override;

private:
    void OnInit();
    void OnStart();
    void OnEnable();
    void OnDisable();
    void OnUpdate(const zzz::engine::Time& time);
    void OnDestroy();
};
