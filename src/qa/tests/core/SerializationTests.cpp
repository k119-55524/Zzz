#include "qa/tests/TestsConfig.h"

#ifdef Z_TEST_CORE_SERIALIZATION

#include <gtest/gtest.h>
#include "math/Math.h"
#include "core/serialize/Serializer.h"

using namespace zzz;
using namespace zzz::math;

TEST(SerializationTest, VectorsPoint2DSize2DRect2D)
{
	core::Serializer serializer;
	std::vector<std::byte> buffer;

	Vec2f originalV2(12.34f, 56.78f);
	Vec3f originalV3(1.0f, 2.0f, 3.0f);
	Vec4f originalV4(4.0f, 5.0f, 6.0f, 7.0f);
	Point2D<zI32> originalPt(1920, 1080);
	Size2D<zU32> originalSz(1280, 720);
	Rect2D<zI32> originalRect(100, 200, 800, 600);

	auto resV2 = serializer.Serialize(buffer, originalV2);
	ASSERT_TRUE(resV2.has_value());

	auto resV3 = serializer.Serialize(buffer, originalV3);
	ASSERT_TRUE(resV3.has_value());

	auto resV4 = serializer.Serialize(buffer, originalV4);
	ASSERT_TRUE(resV4.has_value());

	auto resPt = serializer.Serialize(buffer, originalPt);
	ASSERT_TRUE(resPt.has_value());

	auto resSz = serializer.Serialize(buffer, originalSz);
	ASSERT_TRUE(resSz.has_value());

	auto resRect = serializer.Serialize(buffer, originalRect);
	ASSERT_TRUE(resRect.has_value());

	EXPECT_EQ(buffer.size(),
		sizeof(Vec2f) + sizeof(Vec3f) + sizeof(Vec4f) +
		sizeof(Point2D<zI32>) + sizeof(Size2D<zU32>) + sizeof(Rect2D<zI32>));

	std::size_t offset = 0;
	std::span<const std::byte> span(buffer);

	Vec2f readV2;
	Vec3f readV3;
	Vec4f readV4;
	Point2D<zI32> readPt;
	Size2D<zU32> readSz;
	Rect2D<zI32> readRect;

	auto desV2 = serializer.Deserialize(span, offset, readV2);
	ASSERT_TRUE(desV2.has_value());
	EXPECT_EQ(readV2, originalV2);

	auto desV3 = serializer.Deserialize(span, offset, readV3);
	ASSERT_TRUE(desV3.has_value());
	EXPECT_EQ(readV3, originalV3);

	auto desV4 = serializer.Deserialize(span, offset, readV4);
	ASSERT_TRUE(desV4.has_value());
	EXPECT_EQ(readV4, originalV4);

	auto desPt = serializer.Deserialize(span, offset, readPt);
	ASSERT_TRUE(desPt.has_value());
	EXPECT_EQ(readPt, originalPt);

	auto desSz = serializer.Deserialize(span, offset, readSz);
	ASSERT_TRUE(desSz.has_value());
	EXPECT_EQ(readSz, originalSz);

	auto desRect = serializer.Deserialize(span, offset, readRect);
	ASSERT_TRUE(desRect.has_value());
	EXPECT_EQ(readRect, originalRect);

	EXPECT_EQ(offset, buffer.size());
}

#include "core/io/package/MeshData.h"
#include "core/io/package/DataAssetsManager.h"
#include "core/io/package/SceneData.h"
#include "engine/package/PackageManager.h"
#include "core/io/FileSystem.h"
#include "core/utils/MemoryUtils.h"
#include <windows.h>

