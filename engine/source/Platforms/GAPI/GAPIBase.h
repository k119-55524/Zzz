#pragma once

#include "../WinApplication/WinAppBase.h"

using namespace Zzz;

namespace Zzz::Platforms
{
	enum eGAPIType
	{
		DirectX12,
		OpenGL,
		Vulkan,
		Metal
	};

	class GAPIBase
	{
	public:
		GAPIBase() = delete;
		GAPIBase(GAPIBase&) = delete;
		GAPIBase(GAPIBase&&) = delete;

		GAPIBase(unique_ptr<WinAppBase> _win, eGAPIType type);
		virtual ~GAPIBase() = 0;

		virtual zResult Initialize(const s_zEngineInit* const data) = 0;
		void Update();

	protected:
		eGAPIType gapiType;
		//shared_ptr<IGAPI> gapiInterfaces;
		unique_ptr<WinAppBase> win;

		virtual void OnUpdate() = 0;
		virtual void OnRender() = 0;
	};
}
