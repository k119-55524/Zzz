
export module GAPIConfig;

import IGAPI;
import Result;
import Serializer;

namespace zzz::core
{
	export class GAPIConfig final : public ISerializable
	{
	public:
		GAPIConfig() :
			e_GAPIType{ GetCurrentGAPI() }
		{
		}
		~GAPIConfig() = default;

		constexpr eGAPIType GetGAPIType() const noexcept { return e_GAPIType; }

	private:
		eGAPIType e_GAPIType;

		static constexpr eGAPIType GetCurrentGAPI() noexcept;

		Result<> Serialize(std::vector<std::byte>& buffer, const zzz::core::Serializer& s) const override
		{
			//return s.Serialize(buffer, m_AppWinConfig);
			//.and_then([&]() { return s.Serialize(buffer, i2); });

			return {};
		}

		Result<> DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, const zzz::core::Serializer& s) override
		{
			//return s.DeSerialize(buffer, offset, m_AppWinConfig);
			//.and_then([&]() { return s.DeSerialize(buffer, offset, i2); });

			return {};
		}
	};

	constexpr eGAPIType GAPIConfig::GetCurrentGAPI() noexcept
	{
#if defined(ZRENDER_API_D3D12)
		return eGAPIType::DirectX;
#elif defined(ZRENDER_API_VULKAN)
		return eGAPIType::Vulkan;
#elif defined(ZRENDER_API_METAL)
		return eGAPIType::Metal;
#else
		static_assert(false, "Unsupported graphics API. Define ZRENDER_API_D3D12, ZRENDER_API_VULKAN, or ZRENDER_API_METAL");
#endif
	}
}