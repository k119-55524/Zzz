#pragma once

#include "../WinApplication/IWinApp.h"

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

	class IGAPI
	{
	public:
		IGAPI() = delete;
		IGAPI(IGAPI&) = delete;
		IGAPI(IGAPI&&) = delete;

		IGAPI(eGAPIType type);
		virtual ~IGAPI() = 0;

		virtual zResult Initialize(const s_zEngineInit* const data) = 0;
		void Update();

	protected:
		eGAPIType gapiType;
		//shared_ptr<IGAPI> gapiInterfaces;

		virtual void OnUpdate() = 0;
		virtual void OnRender() = 0;
	};
}
