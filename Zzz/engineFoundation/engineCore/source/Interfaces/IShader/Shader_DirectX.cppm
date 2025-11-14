
#include "pch.h"

export module Shader_DirectX;

#if defined(ZRENDER_API_D3D12)
import IGAPI;
import Result;
import IShader;
import IMeshGPU;
import StrConvert;

using namespace zzz::core;

export namespace zzz::directx
{
	export class Shader_DirectX final : public IShader
	{
	public:
		Shader_DirectX() = delete;
		explicit Shader_DirectX(const std::shared_ptr<IGAPI> gapi, const std::shared_ptr<IMeshGPU> mesh, std::wstring&& name);
		virtual ~Shader_DirectX() override = default;

		Result<> InitializeByText(std::string&& srcVS, std::string&& srcPS) override;
		Result<ComPtr<ID3DBlob>> CompileShaderFromSource(
			std::string&& shaderSource,
			const std::wstring& entryPoint,
			const std::wstring& target,
			const D3D_SHADER_MACRO* defines = nullptr,
			ID3DInclude* includeHandler = nullptr);

		ComPtr<ID3DBlob> GetVS() const noexcept override { return m_VS; }
		ComPtr<ID3DBlob> GetPS() const noexcept override { return m_PS; }

	private:
		ComPtr<ID3DBlob> m_VS;
		ComPtr<ID3DBlob> m_PS;
	};

	Shader_DirectX::Shader_DirectX(const std::shared_ptr<IGAPI> gapi, const std::shared_ptr<IMeshGPU> mesh, std::wstring&& name) :
		IShader(gapi, mesh, std::move(name))
	{}

	Result<> Shader_DirectX::InitializeByText(std::string&& srcVS, std::string&& srcPS)
	{
		//D3D_SHADER_MACRO defines[] =
		//{
		//	{"USE_TEXTURE", "1"},
		//	{"ENABLE_FOG", "0"},
		//	{nullptr, nullptr}		// Обязательный терминатор
		//};

		auto resVS = CompileShaderFromSource(srcVS.c_str(), L"mainVS", m_GAPI->GetHighestShaderModelAsString(eShaderType::Vertex));
		if (!resVS)
			return resVS.error();

		auto resPS = CompileShaderFromSource(srcPS.c_str(), L"mainPS", m_GAPI->GetHighestShaderModelAsString(eShaderType::Pixel));
		if (!resPS)
			return resPS.error();

		m_VS = resVS.value();
		m_PS = resPS.value();

		return {};
	}

	Result<ComPtr<ID3DBlob>> Shader_DirectX::CompileShaderFromSource(
		std::string&& shaderSource,
		const std::wstring& entryPoint,
		const std::wstring& target,
		const D3D_SHADER_MACRO* defines,
		ID3DInclude* includeHandler)
	{
		ComPtr<IDxcLibrary> library;
		ComPtr<IDxcCompiler> compiler;
		ComPtr<IDxcBlobEncoding> sourceBlob;
		ComPtr<IDxcOperationResult> Result;
		HRESULT hr;

		// === 1. Инициализация DXC ===
		hr = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&library));
		if (FAILED(hr))
			return Unexpected(eResult::failure, L">>>>> [haderDX::CompileShaderFromSource( ... )]. Failed to create DXC library");

		hr = library->CreateBlobWithEncodingFromPinned(shaderSource.c_str(), (UINT32)shaderSource.size(), CP_UTF8, &sourceBlob);
		if (FAILED(hr))
			return Unexpected(eResult::failure, L">>>>> [haderDX::CompileShaderFromSource( ... )]. Failed to create source blob");

		hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));
		if (FAILED(hr))
			return Unexpected(eResult::failure, L">>>>> [haderDX::CompileShaderFromSource( ... )]. Failed to create DXC compiler");

		// === 2. Аргументы компиляции ===
		std::vector<LPCWSTR> args = {
			L"-E", entryPoint.c_str(),
			L"-T", target.c_str(),
			#ifdef _DEBUG
				L"-Zi", L"-Od",
			#else
				L"-O3",
			#endif
			L"-WX",
		};

		// === 3. Макросы ===
		std::vector<std::wstring> defineStrings;
		std::vector<DxcDefine> dxcDefines;
		if (defines)
		{
			for (int i = 0; defines[i].Name; ++i)
			{
				auto name = string_to_wstring(defines[i].Name).value();
				auto def = string_to_wstring(defines[i].Definition).value();
				defineStrings.emplace_back(std::move(name));
				defineStrings.emplace_back(std::move(def));

				dxcDefines.push_back(
					{
						defineStrings[defineStrings.size() - 2].c_str(),
						defineStrings[defineStrings.size() - 1].c_str()
					});
			}
		}

		// === 4. Компиляция ===
		hr = compiler->Compile(
			sourceBlob.Get(),
			L"null.hlsl",
			entryPoint.c_str(),
			target.c_str(),
			args.data(), (UINT32)args.size(),
			dxcDefines.data(), (UINT32)dxcDefines.size(),
			nullptr, // includeHandler — можно реализовать позже
			&Result);

		// Проверяем только что вызов выполнен, не результат компиляции
		if (FAILED(hr))
			return Unexpected(eResult::failure, L">>>>> [haderDX::CompileShaderFromSource( ... )]. DXC Compile call failed");

		// === 5. Проверка результата компиляции ===
		HRESULT compileStatus;
		hr = Result->GetStatus(&compileStatus);
		if (FAILED(hr))
			return Unexpected(eResult::failure, L">>>>> [haderDX::CompileShaderFromSource( ... )]. Failed to get compilation status");

		if (FAILED(compileStatus))
		{
			ComPtr<IDxcBlobEncoding> errorBlob;
			hr = Result->GetErrorBuffer(&errorBlob);
			if (SUCCEEDED(hr) && errorBlob && errorBlob->GetBufferSize() > 0)
			{
				std::string errors(static_cast<const char*>(errorBlob->GetBufferPointer()),
					errorBlob->GetBufferSize());
				auto werr = string_to_wstring(errors);
				if (werr.has_value())
					DebugOutput(std::format(L">>>>> [haderDX::CompileShaderFromSource( ... )]. [DXC] Compile error: {}", werr.value()));
				else
					DebugOutput(L">>>>> [haderDX::CompileShaderFromSource( ... )]. Compile error: Failed to convert error message");
			}
			else
				DebugOutput(L">>>>> [haderDX::CompileShaderFromSource( ... )]. Compile error: No error details available");

			return Unexpected(eResult::failure, L">>>>> [haderDX::CompileShaderFromSource( ... )]. Shader compilation failed");
		}

		// === 6. Получение результата ===
		ComPtr<IDxcBlob> shaderBlob;
		hr = Result->GetResult(&shaderBlob);
		if (FAILED(hr) || !shaderBlob)
			return Unexpected(eResult::failure, L">>>>> [haderDX::CompileShaderFromSource( ... )]. Failed to get compiled shader blob");

		// === 7. Конвертация в ID3DBlob ===
		ComPtr<ID3DBlob> finalBlob;
		hr = shaderBlob.As(&finalBlob);
		if (FAILED(hr) || !finalBlob)
			return Unexpected(eResult::failure, L">>>>> [haderDX::CompileShaderFromSource( ... )]. Failed to convert to ID3DBlob");

		DebugOutput(std::format(L">>>>> [haderDX::CompileShaderFromSource( ... )]. Shader '{}' compiled: OK. Entry: {}, Target: {}", m_Name, entryPoint, target));

		return finalBlob;
	}
}
#endif // defined(ZRENDER_API_D3D12)