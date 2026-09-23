#include "ShaderImporter.h"
#include <json.hpp>
#include "core/io/package/assets/ShaderData.h"
#include "core/serialize/Serializer.h"

using json = nlohmann::json;

namespace zzz::builder
{
	ImportResult ShaderImporter::Import(const ImportContext& ctx)
	{
		const std::string source(
			reinterpret_cast<const char*>(ctx.sourceData.data()),
			ctx.sourceData.size());
		json root = json::parse(source, nullptr, false);
		if (root.is_discarded())
		{
			return std::unexpected("Некорректный JSON в шейдере '" + ctx.sourceFilePath.string() + "'.");
		}
		if (!root.is_object())
			return std::unexpected("Корень шейдера должен быть JSON-объектом: " + ctx.sourceFilePath.string());

		std::string shaderName = ctx.assetName;
		if (root.contains("name"))
		{
			if (!root["name"].is_string())
				return std::unexpected("Поле 'name' шейдера должно быть строкой: " + ctx.sourceFilePath.string());

			shaderName = root["name"].get<std::string>();
			if (shaderName.empty())
				return std::unexpected("Поле 'name' шейдера не должно быть пустым: " + ctx.sourceFilePath.string());
		}

		core::ShaderData shaderData(std::move(shaderName));
		core::Serializer serializer;
		std::vector<std::byte> payload;

		auto serRes = serializer.Serialize(payload, shaderData);
		if (!serRes)
		{
			return std::unexpected("Ошибка сериализации ShaderData: " + serRes.error());
		}

		return ImportedAssetData{ .binaryPayload = std::move(payload) };
	}
}
