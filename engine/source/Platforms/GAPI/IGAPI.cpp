#include "pch.h"
#include "IGAPI.h"

using namespace Zzz::Platforms;

IGAPI::IGAPI(eGAPIType type) :
	gapiType{ type }
{
}

IGAPI::~IGAPI()
{
}

void IGAPI::Update()
{
	OnUpdate();
	OnRender();
}