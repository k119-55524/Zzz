#include "pch.h"
#include "DirectX12API.h"

using namespace Zzz;
using namespace Zzz::Platforms;

#ifdef _WINDOWS

DirectX12API::DirectX12API() :
	IGAPI(eGAPIType::DirectX12)//,
	//m_frameIndex{ 0 },
	//m_fenceEvent{ 0 },
	//m_fenceValue{ 0 },
	//m_aspectRatio{ 0 },
	//m_RtvDescrSize{ 0 },
	//m_DsvDescrSize{ 0 },
	//m_CbvSrvDescrSize{ 0 }
{
}

DirectX12API::~DirectX12API()
{
	// Ensure that the GPU is no longer referencing resources that are about to be
	// cleaned up by the destructor.
	//WaitForPreviousFrame();

	//CloseHandle(m_fenceEvent);
}

zResult DirectX12API::Initialize(const DataEngineInitialization& data)
{
	zResult res;

	return res;
}

void DirectX12API::OnUpdate()
{
}

void DirectX12API::OnRender()
{
}

void Zzz::Platforms::DirectX12API::OnResize(const zSize& size)
{
}

#endif // _WINDOWS