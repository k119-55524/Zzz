#include "pch.h"
export module PSO_DX;

import IPSO;

#if defined(RENDER_API_D3D12)
namespace zzz::platforms::directx
{
	export class PSO_DX final : public IPSO
	{
	public:
		PSO_DX() = delete;
		explicit PSO_DX(const std::shared_ptr<IShader> _shader);
		virtual ~PSO_DX() override = default;

	private:
	};

	PSO_DX::PSO_DX(const std::shared_ptr<IShader> _shader) :
		IPSO(_shader)
	{
	}
}
#endif	// RENDER_API_D3D12