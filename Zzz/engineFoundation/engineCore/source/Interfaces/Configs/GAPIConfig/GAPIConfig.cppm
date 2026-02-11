
export module GAPIConfig;

import Result;
import Version;
import AppConfig;
import Serializer;

namespace zzz::core
{
	export class GAPIConfig final : public ISerializable
	{
	public:
		GAPIConfig() = delete;
		GAPIConfig(const std::shared_ptr<AppConfig> appConfig) :
			m_AppConfig{ appConfig }
		{
			ensure(appConfig, "AppConfig cannot be null.");
		}
		~GAPIConfig() = default;

		inline const Version& GetAppVersion() const noexcept { return m_AppConfig->GetVersion(); }
		inline const std::wstring& GetAppName() const noexcept { return m_AppConfig->GetAppName(); }

	private:
		[[nodiscard]] Result<> Serialize(std::vector<std::byte>& buffer, const zzz::Serializer& s) const override;
		[[nodiscard]] Result<> DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, const zzz::Serializer& s) override;

		const std::shared_ptr<AppConfig> m_AppConfig;
	};

	[[nodiscard]] Result<> GAPIConfig::Serialize(std::vector<std::byte>& buffer, const zzz::Serializer& s) const
	{
		return {};
	}

	[[nodiscard]] Result<> GAPIConfig::DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, const zzz::Serializer& s)
	{
		return {};
	}
}