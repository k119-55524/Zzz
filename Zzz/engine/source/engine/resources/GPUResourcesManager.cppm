#include "pch.h"
export module GPUResourcesManager;

import IGAPI;
import result;
import IShader;
import IMeshGPU;
import ShaderDX;
import GPUMeshDX;
import CPUResourcesManager;

using namespace zzz::platforms;
using namespace zzz::platforms::directx;

namespace zzz
{
#if defined(_WIN64)
	typedef GPUMeshDX GPUMesh;
	typedef ShaderDX Shader;
#else
#error ">>>>> [Compile error]. This branch requires implementation for the current platform"
#endif
}

export namespace zzz
{
	export class GPUResourcesManager final
	{
	public:
		GPUResourcesManager() = delete;
		GPUResourcesManager(const GPUResourcesManager&) = delete;
		GPUResourcesManager(GPUResourcesManager&&) = delete;
		GPUResourcesManager(std::shared_ptr<IGAPI> _IGAPI, std::shared_ptr<CPUResourcesManager> resCPU);

		~GPUResourcesManager() = default;

		result<std::shared_ptr<IMeshGPU>> GetGenericMesh(MeshType type);
		result<std::shared_ptr<IShader>> GetGenericShader(std::wstring&& name);


	private:
		std::shared_ptr<IGAPI> m_GAPI;
		std::shared_ptr<CPUResourcesManager> m_ResCPU;
	};

	GPUResourcesManager::GPUResourcesManager(std::shared_ptr<IGAPI> _IGAPI, std::shared_ptr<CPUResourcesManager> resCPU) :
		m_GAPI{ _IGAPI },
		m_ResCPU{ resCPU }
	{
		ensure(m_ResCPU, ">>>>> [GPUResourcesManager::GPUResourcesManager()]. Resource system CPU cannot be null.");
	}

	result<std::shared_ptr<IMeshGPU>> GPUResourcesManager::GetGenericMesh(MeshType type)
	{
		auto meshCPU = m_ResCPU->GetGenericMesh(type);
		if (!meshCPU)
			return meshCPU.error();

		std::shared_ptr<IMeshGPU> meshGPU = safe_make_shared<GPUMesh>(meshCPU.value());
		result<> res = meshGPU->Initialize(m_GAPI);
		if (!res)
			return Unexpected(res.error());

		return meshGPU;
	}

	result<std::shared_ptr<IShader>> GPUResourcesManager::GetGenericShader(std::wstring&& name)
	{
		std::shared_ptr<IShader> shader = safe_make_shared<Shader>(std::move(name));

		// Исходный код шейдера в виде строки
		std::string vs = R"(
			cbuffer cbPerObject : register(b0)
			{
				float4x4 gWorldViewProj; 
			};

			struct VertexIn
			{
				float3 PosL  : POSITION;
				float4 Color : COLOR;
			};

			struct VertexOut
			{
				float4 PosH  : SV_POSITION;
				float4 Color : COLOR;
			};

			VertexOut mainVS(VertexIn vin)
			{
				VertexOut vout;
	
				// Transform to homogeneous clip space.
				vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	
				// Just pass vertex color into the pixel shader.
				vout.Color = vin.Color;

				return vout;
			}
		)";

		std::string ps = R"(
			struct VertexOut
			{
				float4 PosH  : SV_POSITION;
				float4 Color : COLOR;
			};

			float4 mainPS(VertexOut pin) : SV_Target
			{
				return pin.Color;
			}
		)";

		shader->InitializeByText(std::move(vs), std::move(ps));

		return shader;
	}
}