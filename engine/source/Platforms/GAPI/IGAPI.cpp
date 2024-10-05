#include "pch.h"
#include "IGAPI.h"

using namespace Zzz::Platforms;

IGAPI::IGAPI(const shared_ptr<IWinApp> _appWin, eGAPIType type) :
	appWin{ _appWin },
	gapiType{ type },
	initState{ e_InitState::eInitNot }
{
}

IGAPI::~IGAPI()
{
}

void IGAPI::Update()
{
	if (initState != e_InitState::eInitOK)
		return;

	OnUpdate();
	OnRender();
}

void IGAPI::Resize(const zSize& size)
{
	if (initState != e_InitState::eInitOK)
		return;

	OnResize(size);
}
