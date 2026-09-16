#include "qa/tests/TestsConfig.h"

#ifdef Z_TEST_ENGINE_NODE_STORAGE

#include <gtest/gtest.h>
#include "engine/scene/storage/NodeStorage.h"
#include "core/io/package/GameObjectData.h"

using namespace zzz;
using namespace zzz::core;
using namespace zzz::engine;

static GameObjectData CreateDummyObject(
	std::string name,
	Vec3<zF32> pos = { 0.0f, 0.0f, 0.0f },
	Quat<zF32> rot = { 0.0f, 0.0f, 0.0f, 1.0f },
	Vec3<zF32> scale = { 1.0f, 1.0f, 1.0f },
	uint32_t parentIndex = 0xFFFFFFFF,
	bool isEntity = false)
{
	return GameObjectData(
		Guid::Generate(),
		std::move(name),
		isEntity,
		true,
		pos,
		rot,
		scale,
		{},
		{},
		{},
		parentIndex
	);
}

TEST(NodeStorageTest, BatchConstructionAndValidation)
{
	std::vector<GameObjectData> objects = {
		CreateDummyObject("Node0", { 1.0f, 2.0f, 3.0f }),
		CreateDummyObject("Node1", { 4.0f, 5.0f, 6.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f }, 0xFFFFFFFF, true)
	};

	NodeStorage container(objects);

	EXPECT_EQ(container.GetNodeCount(), 2u);

	const zU32 h0 = 0;
	const zU32 h1 = 1;

	EXPECT_TRUE(container.IsValid(h0));
	EXPECT_TRUE(container.IsValid(h1));
	EXPECT_FALSE(container.IsValid(999));

	EXPECT_EQ(container.GetLocalPosition(h0).x, 1.0f);
	EXPECT_EQ(container.GetLocalPosition(h1).x, 4.0f);

	EXPECT_TRUE(container.IsActive(h0));
	container.SetActive(h0, false);
	EXPECT_FALSE(container.IsActive(h0));
}

TEST(NodeStorageTest, HierarchyAndTransforms)
{
	// Root (0) -> Child (1)
	std::vector<GameObjectData> objects = {
		CreateDummyObject("Root", { 10.0f, 0.0f, 0.0f }),
		CreateDummyObject("Child", { 0.0f, 5.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f }, 0)
	};

	NodeStorage container(objects);

	const NodeHandle root = 0;
	const NodeHandle child = 1;

	EXPECT_EQ(container.GetParent(child), root);
	EXPECT_EQ(container.GetParent(root), kInvalidNodeHandle);

	// Мировые матрицы уже рассчитаны в конструкторе
	const auto rootWorld = container.GetWorldMatrix(root);
	const auto childWorld = container.GetWorldMatrix(child);

	EXPECT_NEAR(rootWorld._41, 10.0f, 1e-4f);
	EXPECT_NEAR(rootWorld._42, 0.0f, 1e-4f);

	// Мировое положение ребенка должно быть (10, 5, 0)
	EXPECT_NEAR(childWorld._41, 10.0f, 1e-4f);
	EXPECT_NEAR(childWorld._42, 5.0f, 1e-4f);
}

TEST(NodeStorageTest, DirtyTrackerAndResolveTransforms)
{
	std::vector<GameObjectData> objects = {
		CreateDummyObject("Root", { 0.0f, 0.0f, 0.0f }),
		CreateDummyObject("Child", { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f }, 0)
	};

	NodeStorage container(objects);
	const zU32 root = 0;
	const zU32 child = 1;

	// Сдвигаем Root на (10, 0, 0)
	container.SetLocalPosition(root, math::Vec3<zF32>{ 10.0f, 0.0f, 0.0f });

	// Каскадный пересчет
	container.ResolveTransforms();

	const auto rootWorld = container.GetWorldMatrix(root);
	const auto childWorld = container.GetWorldMatrix(child);

	EXPECT_NEAR(rootWorld._41, 10.0f, 1e-4f);
	EXPECT_NEAR(childWorld._41, 11.0f, 1e-4f);
}

#endif // Z_TEST_ENGINE_NODE_STORAGE