TEST(SerializationTest, MeshDataSymmetricSerialization)
{
	core::Serializer serializer;

	std::vector<std::byte> vertexBytes(24 * 64, std::byte{ 0xAB });
	std::vector<std::byte> indexBytes(36 * sizeof(uint16_t), std::byte{ 0xCD });

	core::MeshData originalMesh(24, 64, vertexBytes, 36, core::eIndexFormat::UInt16, indexBytes);

	std::vector<std::byte> buffer;
	auto serRes = serializer.Serialize(buffer, originalMesh);
	ASSERT_TRUE(serRes.has_value());

	core::MeshData readMesh;
	std::size_t offset = 0;
	auto desRes = serializer.Deserialize(buffer, offset, readMesh);
	ASSERT_TRUE(desRes.has_value());

	EXPECT_EQ(readMesh.GetVertexCount(), 24u);
	EXPECT_EQ(readMesh.GetVertexStride(), 64u);
	EXPECT_EQ(readMesh.GetIndexCount(), 36u);
	EXPECT_EQ(readMesh.GetIndexFormat(), core::eIndexFormat::UInt16);
	EXPECT_EQ(readMesh.GetVertexData(), vertexBytes);
	EXPECT_EQ(readMesh.GetIndexData(), indexBytes);
}

TEST(SerializationTest, PackagePackerAndDataAssetsManagerEndToEnd)
{
	// Загружаем assets_builder_dll и вызываем PackProjectNative
	HMODULE hDll = LoadLibraryA("assets_builder_dll.dll");
	if (hDll == nullptr)
	{
		hDll = LoadLibraryA("dist/Debug/assets_builder_dll.dll");
	}
	ASSERT_NE(hDll, nullptr) << "Не удалось загрузить assets_builder_dll.dll";

	using PackFn = bool (*)(const char*, const char*, uint32_t);
	auto packProject = reinterpret_cast<PackFn>(GetProcAddress(hDll, "PackProjectNative"));
	ASSERT_NE(packProject, nullptr) << "Не найдена функция PackProjectNative";

	// Собираем пакет в dist/Debug
	bool ok = packProject("src/projects/assets_projects/zzz_assets_test_000", "dist/Debug", 0);
	EXPECT_TRUE(ok);

	// Собираем также в zzz_assets_test_000_build/game_win_Windows для game_win
	packProject("src/projects/assets_projects/zzz_assets_test_000", "src/projects/assets_projects/zzz_assets_test_000_build/game_win_Windows", 0);

	FreeLibrary(hDll);

	// Проверяем чтение из data.dat через DataAssetsManager
	auto fs = core::safe_make_shared<core::FileSystem>();
	auto dataMgr = core::safe_make_shared<core::DataAssetsManager>(fs);

	core::Guid cubeMeshGuid = *core::Guid::Parse("00000000-0000-0000-0000-000000000010");
	auto meshRes = dataMgr->LoadAsset<core::MeshData>(cubeMeshGuid);
	ASSERT_TRUE(meshRes.has_value()) << "Ошибка загрузки меша куба: " << meshRes.error();

	EXPECT_EQ(meshRes->GetVertexCount(), 24u);
	EXPECT_EQ(meshRes->GetVertexStride(), 64u);
	EXPECT_EQ(meshRes->GetIndexCount(), 36u);
	EXPECT_EQ(meshRes->GetIndexFormat(), core::eIndexFormat::UInt16);

	// Проверяем чтение из package.dat через PackageManager
	auto pkgMgr = core::safe_make_shared<engine::PackageManager>(fs);
	auto sceneRes = pkgMgr->LoadAsset<core::SceneData>("MainScene");
	ASSERT_TRUE(sceneRes.has_value()) << "Ошибка загрузки MainScene: " << sceneRes.error();

	EXPECT_EQ(sceneRes->GetTransitionSource(), core::eTransitionSource::Custom);
	EXPECT_EQ(sceneRes->GetTransitionParams().type, core::eTransitionType::Instant);
	EXPECT_FLOAT_EQ(sceneRes->GetTransitionParams().durationSeconds, 0.0f);
	EXPECT_FALSE(sceneRes->GetGameObjects().empty());
}

#endif // Z_TEST_CORE_SERIALIZATION
