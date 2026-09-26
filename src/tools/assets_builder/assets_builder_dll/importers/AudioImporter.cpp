#include "AudioImporter.h"
#include <cstring>
#include "core/utils/macros/MiscMacros.h"
#include "core/logger/logger.h"

namespace zzz::builder
{
	ImportResult AudioImporter::Import(const ImportContext& ctx)
	{
		if (ctx.sourceData.empty())
		{
			return UNEXPECTED("Исходные данные аудиофайла пусты: {}", ctx.sourceFilePath.string());
		}

		ImportedAssetData result;
		result.binaryPayload.assign(ctx.sourceData.begin(), ctx.sourceData.end());

		const auto ext = ctx.sourceFilePath.extension().string();

		if (ext == ".wav" || ext == ".WAV")
		{
			// Базовый парсинг RIFF/WAVE заголовка
			const auto* bytes = reinterpret_cast<const uint8_t*>(ctx.sourceData.data());
			const std::size_t size = ctx.sourceData.size();

			if (size >= 44 && std::memcmp(bytes, "RIFF", 4) == 0 && std::memcmp(bytes + 8, "WAVE", 4) == 0)
			{
				std::size_t offset = 12;
				uint16_t channels = 0;
				uint32_t sampleRate = 0;
				uint16_t bitsPerSample = 0;
				uint32_t dataSize = 0;

				while (offset + 8 <= size)
				{
					char chunkId[5] = { 0 };
					std::memcpy(chunkId, bytes + offset, 4);
					uint32_t chunkSize = 0;
					std::memcpy(&chunkSize, bytes + offset + 4, 4);
					offset += 8;

					if (std::strcmp(chunkId, "fmt ") == 0 && chunkSize >= 16 && offset + 16 <= size)
					{
						uint16_t formatTag = 0;
						std::memcpy(&formatTag, bytes + offset, 2);
						std::memcpy(&channels, bytes + offset + 2, 2);
						std::memcpy(&sampleRate, bytes + offset + 4, 4);
						std::memcpy(&bitsPerSample, bytes + offset + 14, 2);
						result.metadata.audio.audioFormat = (formatTag == 1) ? 0 : 3; // 0 = PCM
					}
					else if (std::strcmp(chunkId, "data") == 0)
					{
						dataSize = chunkSize;
						break;
					}

					offset += chunkSize;
					// Выравнивание чанков по 2 байтам в RIFF
					if (chunkSize % 2 != 0)
						offset += 1;
				}

				result.metadata.audio.sampleRate = sampleRate;
				result.metadata.audio.channels = channels;
				result.metadata.audio.bitsPerSample = bitsPerSample;
				if (channels > 0 && bitsPerSample > 0)
				{
					const uint32_t bytesPerSample = (bitsPerSample / 8) * channels;
					if (bytesPerSample > 0)
						result.metadata.audio.sampleCount = dataSize / bytesPerSample;
				}
			}
		}
		else if (ext == ".ogg" || ext == ".OGG")
		{
			result.metadata.audio.audioFormat = 1; // Vorbis
		}

		return result;
	}
}
