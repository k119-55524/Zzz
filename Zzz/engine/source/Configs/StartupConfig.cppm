
export module StartupConfig;

import GAPIConfig;
import Serializer;
import AppConfig;

using namespace zzz::core;

namespace zzz
{
	export class StartupConfig final : public ISerializable
	{
	public:
		StartupConfig() = default;
		explicit StartupConfig(std::shared_ptr<AppConfig> appWinConfig, std::shared_ptr<GAPIConfig> gapionfig) :
			m_AppWinConfig{ appWinConfig },
			m_GAPIConfig{gapionfig}
		{
		}
		~StartupConfig() = default;


		const std::shared_ptr<AppConfig> GetAppWinConfig() const noexcept { return m_AppWinConfig; }
		const std::shared_ptr<GAPIConfig> GetGAPIConfig() const noexcept { return m_GAPIConfig; }

	protected:
		std::shared_ptr<AppConfig> m_AppWinConfig;
		std::shared_ptr<GAPIConfig> m_GAPIConfig;

		Result<> Serialize(std::vector<std::byte>& buffer, const zzz::Serializer& s) const override
		{
			return s.Serialize(buffer, *m_AppWinConfig)
				.and_then([&]() { return s.Serialize(buffer, *m_GAPIConfig); });
		}

		Result<> DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, const zzz::Serializer& s) override
		{
			return s.DeSerialize(buffer, offset, *m_AppWinConfig)
				.and_then([&]() { return s.DeSerialize(buffer, offset, *m_GAPIConfig); });
		}
	};
}
