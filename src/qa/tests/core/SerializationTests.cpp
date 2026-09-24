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

#include "core/io/package/assets/MeshData.h"
#include "core/io/package/DataAssetsManager.h"
#include "core/io/package/scene/SceneData.h"
#include "engine/package/PackageManager.h"
#include "core/io/storage/FileSystem.h"
#include "core/io/storage/ReadOnlyFile.h"
#include "core/io/storage/ReadWriteFile.h"
#include "core/utils/MemoryUtils.h"
#include <future>
#include <limits>
#include <optional>
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
	EXPECT_TRUE(std::ranges::equal(readMesh.GetVertexData(), vertexBytes));
	EXPECT_TRUE(std::ranges::equal(readMesh.GetIndexData(), indexBytes));
}

TEST(SerializationTest, PackagePackerAndDataAssetsManagerEndToEnd)
{
	std::filesystem::path rootDir = std::filesystem::current_path();
	while (!rootDir.empty() && !std::filesystem::exists(rootDir / "src/projects/assets_projects/zzz_assets_test_000/project.json"))
	{
		if (!rootDir.has_parent_path() || rootDir == rootDir.parent_path())
			break;
		rootDir = rootDir.parent_path();
	}
	ASSERT_TRUE(std::filesystem::exists(rootDir / "src/projects/assets_projects/zzz_assets_test_000/project.json"))
		<< "Не удалось найти корень репозитория из текущего пути: " << std::filesystem::current_path().string();

	const auto dllPath = (rootDir / "dist/Debug/assets_builder_dll.dll").string();
	HMODULE hDll = LoadLibraryA(dllPath.c_str());
	if (hDll == nullptr)
	{
		hDll = LoadLibraryA("assets_builder_dll.dll");
	}
	ASSERT_NE(hDll, nullptr) << "Не удалось загрузить assets_builder_dll.dll из: " << dllPath;

	using PackFn = bool (*)(const char*, const char*, uint32_t, const char*, uint64_t, uint64_t*);
	auto packProject = reinterpret_cast<PackFn>(GetProcAddress(hDll, "PackProjectNative"));
	ASSERT_NE(packProject, nullptr) << "Не найдена функция PackProjectNative";

	const auto srcDir = (rootDir / "src/projects/assets_projects/zzz_assets_test_000").string();
	const auto dstDir = (rootDir / "dist/Debug").string();

	// Собираем пакет
	bool ok = packProject(srcDir.c_str(), dstDir.c_str(), 0, nullptr, 0, nullptr);
	EXPECT_TRUE(ok);

	FreeLibrary(hDll);

	// Проверяем чтение из data.dat через DataAssetsManager
	auto fs = core::safe_make_shared<core::FileSystem>();
	auto dataPathRes = fs->GetDataPackagePath();
	ASSERT_TRUE(dataPathRes.has_value()) << dataPathRes.error();
	auto dataMgr = core::safe_make_shared<core::DataAssetsManager>(*dataPathRes);

	core::Guid cubeMeshGuid = *core::Guid::Parse("00000000-0000-0000-0000-000000000010");
	const auto* entry = dataMgr->GetEntry(core::eDataDatType::Mesh, cubeMeshGuid);
	ASSERT_NE(entry, nullptr) << "Ресурс меша куба не найден в оглавлении data.dat";
	EXPECT_EQ(entry->GetGuid(), cubeMeshGuid);

	auto payloadRes = dataMgr->ReadRawPayload(*entry);
	ASSERT_TRUE(payloadRes.has_value()) << payloadRes.error();
	EXPECT_EQ(payloadRes->size(), entry->GetSize());

	core::PackageEntry emptyEntry(cubeMeshGuid, static_cast<zU32>(core::eDataDatType::Mesh), entry->GetOffset(), 0);
	auto emptyPayloadRes = dataMgr->ReadRawPayload(emptyEntry);
	ASSERT_TRUE(emptyPayloadRes.has_value()) << emptyPayloadRes.error();
	EXPECT_TRUE(emptyPayloadRes->empty());

	core::PackageEntry invalidEntry(
		cubeMeshGuid,
		static_cast<zU32>(core::eDataDatType::Mesh),
		(std::numeric_limits<zU64>::max)(),
		1);
	EXPECT_FALSE(dataMgr->ReadRawPayload(invalidEntry).has_value());

	auto meshRes = dataMgr->LoadAsset<core::MeshData>(cubeMeshGuid);
	ASSERT_TRUE(meshRes.has_value()) << "Ошибка загрузки меша куба: " << meshRes.error();

	EXPECT_EQ(meshRes->GetVertexCount(), 24u);
	EXPECT_EQ(meshRes->GetVertexStride(), 64u);
	EXPECT_EQ(meshRes->GetIndexCount(), 36u);
	EXPECT_EQ(meshRes->GetIndexFormat(), core::eIndexFormat::UInt16);

	std::vector<std::future<bool>> readers;
	for (std::size_t i = 0; i < 8; ++i)
	{
		readers.push_back(std::async(std::launch::async, [dataMgr, cubeMeshGuid]()
		{
			for (std::size_t iteration = 0; iteration < 64; ++iteration)
			{
				auto concurrentMesh = dataMgr->LoadAsset<core::MeshData>(cubeMeshGuid);
				if (!concurrentMesh || concurrentMesh->GetVertexCount() != 24u || concurrentMesh->GetIndexCount() != 36u)
					return false;
			}
			return true;
		}));
	}

	for (auto& reader : readers)
		EXPECT_TRUE(reader.get());

	// Проверяем строгий контроль типов: запрос Mesh GUID с неверным типом ресурса должен возвращать nullptr
	const auto* wrongTypeEntry = dataMgr->GetEntry(core::eDataDatType::Texture2D, cubeMeshGuid);
	EXPECT_EQ(wrongTypeEntry, nullptr);

	// Проверяем чтение из package.dat через PackageManager
	auto pkgPathRes = fs->GetGamePackagePath();
	ASSERT_TRUE(pkgPathRes.has_value()) << pkgPathRes.error();
	auto pkgMgr = core::safe_make_shared<engine::PackageManager>(*pkgPathRes);
	core::Guid sceneGuid = *core::Guid::Parse("3cbf41ff-f608-47ca-b383-ea5698648aca");

	const auto* sceneEntry = pkgMgr->GetEntry(core::ePackageDatType::Scene, sceneGuid);
	ASSERT_NE(sceneEntry, nullptr) << "Запись сцены не найдена в оглавлении package.dat";
	EXPECT_EQ(sceneEntry->GetGuid(), sceneGuid);

	auto sceneRes = pkgMgr->LoadAsset<core::SceneData>(sceneGuid);
	ASSERT_TRUE(sceneRes.has_value()) << "Ошибка загрузки MainScene: " << sceneRes.error();

	EXPECT_EQ(sceneRes->GetTransitionSource(), core::eTransitionSource::Custom);
	EXPECT_EQ(sceneRes->GetTransitionParams().type, core::eTransitionType::Instant);
	EXPECT_FLOAT_EQ(sceneRes->GetTransitionParams().durationSeconds, 0.0f);
	EXPECT_FALSE(sceneRes->GetLayers().empty());
	// Проверяем прямую работу с ReadOnlyFile и ReadWriteFile
	{
		auto readOnlyRes = core::ReadOnlyFile::Open(*dataPathRes);
		ASSERT_TRUE(readOnlyRes.has_value()) << readOnlyRes.error();
		EXPECT_TRUE(readOnlyRes->IsValid());
		EXPECT_GT(readOnlyRes->GetSize(), 0u);
		auto magicSpan = readOnlyRes->Subspan(0, 3);
		ASSERT_EQ(magicSpan.size(), 3u);
		EXPECT_EQ(static_cast<char>(magicSpan[0]), 'Z');
		EXPECT_EQ(static_cast<char>(magicSpan[1]), 'D');
		EXPECT_EQ(static_cast<char>(magicSpan[2]), 'D');
	}

	{
		auto initRes = fs->InitializeUserData("ZzzTestCompany", "ZzzTestApp");
		ASSERT_TRUE(initRes.has_value()) << initRes.error();

		const std::filesystem::path testRelPath = "test_rw_file.bin";
		const std::vector<std::byte> testPayload = {
			std::byte{ 0x11 }, std::byte{ 0x22 }, std::byte{ 0x33 }, std::byte{ 0x44 }
		};

		// 1. Запись через ReadWriteFile
		{
			core::ReadWriteFile rwFile(*fs, core::eFileLocation::User, testRelPath, core::eFileAccessMode::Write);
			ASSERT_TRUE(rwFile.IsValid()) << rwFile.GetError();
			auto writeRes = rwFile.WriteAll(testPayload);
			EXPECT_TRUE(writeRes.has_value());
			EXPECT_EQ(rwFile.GetSize(), testPayload.size());
		}

		// 2. Чтение через ReadWriteFile
		{
			core::ReadWriteFile rwFile(*fs, core::eFileLocation::User, testRelPath, core::eFileAccessMode::Read);
			ASSERT_TRUE(rwFile.IsValid()) << rwFile.GetError();
			auto readRes = rwFile.ReadAll();
			ASSERT_TRUE(readRes.has_value()) << readRes.error();
			EXPECT_EQ(*readRes, testPayload);
		}

		// 3. Удаление через FileSystem
		EXPECT_TRUE(fs->FileExists(core::eFileLocation::User, testRelPath));
		auto delRes = fs->DeleteFile(core::eFileLocation::User, testRelPath);
		EXPECT_TRUE(delRes.has_value());
		EXPECT_FALSE(fs->FileExists(core::eFileLocation::User, testRelPath));
	}
}

#endif // Z_TEST_CORE_SERIALIZATION
