#pragma once
#include <ViewScript.h>

class SampleViewScript : public zzz::script::ViewScript {
public:
    using ViewScript::ViewScript;
    const char* GetScriptTypeName() const override { return "SampleViewScript"; }
    void OnBindEvents() override {}
    void OnShow();
};
