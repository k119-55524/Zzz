#include "pch.h"
#include "DirectX12API.h"

using namespace Zzz;
using namespace Zzz::Platforms;

#ifdef _GAPI_DX12

DirectX12API::DirectX12API(const shared_ptr<IWinApp> appWin) :
	IGAPI(appWin, eGAPIType::DirectX12)//,
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
	UINT dxgiFactoryFlags = 0;

#if _DEBUG
	// Включаем уровень отладки.
	//{
	//	ComPtr<ID3D12Debug> debugController;
	//	if (S_OK == D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))
	//	{
	//		debugController->EnableDebugLayer();

	//		// Включить дополнительные уровни отладки.
	//		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	//	}
	//}
#endif

	//ComPtr<IDXGIFactory4> factory;
	//if (S_OK != CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)))
	//	THROW_RUNTIME_ERROR("");

	//ComPtr<IDXGIAdapter1> adapter;
	//GetAdapter(factory.Get(), &adapter);

	//if (S_OK != D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device)))
	//	THROW_RUNTIME_ERROR("");

	zSize winSize = appWin->GetWinSize();

	return zResult();
}

void DirectX12API::OnUpdate()
{
}

void DirectX12API::OnRender()
{
}

void DirectX12API::OnResize(const zSize& size)
{
}

//void DirectX12API::GetAdapter(IDXGIFactory1* pFactory, IDXGIAdapter1** ppAdapter)
//{
//	*ppAdapter = nullptr;
//	ComPtr<IDXGIAdapter1> adapter;
//	ComPtr<IDXGIFactory6> factory6;
//	if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
//	{
//		for (
//			UINT adapterIndex = 0;
//			SUCCEEDED(factory6->EnumAdapterByGpuPreference(
//				adapterIndex,
//				DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
//				IID_PPV_ARGS(&adapter)));
//				++adapterIndex)
//		{
//			DXGI_ADAPTER_DESC1 desc;
//			adapter->GetDesc1(&desc);
//
//#ifdef _DEBUG
//			OutputDebugString(L">>>>> [DirectX12API::GetAdapter()]. EnumAdapterByGpuPreference: ");
//			OutputDebugString(desc.Description);
//			OutputDebugString(L"\n");
//#endif
//
//			// Пропускаем Basic Render Driver адаптер.
//			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
//				continue;
//
//			// Проверяем, поддерживает ли адаптер Direct3D 12(без фактического создания адаптера)
//			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr)))
//				break;
//		}
//	}
//
//	// Если подходящий адаптер создать не удалось пытаемся найти другим способом
//	if (adapter.Get() == nullptr)
//	{
//		for (
//			UINT adapterIndex = 0;
//			SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter));
//			++adapterIndex)
//		{
//			DXGI_ADAPTER_DESC1 desc;
//			adapter->GetDesc1(&desc);
//
//			OutputDebugString(_T(">>>>> [DirectX12API::GetAdapter()]. EnumAdapters1: "));
//			OutputDebugString(desc.Description);
//			OutputDebugString(_T("\n"));
//
//			// Пропускаем Basic Render Driver адаптер.
//			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
//				continue;
//
//			// Проверяем, поддерживает ли адаптер Direct3D 12(без фактического создания адаптера)
//			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
//				break;
//		}
//	}
//
//	//{
//	//	IDXGIAdapter3* pDXGIAdapter3;
//	//	ThrowIfFailed(adapter->QueryInterface(IID_PPV_ARGS(&pDXGIAdapter3)));
//
//	//	// Now I can query video/system memory usage.
//	//	DXGI_QUERY_VIDEO_MEMORY_INFO vm_info;
//	//	ThrowIfFailed(pDXGIAdapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &vm_info));
//	//	ThrowIfFailed(pDXGIAdapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL, &vm_info));
//	//	int i = 0;
//	//	i++;
//	//}
//
//	*ppAdapter = adapter.Detach();
//}

#endif // _GAPI_DX12