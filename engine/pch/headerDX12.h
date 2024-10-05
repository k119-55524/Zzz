#pragma once

#ifdef _GAPI_DX12

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dCompiler.lib")
//#pragma comment(lib, "comsuppwd.lib")

//#include <comdef.h>

#include <wrl.h>
#include <d3d12.h>
#include <d3d11_1.h>
#include <dxgi1_6.h>
#include <SDKDDKVer.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

using namespace DirectX;
using namespace Microsoft::WRL;

#define NOMINMAX

#endif // _GAPI_DX12