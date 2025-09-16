#include "pch.h"
export module IPSO;

import IShader;

namespace zzz
{
	export class IPSO
	{
	public:
		IPSO() = delete;
		explicit IPSO(const std::shared_ptr<IShader> _shader);
		virtual ~IPSO() = default;

#if defined(RENDER_API_D3D12)

#elif defined(RENDER_API_VULKAN)
#elif defined(RENDER_API_METAL)
#else
#error ">>>>> [Compile error]. This branch requires implementation for the current platform"
#endif

	protected:
		const std::shared_ptr<IShader> shader;
	};

	IPSO::IPSO(const std::shared_ptr<IShader> _shader) :
		shader{ _shader }
	{
		ensure(shader != nullptr, ">>>>> [IPSO::IPSO( ... )]. Shader pointer cannot be null.");
	}
}