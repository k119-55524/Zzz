
#include <format>
#include <string>

#include "core/utils/SafeMath.h"
#include "core/utils/Macroses.h"
#include "core/headers/Android.h"

#include "MappedFileHandle.h"

namespace zzz::core
{
	struct MappedFileHandle::Impl
	{
		std::size_t size = 0;
		const std::byte* data = nullptr;
		AAsset* asset = nullptr;

		~Impl()
		{
			if (asset)
				AAsset_close(asset);
		}
	};

	MappedFileHandle::MappedFileHandle() noexcept = default;

	MappedFileHandle::MappedFileHandle(std::unique_ptr<Impl> impl) noexcept
		: m_Impl(std::move(impl))
	{
	}

	MappedFileHandle::~MappedFileHandle() = default;
	MappedFileHandle::MappedFileHandle(MappedFileHandle&& other) noexcept = default;
	MappedFileHandle& MappedFileHandle::operator=(MappedFileHandle&& other) noexcept = default;

	std::expected<MappedFileHandle, std::string> MappedFileHandle::Open(
		const std::filesystem::path& physicalPath,
		NativeAppData* nativeData)
	{
		if (!nativeData || !nativeData->activity || !nativeData->activity->assetManager)
			return UNEXPECTED("Android AssetManager недоступен.");

		std::string assetPath = physicalPath.generic_string();
		constexpr std::string_view assetsMarker = "/assets/";
		if (const auto markerPos = assetPath.rfind(assetsMarker); markerPos != std::string::npos)
			assetPath.erase(0, markerPos + 1);

		auto impl = std::make_unique<Impl>();
		impl->asset = AAssetManager_open(
			nativeData->activity->assetManager,
			assetPath.c_str(),
			AASSET_MODE_BUFFER);
		if (!impl->asset && assetPath.starts_with("assets/"))
		{
			impl->asset = AAssetManager_open(
				nativeData->activity->assetManager,
				assetPath.c_str() + 7,
				AASSET_MODE_BUFFER);
		}
		if (!impl->asset)
			return UNEXPECTED("Файл '{}' не найден в Android assets.", assetPath);

		const auto assetSize = AAsset_getLength64(impl->asset);
		if (assetSize < 0)
			return UNEXPECTED("Android asset '{}' имеет некорректный отрицательный размер.", assetPath);

		const auto mappedSize = NarrowTo<std::size_t>(static_cast<std::uint64_t>(assetSize));
		if (!mappedSize)
			return UNEXPECTED("Размер Android asset '{}' не представим типом std::size_t.", assetPath);

		impl->size = *mappedSize;
		if (impl->size > 0)
		{
			const void* buffer = AAsset_getBuffer(impl->asset);
			if (!buffer)
				return UNEXPECTED("Не удалось получить буфер Android asset '{}'.", assetPath);

			impl->data = static_cast<const std::byte*>(buffer);
		}

		return MappedFileHandle(std::move(impl));
	}

	const std::byte* MappedFileHandle::GetData() const noexcept
	{
		return m_Impl ? m_Impl->data : nullptr;
	}

	std::size_t MappedFileHandle::GetSize() const noexcept
	{
		return m_Impl ? m_Impl->size : 0;
	}

	bool MappedFileHandle::IsValid() const noexcept
	{
		return m_Impl != nullptr;
	}

	void MappedFileHandle::Close() noexcept
	{
		m_Impl.reset();
	}
}
