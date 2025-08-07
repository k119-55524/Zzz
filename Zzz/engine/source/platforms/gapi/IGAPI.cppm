#include "pch.h"
export module IGAPI;

import result;
import zSize2D;
import ISuperWidget;

using namespace zzz::result;

namespace zzz
{
	class engine;
}

export namespace zzz::platforms
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

		IGAPI& operator=(const IGAPI&) = delete;

		explicit IGAPI(eGAPIType type);
		virtual ~IGAPI() = 0;

	protected:
		virtual zResult<> Initialize(const std::shared_ptr<ISuperWidget> appWin) = 0;
		friend class zzz::engine;

		eGAPIType gapiType;
		eInitState initState;

		virtual void OnUpdate() = 0;
		virtual void OnRender() = 0;
		virtual void OnResize(const zSize2D<>& size) = 0;
	};

	IGAPI::IGAPI(eGAPIType type) :
		gapiType{ type },
		initState{ eInitState::eInitNot }
	{
	}

	IGAPI::~IGAPI()
	{
	}
}