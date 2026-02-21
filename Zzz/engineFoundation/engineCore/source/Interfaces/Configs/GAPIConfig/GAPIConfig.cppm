
export module GAPIConfig;

import Result;
import Ensure;
import Version;
import Serializer;
import PlatformConfig;

using namespace zzz;

namespace zzz::core
{
	export class GAPIConfig final : public ISerializable
	{
	public:
		GAPIConfig() = delete;
		GAPIConfig(const std::shared_ptr<PlatformConfig>& appConfig) :
			m_AppConfig{ appConfig },
			m_VSyncEnabledOnStartup{ false }
		{
			ensure(m_AppConfig, "PlatfotmConfig cannot be null.");
		}
		~GAPIConfig() = default;

		inline const Version& GetAppVersion() const noexcept { return m_AppConfig->GetVersion(); }
		inline const std::wstring& GetAppName() const noexcept { return m_AppConfig->GetAppName(); }

		inline bool GetVSyncEnabledOnStartup() const noexcept { return m_VSyncEnabledOnStartup; };
		inline void SetVSyncEnabledOnStartup(bool vss) noexcept { m_VSyncEnabledOnStartup = vss; };

	private:
		[[nodiscard]] Result<> Serialize(std::vector<std::byte>& buffer, const zzz::Serializer& s) const override;
		[[nodiscard]] Result<> DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, const zzz::Serializer& s) override;

		const std::shared_ptr<PlatformConfig> m_AppConfig;

		bool m_VSyncEnabledOnStartup;
	};

	[[nodiscard]] Result<> GAPIConfig::Serialize(std::vector<std::byte>& buffer, const zzz::Serializer& s) const
	{
		return s.Serialize(buffer, m_VSyncEnabledOnStartup);
	}

	[[nodiscard]] Result<> GAPIConfig::DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, const zzz::Serializer& s)
	{
		return s.DeSerialize(buffer, offset, m_VSyncEnabledOnStartup);
	}
}