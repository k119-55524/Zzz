
export module StartupConfig;

import GAPIConfig;
import Serializer;
import AppWinConfig;

namespace zzz::core
{
	export class StartupConfig final : public ISerializable
	{
	public:
		StartupConfig() = default;
		explicit StartupConfig(const AppWinConfig& appWinConfig) :
			m_AppWinConfig(appWinConfig)
		{
		}
		~StartupConfig() = default;


		const AppWinConfig& GetAppWinConfig() const noexcept { return m_AppWinConfig; }
		const GAPIConfig& GetGAPIConfig() const noexcept { return m_GAPIConfig; }

	protected:
		AppWinConfig m_AppWinConfig;
		GAPIConfig m_GAPIConfig;

		Result<> Serialize(std::vector<std::byte>& buffer, const zzz::core::Serializer& s) const override
		{
			return s.Serialize(buffer, m_AppWinConfig)
				.and_then([&]() { return s.Serialize(buffer, m_GAPIConfig); });
		}

		Result<> DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, const zzz::core::Serializer& s) override
		{
			return s.DeSerialize(buffer, offset, m_AppWinConfig)
				.and_then([&]() { return s.DeSerialize(buffer, offset, m_GAPIConfig); });
		}
	};
}
