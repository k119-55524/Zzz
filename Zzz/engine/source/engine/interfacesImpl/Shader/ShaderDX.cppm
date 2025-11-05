#include "pch.h"
export module ShaderDX;

#if defined(ZRENDER_API_D3D12)
import result;
import IShader;
import IMeshGPU;
import StrConvert;

using namespace zzz;

export namespace zzz::platforms::directx
{
	export class ShaderDX final : public IShader
	{
	public:
		ShaderDX() = delete;
		explicit ShaderDX(const std::shared_ptr<IMeshGPU> mesh, std::wstring&& name);

		virtual ~ShaderDX() override = default;

		result<> InitializeByText(std::string&& srcVS, std::string&& srcPS) override;
		result<ComPtr<ID3DBlob>> CompileShaderFromSource(
			std::string&& shaderSource,
			const std::string& entryPoint,
			const std::string& target,
			const D3D_SHADER_MACRO* defines = nullptr,
			ID3DInclude* includeHandler = nullptr);

		ComPtr<ID3DBlob> GetVS() const noexcept override { return m_VS; }
		ComPtr<ID3DBlob> GetPS() const noexcept override { return m_PS; }

	private:
		ComPtr<ID3DBlob> m_VS;
		ComPtr<ID3DBlob> m_PS;
	};

	ShaderDX::ShaderDX(const std::shared_ptr<IMeshGPU> mesh, std::wstring&& name) :
		IShader(mesh, std::move(name))
	{
	}

	result<> ShaderDX::InitializeByText(std::string&& srcVS, std::string&& srcPS)
	{
		UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;

		// Для отладки добавляем дополнительные флаги
#ifdef _DEBUG
		flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

		//D3D_SHADER_MACRO defines[] =
		//{
		//	{"USE_TEXTURE", "1"},
		//	{"ENABLE_FOG", "0"},
		//	{nullptr, nullptr}		// Обязательный терминатор
		//};

		auto resVS = CompileShaderFromSource(srcVS.c_str(), std::string("mainVS"), "vs_6_8", nullptr);
		if (!resVS)
			return resVS.error();

		auto resPS = CompileShaderFromSource(srcPS.c_str(), "mainPS", "ps_6_8", nullptr);
		if (!resPS)
			return resPS.error();

		m_VS = resVS.value();
		m_PS = resPS.value();

		return {};
	}

	result<ComPtr<ID3DBlob>> ShaderDX::CompileShaderFromSource(
		std::string&& shaderSource,
		const std::string& entryPoint,
		const std::string& target,
		const D3D_SHADER_MACRO* defines,
		ID3DInclude* includeHandler)
	{
		ComPtr<ID3DBlob> shaderBlob;
		ComPtr<ID3DBlob> errorBlob;

		UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;

		// Для отладки добавляем дополнительные флаги
#ifdef _DEBUG
		flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

		HRESULT hr = D3DCompile(
			shaderSource.c_str(),	// Указатель на исходный код
			shaderSource.size(),	// Длина исходного кода
			nullptr,				// Имя источника (для ошибок)
			defines,				// Макросы препроцессора
			includeHandler,			// Обработчик #include
			entryPoint.c_str(),		// Точка входа ("main", "VS_main", etc.)
			target.c_str(),			// Таргет ("vs_5_0", "ps_5_0", etc.)
			flags,					// Флаги компиляции
			0,						// Дополнительные флаги (устарело)
			&shaderBlob,			// Результат компиляции
			&errorBlob);			// Ошибки компиляции

		if (S_OK != hr)
		{
			if (errorBlob)
			{
				auto err = string_to_wstring((char*)errorBlob->GetBufferPointer());

				if (err)
				{
					DebugOutput(std::format(L">>>>> [ShaderDX::CompileShaderFromSource( ... )]. Error compile: {}.", err.value()));
					return Unexpected(eResult::failure, L">>>>> [ShaderDX::CompileShaderFromSource( ... )]. Undefined error");
				}
			}

			DebugOutput(std::format(L">>>>> [ShaderDX::CompileShaderFromSource( ... )]. Failed to compile shader. HRESULT = 0x{:08X}", hr).c_str());
			return Unexpected(eResult::failure, L">>>>> [ShaderDX::CompileShaderFromSource( ... )]. Undefined error");
		}

		auto res = string_to_wstring(target).value();
		DebugOutput(std::format(
			L">>>>> [ShaderDX::CompileShaderFromSource( ... )]. Shader compiled successfully. EntryPoint: {}, Target: {}.",
			string_to_wstring(entryPoint).value(),
			string_to_wstring(target).value()));

		return shaderBlob;
	}
}
#endif // defined(ZRENDER_API_D3D12)