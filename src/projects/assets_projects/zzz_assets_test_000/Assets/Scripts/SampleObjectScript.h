#pragma once
#include <Script.h>

class SampleObjectScript : public zzz::script::Script {
public:
    using Script::Script;
    const char* GetScriptTypeName() const override { return "SampleObjectScript"; }
    void OnBindEvents() override {}
    void OnStart();
};
