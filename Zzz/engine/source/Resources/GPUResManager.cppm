
#include "pch.h"

export module GPUResManager;

import IPSO;
import IGAPI;
import Result;
import PSO_DX;
import IShader;
import IMeshGPU;
import Material;
import CPUResManager;
import Shader_DirectX;
import MeshGPU_DirectX;

using namespace zzz::directx;
using namespace zzz::core;

namespace zzz
{
#if defined(ZRENDER_API_D3D12)
	typedef MeshGPU_DirectX MeshGPU;
	typedef Shader_DirectX Shader;
	typedef PSO_DX PSO;
#else
#error ">>>>> [Compile error]. This branch requires implementation for the current platform"
#endif
}

export namespace zzz
{
	export class GPUResManager final
	{
		Z_NO_CREATE_COPY(GPUResManager);

	public:
		GPUResManager(std::shared_ptr<IGAPI> _IGAPI, std::shared_ptr<CPUResManager> resCPU);
		~GPUResManager() = default;

		Result<std::shared_ptr<IMeshGPU>> GetGenericMesh(MeshType type);
		Result<std::shared_ptr<Material>> GetGenericMaterial(const std::shared_ptr<IMeshGPU> mesh);

	private:
		Result<std::shared_ptr<IPSO>> GetGenericPSO(const std::shared_ptr<IMeshGPU> mesh);
		Result<std::shared_ptr<IShader>> GetGenericShader(const std::shared_ptr<IMeshGPU> mesh, std::wstring&& name);

		std::shared_ptr<IGAPI> m_GAPI;
		std::shared_ptr<CPUResManager> m_ResCPU;
	};

	GPUResManager::GPUResManager(std::shared_ptr<IGAPI> _IGAPI, std::shared_ptr<CPUResManager> resCPU) :
		m_GAPI{ _IGAPI },
		m_ResCPU{ resCPU }
	{
		ensure(m_ResCPU, ">>>>> [GPUResManager::GPUResManager()]. Resource system CPU cannot be null.");
	}

	Result<std::shared_ptr<IMeshGPU>> GPUResManager::GetGenericMesh(MeshType type)
	{
		auto meshCPU = m_ResCPU->GetGenericMesh(type);
		if (!meshCPU)
			return meshCPU.error();

		std::shared_ptr<IMeshGPU> meshGPU = safe_make_shared<MeshGPU>(meshCPU.value());
		Result<> res = meshGPU->Initialize(m_GAPI);
		if (!res)
			return Unexpected(res.error());

		return meshGPU;
	}

	Result<std::shared_ptr<Material>> GPUResManager::GetGenericMaterial(const std::shared_ptr<IMeshGPU> mesh)
	{
		Result<std::shared_ptr<IPSO>> pso = GetGenericPSO(mesh);
		if (!pso)
			return pso.error();

		std::shared_ptr<Material> material = safe_make_shared<Material>(pso.value());
		if (!material)
			return Unexpected(eResult::no_make_shared_ptr, L">>>>> [GPUResManager::GetGenericMaterial()]. Failed to create Material.");

		return material;
	}

	Result<std::shared_ptr<IPSO>> GPUResManager::GetGenericPSO(const std::shared_ptr<IMeshGPU> mesh)
	{
		Result<std::shared_ptr<IShader>> shader = GetGenericShader(mesh, L"GenShader");
		if (!shader)
			return shader.error();

		std::shared_ptr<IPSO> pso = safe_make_shared<PSO>(m_GAPI, shader.value(), mesh->GetInputLayout());
		if (!pso)
			return Unexpected(eResult::no_make_shared_ptr, L">>>>> [GPUResManager::GetGenericPSO()]. Failed to create IPSO.");

		return pso;
	}

	Result<std::shared_ptr<IShader>> GPUResManager::GetGenericShader(const std::shared_ptr<IMeshGPU> mesh, std::wstring&& name)
	{
		std::shared_ptr<IShader> shader = safe_make_shared<Shader>(m_GAPI, mesh, std::move(name));

		// Исходный код шейдера в виде строки
		std::string vs = R"(
			cbuffer cbPerObject : register(b0)
			{
				row_major float4x4 gWorldViewProj;
				row_major float4x4 view;
				row_major float4x4 proj;
				row_major float4x4 viewProj;

				float4 cameraPos;
			};

			cbuffer cbMaterial : register(b1)
			{
				float4 gBaseColor;
				float  gRoughness;
				float  gMetallic;
			};

			cbuffer cbObject : register(b2)
			{
				row_major float4x4 gWorld;
				row_major float4x4 gWorldViewProj1;
			};

			struct VertexIn
			{
				float3 Pos  : POSITION;
				float4 Color : COLOR;
			};

			struct VertexOut
			{
				float4 Pos  : SV_POSITION;
				float4 Color : COLOR;
			};

			VertexOut mainVS(VertexIn vin)
			{
				VertexOut vout;
	
				// Transform to homogeneous clip space.
				vout.Pos = mul(float4(vin.Pos, 1.0f), gWorldViewProj1);
	
				// Just pass vertex color into the pixel shader.
				vout.Color = vin.Color;

				return vout;
			}
		)";

		std::string ps = R"(
			struct VertexOut
			{
				float4 Pos  : SV_POSITION;
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