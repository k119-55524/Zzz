#include "pch.h"
#include "GAPIBase.h"

using namespace Zzz::Platforms;

GAPIBase::GAPIBase(unique_ptr<WinAppBase> _win, eGAPIType type) :
	gapiType{ type }
{
#if defined(_DEBUG)
	assert(_win.get() != nullptr);
#endif

	win = std::move(_win);
}

GAPIBase::~GAPIBase()
{}

void GAPIBase::Update()
{
	OnUpdate();
	OnRender();
}