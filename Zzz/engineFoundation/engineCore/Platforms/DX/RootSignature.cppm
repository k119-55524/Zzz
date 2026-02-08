
export module RootSignature;

#if defined(ZRENDER_API_D3D12)
import Result;
import StrConvert;

export namespace zzz::dx
{
	class RootSignature
	{
	public:
		RootSignature();
		RootSignature(RootSignature&) = delete;
		RootSignature(RootSignature&&) = delete;

		const ComPtr<ID3D12RootSignature> Get() const noexcept { return m_RootSignature; };
		Result<> Initialize(ComPtr<ID3D12Device> device);

	private:
		ComPtr<ID3D12RootSignature> m_RootSignature;
		std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();
	};

	RootSignature::RootSignature() :
		m_RootSignature{ nullptr }
	{
	}

	Result<> RootSignature::Initialize(ComPtr<ID3D12Device> device)
	{
		// Создаём таблицу дескрипторов для текстурных SRV (Shader Resource View)
		CD3DX12_DESCRIPTOR_RANGE texTable;
		texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		
		// Описание root-параметров: могут быть таблицами, константами или дескрипторами
		CD3DX12_ROOT_PARAMETER rootParams[5];
		rootParams[0].InitAsConstantBufferView(0);					// Глобальный CBV (b0)
		rootParams[1].InitAsConstantBufferView(1);					// CBV для объекта (b1)
		rootParams[2].InitAsConstantBufferView(2);					// CBV для материала (b2)
		CD3DX12_DESCRIPTOR_RANGE ranges[2];
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 128, 0);	// Таблица SRV (t0..t127)
		rootParams[3].InitAsDescriptorTable(1, &ranges[0]);
		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 16, 0);		// Таблица UAV (u0..u15)
		rootParams[4].InitAsDescriptorTable(1, &ranges[1]);

		// Получаем набор статических сэмплеров (обычно point/linear/wrap/clamp)
		auto staticSamplers = GetStaticSamplers();

		// Формируем описание root signature
		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(
			_countof(rootParams),		// количество root-параметров
			rootParams,					// массив параметров
			(UINT)staticSamplers.size(),// количество статических сэмплеров
			staticSamplers.data(),		// массив статических сэмплеров
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT // разрешаем Input Assembler Layout
		);

		// Сериализуем root signature в бинарный формат (для передачи в драйвер)
		ComPtr<ID3DBlob> serializedRootSig = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;
		HRESULT hr = D3D12SerializeRootSignature(
			&rootSigDesc,						// описание
			D3D_ROOT_SIGNATURE_VERSION_1,		// версия root signature
			serializedRootSig.GetAddressOf(),	// результат сериализации
			errorBlob.GetAddressOf());			// ошибки компиляции (если есть)

		// Проверяем успешность сериализации
		if (S_OK != hr)
		{
			if (errorBlob)
			{
				// Получаем текст ошибки из blob'а
				const char* msg = static_cast<const char*>(errorBlob->GetBufferPointer());
				std::string errorStr(msg, errorBlob->GetBufferSize());
				auto wstr = string_to_wstring(errorStr);

				std::wstring errMsg = L">>>>> [RootSignature.Initialize( ... )]. ";
				if (wstr)
					errMsg += wstr.value();

				DebugOutput(errMsg);
				return Unexpected(eResult::failure, errMsg);
			}

			return Unexpected(eResult::failure, L">>>>> [RootSignature.Initialize( ... )]. Failed to serialize root signature.");
		}

		// Создаём сам объект Root Signature на GPU
		if (S_OK != device->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(m_RootSignature.GetAddressOf())))
		{
			return Unexpected(eResult::failure, L">>>>> [RootSignature.Initialize( ... )]. Failed to create root signature.");
		}

		return {};
	}

	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> RootSignature::GetStaticSamplers()
	{
		// Обычно требуется лишь несколько вариантов сэмплеров.
		// Определяем их заранее и делаем доступными как часть корневой сигнатуры.
		const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
			0, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_POINT,		// filter
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,	// addressU
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,	// addressV
			D3D12_TEXTURE_ADDRESS_MODE_WRAP);	// addressW

		const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
			1, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_POINT,		// filter
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,	// addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,	// addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP);	// addressW

		const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
			2, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_LINEAR,	// filter
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,	// addressU
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,	// addressV
			D3D12_TEXTURE_ADDRESS_MODE_WRAP);	// addressW

		const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
			3, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_LINEAR,	// filter
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,	// addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,	// addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP);	// addressW

		const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
			4, // shaderRegister
			D3D12_FILTER_ANISOTROPIC, // filter
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,	// addressU
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,	// addressV
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,	// addressW
			0.0f,								// mipLODBias
			8);									// maxAnisotropy

		const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
			5, // shaderRegister
			D3D12_FILTER_ANISOTROPIC, // filter
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,	// addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,	// addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,	// addressW
			0.0f,								// mipLODBias
			8);									// maxAnisotropy

		return {
			pointWrap, pointClamp,
			linearWrap, linearClamp,
			anisotropicWrap, anisotropicClamp };
	}
}
#endif // defined(ZRENDER_API_D3D12)