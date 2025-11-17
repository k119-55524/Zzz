
#if defined(ZPLATFORM_MSWINDOWS)
export module AppWinConfig_MSWin;

import Result;
import Size2D;
import ioZaml;
import Serializer;
import IAppWinConfig;

namespace zzz
{
	class ZamlProcessor;
}

export namespace zzz::core
{
	export class AppWinConfig_MSWin final : public IAppWinConfig, public ISerializable
	{
		friend class zzz::ZamlProcessor;

	public:
		AppWinConfig_MSWin()
		{
			SetDefaults();
		}
		AppWinConfig_MSWin(std::wstring caption, std::wstring className, Size2D<LONG> winSize, std::wstring icoFullPath, int icoSize) :
			m_Caption(caption),
			m_ClassName(className),
			m_WinSize(winSize),
			m_IcoFullPath(icoFullPath),
			m_IcoSize(icoSize)
		{
		}
		~AppWinConfig_MSWin() = default;

		const std::wstring& GetCaption() const noexcept { return m_Caption; }
		const std::wstring& GetClassName() const noexcept { return m_ClassName; }
		const Size2D<LONG>& GetWinSize() const noexcept { return m_WinSize; }
		const std::wstring& GetIcoFullPath() const noexcept { return m_IcoFullPath; }
		int GetIcoSize() const noexcept { return m_IcoSize; }

	protected:
		Result<> Configure(std::shared_ptr<ZamlProcessor> zamlData) override;

	private:
		std::wstring m_Caption;
		std::wstring m_ClassName;
		Size2D<LONG> m_WinSize;
		std::wstring m_IcoFullPath;
		int m_IcoSize;

		void SetDefaults()
		{
			m_Caption = L"zGame3D";
			m_ClassName = L"zEngineClassName";
			m_WinSize = Size2D<LONG>(800, 600);
			m_IcoFullPath = L"";
			m_IcoSize = 32;
		}

		Result<> Serialize(std::vector<std::byte>& buffer, const zzz::core::Serializer& s) const override
		{
			return s.Serialize(buffer, m_Caption)
				.and_then([&]() { return s.Serialize(buffer, m_ClassName); })
				.and_then([&]() { return s.Serialize(buffer, m_WinSize); })
				.and_then([&]() { return s.Serialize(buffer, m_IcoFullPath); })
				.and_then([&]() { return s.Serialize(buffer, m_IcoSize); });
		}

		Result<> DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, const zzz::core::Serializer& s) override
		{
			return s.DeSerialize(buffer, offset, m_Caption)
				.and_then([&]() { return s.DeSerialize(buffer, offset, m_ClassName); })
				.and_then([&]() { return s.DeSerialize(buffer, offset, m_WinSize); })
				.and_then([&]() { return s.DeSerialize(buffer, offset, m_IcoFullPath); })
				.and_then([&]() { return s.DeSerialize(buffer, offset, m_IcoSize); });
		}
	};

	Result<> AppWinConfig_MSWin::Configure(std::shared_ptr<ZamlProcessor> zamlData)
	{
		auto res = zamlData->GetParam<std::wstring>(L"Caption")
			.and_then([&](std::wstring name) { m_Caption = name; });

		if (!res)
			return Unexpected(eResult::failure, L">>>>> [SW_MSWindows.Initialize( ... )]. GetParam( ... ). Failed to get 'Caption' parameter.");

		//m_Settings->GetParam<std::wstring>(L"MSWin_SpecSettings", L"ClassName")
		//	.and_then([&](std::wstring name) {ClassName = name; })
		//	.or_else([&](auto error) { ClassName = m_Caption; });

		//LONG swWidth;
		//auto res = m_Settings->GetParam<int>(L"Width")
		//	.and_then([&](int width) {swWidth = width; });
		//if (!res)
		//	return Unexpected(eResult::failure, L">>>>> [SW_MSWindows.Initialize( ... )]. GetParam( ... ). Failed to get 'Width' parameter. More specifically: " + res.error().getMessage());

		//LONG swHeight;
		//res = m_Settings->GetParam<int>(L"Height")
		//	.and_then([&](int height) {swHeight = height; });
		//if (!res)
		//	return Unexpected(eResult::failure, L">>>>> [SW_MSWindows.Initialize( ... )]. GetParam( ... ). Failed to get 'Height' parameter. More specifically:" + res.error().getMessage());

		//HICON iconHandle = nullptr;
		//{
		//	Result<std::wstring> icoPath = m_Settings->GetParam<std::wstring>(L"IcoFullPath");
		//	if (icoPath)
		//	{
		//		ibMSWin icoBuilder;
		//		auto res = icoBuilder.LoadIco(icoPath.value(), m_Settings->GetParam<int>(L"IcoSize").value_or(32));
		//		if (res)
		//			iconHandle = res.value();
		//	}
		//}

		return {};
	}
}
#endif