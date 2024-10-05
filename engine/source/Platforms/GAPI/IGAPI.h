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

		IGAPI(const shared_ptr<IWinApp> _appWin, eGAPIType type);
		virtual ~IGAPI() = 0;

		virtual zResult Initialize(const DataEngineInitialization& data) = 0;
		void Update();
		void Resize(const zSize& size);

	protected:
		eGAPIType gapiType;
		e_InitState initState;

		shared_ptr<IWinApp> appWin;

		virtual void OnUpdate() = 0;
		virtual void OnRender() = 0;
		virtual void OnResize(const zSize& size) = 0;
	};
}
