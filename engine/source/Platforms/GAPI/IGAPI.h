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

		virtual zResult Initialize(const DataEngineInitialization& data) = 0;
		void Update();
		void Resize(const zSize& size);

	protected:
		eGAPIType gapiType;
		e_InitState initState;

		virtual void OnUpdate() = 0;
		virtual void OnRender() = 0;
		virtual void OnResize(const zSize& size) = 0;
	};
}
