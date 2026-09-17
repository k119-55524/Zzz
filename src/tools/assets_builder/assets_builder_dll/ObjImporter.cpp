#include "ObjImporter.h"
#include <sstream>
#include <vector>
#include <unordered_map>
#include <cstring>
#include "core/io/package/MeshData.h"
#include "core/serialize/Serializer.h"
#include "math/geometry/Vertex3D.h"
#include "math/vector/Vec2.h"
#include "math/vector/Vec3.h"

namespace zzz::builder
{
	namespace
	{
		struct VertexKey
		{
			int posIdx{ -1 };
			int texIdx{ -1 };
			int normIdx{ -1 };

			bool operator==(const VertexKey& other) const noexcept
			{
				return posIdx == other.posIdx && texIdx == other.texIdx && normIdx == other.normIdx;
			}
		};

		struct VertexKeyHash
		{
			std::size_t operator()(const VertexKey& k) const noexcept
			{
				std::size_t h1 = std::hash<int>{}(k.posIdx);
				std::size_t h2 = std::hash<int>{}(k.texIdx);
				std::size_t h3 = std::hash<int>{}(k.normIdx);
				return h1 ^ (h2 << 1) ^ (h3 << 2);
			}
		};

		// Парсит фрагмент вершины face: "v", "v/vt", "v//vn" или "v/vt/vn"
		VertexKey ParseFaceVertex(std::string_view token, int totalV, int totalVT, int totalVN)
		{
			VertexKey key{};
			int parts[3] = { 0, 0, 0 };
			int partIndex = 0;
			std::string current;

			for (size_t i = 0; i <= token.size(); ++i)
			{
				if (i == token.size() || token[i] == '/')
				{
					if (!current.empty() && partIndex < 3)
					{
						parts[partIndex] = std::stoi(current);
					}
					current.clear();
					++partIndex;
				}
				else
				{
					current += token[i];
				}
			}

			// Позиция (1-based или отрицательный индекс)
			if (parts[0] > 0) key.posIdx = parts[0] - 1;
			else if (parts[0] < 0) key.posIdx = totalV + parts[0];

			// Текстура (1-based или отрицательный индекс)
			if (parts[1] > 0) key.texIdx = parts[1] - 1;
			else if (parts[1] < 0) key.texIdx = totalVT + parts[1];

			// Нормаль (1-based или отрицательный индекс)
			if (parts[2] > 0) key.normIdx = parts[2] - 1;
			else if (parts[2] < 0) key.normIdx = totalVN + parts[2];

			return key;
		}
	}

	ImportResult ObjImporter::Import(const ImportContext& ctx)
	{
		const std::string source(
			reinterpret_cast<const char*>(ctx.sourceData.data()),
			ctx.sourceData.size());
		std::istringstream file(source);

		std::vector<math::Vec3f> rawPositions;
		std::vector<math::Vec2f> rawTexCoords;
		std::vector<math::Vec3f> rawNormals;

		std::vector<math::Vertex3D> uniqueVertices;
		std::vector<zU16> indices;
		std::unordered_map<VertexKey, zU16, VertexKeyHash> vertexMap;

		std::string line;
		while (std::getline(file, line))
		{
			// Удаляем пробельные символы в начале
			size_t start = line.find_first_not_of(" \t\r\n");
			if (start == std::string::npos)
				continue;

			if (line[start] == '#')
				continue;

			std::istringstream ss(line.substr(start));
			std::string prefix;
			ss >> prefix;

			if (prefix == "v")
			{
				float x = 0.0f, y = 0.0f, z = 0.0f;
				ss >> x >> y >> z;
				rawPositions.emplace_back(x, y, z);
			}
			else if (prefix == "vt")
			{
				float u = 0.0f, v = 0.0f;
				ss >> u >> v;
				rawTexCoords.emplace_back(u, v);
			}
			else if (prefix == "vn")
			{
				float nx = 0.0f, ny = 0.0f, nz = 0.0f;
				ss >> nx >> ny >> nz;
				rawNormals.emplace_back(nx, ny, nz);
			}
			else if (prefix == "f")
			{
				std::vector<std::string> faceTokens;
				std::string token;
				while (ss >> token)
				{
					faceTokens.push_back(token);
				}

				if (faceTokens.size() < 3)
					continue;

				// Триангуляция многоугольника методом fan (веер): (0, i, i+1)
				// Сохраняет заданный порядок обхода (CW для канонических файлов движка)
				for (size_t i = 1; i + 1 < faceTokens.size(); ++i)
				{
					std::string_view triTokens[3] = { faceTokens[0], faceTokens[i], faceTokens[i + 1] };

					for (int j = 0; j < 3; ++j)
					{
						VertexKey key = ParseFaceVertex(triTokens[j],
							static_cast<int>(rawPositions.size()),
							static_cast<int>(rawTexCoords.size()),
							static_cast<int>(rawNormals.size()));

						auto it = vertexMap.find(key);
						if (it != vertexMap.end())
						{
							indices.push_back(it->second);
						}
						else
						{
							math::Vec3f pos = (key.posIdx >= 0 && key.posIdx < static_cast<int>(rawPositions.size()))
								? rawPositions[key.posIdx]
								: math::Vec3f(0.0f, 0.0f, 0.0f);

							math::Vec3f norm = (key.normIdx >= 0 && key.normIdx < static_cast<int>(rawNormals.size()))
								? rawNormals[key.normIdx]
								: math::Vec3f(0.0f, 1.0f, 0.0f);

							math::Vec2f uv = (key.texIdx >= 0 && key.texIdx < static_cast<int>(rawTexCoords.size()))
								? rawTexCoords[key.texIdx]
								: math::Vec2f(0.0f, 0.0f);

							math::Vertex3D vertex(pos, norm, uv, math::Palette4::White, math::Vec4f(1.0f, 0.0f, 0.0f, 1.0f));

							if (uniqueVertices.size() >= 65535)
							{
								return std::unexpected("Меш превышает 65535 вершин для UInt16 индексов: " + ctx.sourceFilePath.string());
							}

							zU16 newIndex = static_cast<zU16>(uniqueVertices.size());
							uniqueVertices.push_back(vertex);
							vertexMap[key] = newIndex;
							indices.push_back(newIndex);
						}
					}
				}
			}
		}

		if (uniqueVertices.empty() || indices.empty())
		{
			return std::unexpected("Файл .obj не содержит валидной геометрии: " + ctx.sourceFilePath.string());
		}

		// Формируем байтовые буферы
		std::vector<std::byte> vertexBytes(uniqueVertices.size() * sizeof(math::Vertex3D));
		std::memcpy(vertexBytes.data(), uniqueVertices.data(), vertexBytes.size());

		std::vector<std::byte> indexBytes(indices.size() * sizeof(zU16));
		std::memcpy(indexBytes.data(), indices.data(), indexBytes.size());

		core::MeshData meshData(
			static_cast<zU32>(uniqueVertices.size()),
			static_cast<zU32>(sizeof(math::Vertex3D)),
			std::move(vertexBytes),
			static_cast<zU32>(indices.size()),
			core::eIndexFormat::UInt16,
			std::move(indexBytes)
		);

		core::Serializer serializer;
		std::vector<std::byte> payload;
		auto serRes = serializer.Serialize(payload, meshData);
		if (!serRes)
		{
			return std::unexpected("Ошибка сериализации MeshData: " + serRes.error());
		}

		return ImportedAssetData{ .binaryPayload = std::move(payload) };
	}
}
