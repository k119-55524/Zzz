
export module StartupConfig;

import Result;
import GAPIConfig;
import Serializer;
import PlatformConfig;

using namespace zzz::core;

namespace zzz
{
	export class StartupConfig final : public ISerializable
	{
	public:
		StartupConfig() = default;
		explicit StartupConfig(const std::shared_ptr<PlatformConfig>& appWinConfig, const std::shared_ptr<GAPIConfig>& gapiConfig) :
			m_AppWinConfig{ appWinConfig },
			m_GAPIConfig{gapiConfig}
		{
		}
		~StartupConfig() override
		{
		};

		std::shared_ptr<PlatformConfig> GetAppWinConfig() const noexcept { return m_AppWinConfig; }
		std::shared_ptr<GAPIConfig> GetGAPIConfig() const noexcept { return m_GAPIConfig; }

	protected:
		std::shared_ptr<PlatformConfig> m_AppWinConfig;
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
