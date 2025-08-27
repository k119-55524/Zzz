#include "pch.h"
export module MeshGPU_DX;

import MeshCPU;
import IMeshGPU;

export namespace zzz
{
	export class MeshGPU_DX final : public IMeshGPU
	{
	public:
		MeshGPU_DX() = delete;
		MeshGPU_DX(std::shared_ptr<MeshCPU> meshCPU);

		~MeshGPU_DX() = default;

	private:


		void Initialize();
	};

	MeshGPU_DX::MeshGPU_DX(std::shared_ptr<MeshCPU> meshCPU) :
		IMeshGPU(meshCPU)
	{
		
		Initialize();
	}

	void MeshGPU_DX::Initialize()
	{

	}
}