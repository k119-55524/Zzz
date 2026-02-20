
#if defined(ZPLATFORM_MSWINDOWS)

export module PlatformConfig_MSWin;

import Result;
import Size2D;
import Version;
import Serializer;

namespace zzz
{
	class ZamlConfig;
}

export namespace zzz::core
{
	export class PlatformConfig_MSWin final : public ISerializable
	{
		friend class zzz::ZamlConfig;

	public:
		PlatformConfig_MSWin()
		{
			SetDefaults();
		}
		PlatformConfig_MSWin(std::wstring appName, std::wstring className, Size2D<LONG> winSize, std::wstring icoFullPath, int icoSize) :
			m_Version{ 0, 0, 1 },
			m_AppName{ appName },
			m_ClassName{ className },
			m_WinSize{ winSize },
			m_IcoFullPath{ icoFullPath },
			m_IcoSize{ icoSize },
			m_SupportsTearing{ true }
		{
		}
		~PlatformConfig_MSWin() = default;

		inline const Version& GetVersion() const noexcept { return m_Version; }
		inline const std::wstring& GetAppName() const noexcept { return m_AppName; }
		inline const std::wstring& GetClassName() const noexcept { return m_ClassName; }
		inline const Size2D<LONG>& GetWinSize() const noexcept { return m_WinSize; }
		inline const std::wstring& GetIcoFullPath() const noexcept { return m_IcoFullPath; }
		inline int GetIcoSize() const noexcept { return m_IcoSize; }
		inline bool IsSupportsTearing() const noexcept { return m_SupportsTearing; }

	private:
		Version m_Version;
		std::wstring m_AppName;
		std::wstring m_ClassName;
		Size2D<LONG> m_WinSize;
		std::wstring m_IcoFullPath;
		int m_IcoSize;
		bool m_SupportsTearing;	// Можно ли отключать Vsync

		void SetDefaults()
		{
			m_AppName = L"zGame3D";
			m_ClassName = L"zEngineClassName";
			m_WinSize = Size2D<LONG>(800, 600);
			m_IcoFullPath = L"";
			m_IcoSize = 32;
			m_SupportsTearing = true;
		}

		Result<> Serialize(std::vector<std::byte>& buffer, const zzz::Serializer& s) const override
		{
			return s.Serialize(buffer, m_AppName)
				.and_then([&]() { return s.Serialize(buffer, m_ClassName); })
				.and_then([&]() { return s.Serialize(buffer, m_WinSize); })
				.and_then([&]() { return s.Serialize(buffer, m_IcoFullPath); })
				.and_then([&]() { return s.Serialize(buffer, m_IcoSize); })
				.and_then([&]() { return s.Serialize(buffer, m_SupportsTearing); });
		}

		Result<> DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, const zzz::Serializer& s) override
		{
			return s.DeSerialize(buffer, offset, m_AppName)
				.and_then([&]() { return s.DeSerialize(buffer, offset, m_ClassName); })
				.and_then([&]() { return s.DeSerialize(buffer, offset, m_WinSize); })
				.and_then([&]() { return s.DeSerialize(buffer, offset, m_IcoFullPath); })
				.and_then([&]() { return s.DeSerialize(buffer, offset, m_IcoSize); })
				.and_then([&]() { return s.DeSerialize(buffer, offset, m_SupportsTearing); });
		}
	};
}
#endif