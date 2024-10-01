#include "pch.h"
#include "IMainAppLoop.h"

using namespace Zzz::Platforms;

IMainAppLoop::IMainAppLoop(unique_ptr<IWinApp> _win, unique_ptr<IGAPI> _gapi) :
	win(move(_win)),
	gapi(move(_gapi))
{
}

IMainAppLoop::~IMainAppLoop()
{
}

zResult IMainAppLoop::Initialize(const s_zEngineInit* const initData)
{
	return zResult();
}
