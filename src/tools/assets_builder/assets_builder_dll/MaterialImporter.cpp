#include "MaterialImporter.h"
#include <json.hpp>
#include "core/io/package/assets/MaterialData.h"
#include "core/serialize/Serializer.h"

using json = nlohmann::json;

namespace zzz::builder
{
	ImportResult MaterialImporter::Import(const ImportContext& ctx)
	{
		const std::string source(
			reinterpret_cast<const char*>(ctx.sourceData.data()),
			ctx.sourceData.size());
		json root = json::parse(source, nullptr, false);
		if (root.is_discarded())
		{
			return std::unexpected("Некорректный JSON в материале '" + ctx.sourceFilePath.string() + "'.");
		}
		if (!root.is_object())
			return std::unexpected("Корень материала должен быть JSON-объектом: " + ctx.sourceFilePath.string());

		std::string materialName = ctx.assetName;
		if (root.contains("name"))
		{
			if (!root["name"].is_string())
				return std::unexpected("Поле 'name' материала должно быть строкой: " + ctx.sourceFilePath.string());

			materialName = root["name"].get<std::string>();
			if (materialName.empty())
				return std::unexpected("Поле 'name' материала не должно быть пустым: " + ctx.sourceFilePath.string());
		}

		core::Guid shaderGuid{};
		if (root.contains("shader"))
		{
			if (!root["shader"].is_string())
				return std::unexpected("Поле 'shader' материала должно быть строкой (GUID): " + ctx.sourceFilePath.string());

			const auto shaderStr = root["shader"].get<std::string>();
			auto parseRes = core::Guid::Parse(shaderStr);
			if (!parseRes)
			{
				return std::unexpected("Некорректный GUID шейдера в материале '" + ctx.sourceFilePath.string() + "': " + shaderStr);
			}
			shaderGuid = *parseRes;
		}

		core::MaterialData matData(std::move(materialName), shaderGuid);
		core::Serializer serializer;
		std::vector<std::byte> payload;

		auto serRes = serializer.Serialize(payload, matData);
		if (!serRes)
		{
			return std::unexpected("Ошибка сериализации MaterialData: " + serRes.error());
		}

		return ImportedAssetData{ .binaryPayload = std::move(payload) };
	}
}
