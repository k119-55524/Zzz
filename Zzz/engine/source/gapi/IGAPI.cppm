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

		explicit IGAPI(const std::shared_ptr<ISuperWidget> _appWin, eGAPIType type);
		virtual ~IGAPI() = 0;

	protected:
		virtual zResult<> Initialize() = 0;
		friend class zzz::engine;

		eGAPIType gapiType;
		eInitState initState;

		std::shared_ptr<ISuperWidget> appWin;

		virtual void OnUpdate() = 0;
		virtual void OnRender() = 0;
		virtual void OnResize(const zSize2D<>& size) = 0;
	};

	IGAPI::IGAPI(const std::shared_ptr<ISuperWidget> _appWin, eGAPIType type) :
		appWin{ _appWin },
		gapiType{ type },
		initState{ eInitState::eInitNot }
	{
		ensure(appWin);
	}

	IGAPI::~IGAPI()
	{
	}
}