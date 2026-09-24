#pragma once
#include <span>
#include <string>
#include <vector>
#include <memory>
#include <expected>
#include <cstddef>
#include "engine/resources/ResourceBase.h"
#include "core/io/package/PackageEntry.h"
#include "core/enums/eEngineResourceType.h"

namespace zzz::engine
{
	/**
	 * @class CpuTexture2D
	 * @brief Ресурс 2D-текстуры в оперативной памяти (CPU).
	 */
	class CpuTexture2D final : public ResourceBase
	{
	public:
		static constexpr ::zzz::core::eEngineResourceType c_ResourceType = ::zzz::core::eEngineResourceType::Texture2D;

		CpuTexture2D(
			const ::zzz::core::Guid& guid,
			std::string name,
			uint32_t width = 0,
			uint32_t height = 0,
			std::vector<std::byte> pixelData = {});
		~CpuTexture2D() override = default;

		[[nodiscard]] static std::expected<std::shared_ptr<CpuTexture2D>, std::string> CreateCpuResourceFromPackageBytes(
			const ::zzz::core::PackageEntry& entry,
			std::span<const std::byte> bytes);

		[[nodiscard]] uint32_t GetWidth() const noexcept { return m_Width; }
		[[nodiscard]] uint32_t GetHeight() const noexcept { return m_Height; }
		[[nodiscard]] std::span<const std::byte> GetPixelData() const noexcept { return m_PixelData; }

	private:
		uint32_t m_Width;
		uint32_t m_Height;
		std::vector<std::byte> m_PixelData;
	};
}
