#pragma once
#include <core/Core.h>

class SampleObjectScript : public zzz::core::Script {
public:
    using Script::Script;
    const char* GetScriptTypeName() const override { return "SampleObjectScript"; }
    void OnBindEvents() override;

private:
    void OnInit();
    void OnStart();
    void OnEnable();
    void OnDisable();
    void OnUpdate(const zzz::core::Time& time);
    void OnDestroy();
};
