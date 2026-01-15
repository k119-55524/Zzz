
#if defined(ZPLATFORM_MSWINDOWS)

export module AppWinConfig_MSWin;

import Result;
import Size2D;
import Serializer;

namespace zzz
{
	class ZamlConfig;
}

export namespace zzz::core
{
	export class AppWinConfig_MSWin final : public ISerializable
	{
		friend class zzz::ZamlConfig;

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

		Result<> Serialize(std::vector<std::byte>& buffer, const zzz::Serializer& s) const override
		{
			return s.Serialize(buffer, m_Caption)
				.and_then([&]() { return s.Serialize(buffer, m_ClassName); })
				.and_then([&]() { return s.Serialize(buffer, m_WinSize); })
				.and_then([&]() { return s.Serialize(buffer, m_IcoFullPath); })
				.and_then([&]() { return s.Serialize(buffer, m_IcoSize); });
		}

		Result<> DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, const zzz::Serializer& s) override
		{
			return s.DeSerialize(buffer, offset, m_Caption)
				.and_then([&]() { return s.DeSerialize(buffer, offset, m_ClassName); })
				.and_then([&]() { return s.DeSerialize(buffer, offset, m_WinSize); })
				.and_then([&]() { return s.DeSerialize(buffer, offset, m_IcoFullPath); })
				.and_then([&]() { return s.DeSerialize(buffer, offset, m_IcoSize); });
		}
	};
}
#endif