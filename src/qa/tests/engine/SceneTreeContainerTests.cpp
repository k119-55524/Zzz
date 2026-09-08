#include "qa/tests/TestsConfig.h"

#ifdef Z_TEST_ENGINE_SCENE_TREE_CONTAINER

#include <gtest/gtest.h>
#include "engine/scene/storage/SceneTreeContainer.h"
#include "engine/scene/storage/DefaultSpatialStorage.h"

using namespace zzz;
using namespace zzz::engine;

TEST(SceneTreeContainerTest, NodeCreationAndValidation)
{
	SceneTreeContainer container;

	NodeHandle h1 = container.CreateNode("Node1");
	EXPECT_TRUE(h1.IsValid());
	EXPECT_TRUE(container.IsValid(h1));
	EXPECT_EQ(container.GetName(h1), "Node1");
	EXPECT_TRUE(container.IsActive(h1));

	container.SetName(h1, "Node1_Renamed");
	EXPECT_EQ(container.GetName(h1), "Node1_Renamed");

	container.SetActive(h1, false);
	EXPECT_FALSE(container.IsActive(h1));
}

TEST(SceneTreeContainerTest, AbadProtectionViaGeneration)
{
	SceneTreeContainer container;

	NodeHandle h1 = container.CreateNode("First");
	EXPECT_EQ(h1.index, 0u);
	EXPECT_EQ(h1.generation, 1u);

	container.DestroySubtree(h1);
	EXPECT_FALSE(container.IsValid(h1));

	// Повторное создание берет тот же слот 0, но generation растет
	NodeHandle h2 = container.CreateNode("Second");
	EXPECT_EQ(h2.index, 0u);
	EXPECT_EQ(h2.generation, 2u);

	EXPECT_TRUE(container.IsValid(h2));
	EXPECT_FALSE(container.IsValid(h1)); // Старый дескриптор h1 не валиден!
}

TEST(SceneTreeContainerTest, HierarchyAndReparenting)
{
	SceneTreeContainer container;

	NodeHandle root = container.CreateNode("Root");
	NodeHandle child1 = container.CreateNode("Child1");
	NodeHandle child2 = container.CreateNode("Child2");

	container.SetParent(child1, root, false);
	container.SetParent(child2, root, false);

	EXPECT_EQ(container.GetParent(child1), root);
	EXPECT_EQ(container.GetParent(child2), root);

	// Отвязываем child1 от родителя
	container.SetParent(child1, NodeHandle{}, false);
	EXPECT_FALSE(container.GetParent(child1).IsValid());
	EXPECT_EQ(container.GetParent(child2), root);
}

TEST(SceneTreeContainerTest, ResolveTransformsChain)
{
	SceneTreeContainer container;

	NodeHandle root = container.CreateNode("Root");
	container.SetLocalPosition(root, math::Vec3<zF32>{ 10.0f, 0.0f, 0.0f });

	NodeHandle child = container.CreateNode("Child");
	container.SetLocalPosition(child, math::Vec3<zF32>{ 0.0f, 5.0f, 0.0f });
	container.SetParent(child, root, false);

	container.ResolveTransforms();

	const auto rootWorld = container.GetWorldMatrix(root);
	const auto childWorld = container.GetWorldMatrix(child);

	EXPECT_NEAR(rootWorld._41, 10.0f, 1e-4f);
	EXPECT_NEAR(rootWorld._42, 0.0f, 1e-4f);

	// Мировое положение ребенка должно быть (10, 5, 0)
	EXPECT_NEAR(childWorld._41, 10.0f, 1e-4f);
	EXPECT_NEAR(childWorld._42, 5.0f, 1e-4f);
}

TEST(SceneTreeContainerTest, DestroySubtreeCleansChildrenAndSpatial)
{
	SceneTreeContainer container;
	DefaultSpatialStorage spatial;

	NodeHandle parent = container.CreateNode("Parent");
	NodeHandle child1 = container.CreateNode("Child1");
	NodeHandle child2 = container.CreateNode("Child2");

	container.SetParent(child1, parent, false);
	container.SetParent(child2, parent, false);

	uint32_t spHandle = spatial.Insert(100ULL);
	container.SetSpatialHandle(child1, spHandle);
	EXPECT_EQ(container.GetSpatialHandle(child1), spHandle);

	container.DestroySubtree(parent, &spatial);

	EXPECT_FALSE(container.IsValid(parent));
	EXPECT_FALSE(container.IsValid(child1));
	EXPECT_FALSE(container.IsValid(child2));
	EXPECT_EQ(spatial.GetCount(), 0u);
}

TEST(SceneTreeContainerTest, CreateNodeDoesNotClearDirtyTracker)
{
	SceneTreeContainer container;

	// Кадр 1: создаем узлы и фиксируем барьер
	NodeHandle h0 = container.CreateNode("Node0");
	NodeHandle h1 = container.CreateNode("Node1");
	container.ApplyHandoverBarrier();

	// Кадр 2: новый кадр
	container.BeginFrame();
	EXPECT_TRUE(container.GetSecondaryNodes().dirtyTracker.GetDirtyIndices().empty());

	// Мутируем Node0
	container.SetLocalPosition(h0, math::Vec3<zF32>{ 1.0f, 2.0f, 3.0f });
	auto dirtyIndicesBefore = container.GetSecondaryNodes().dirtyTracker.GetDirtyIndices();
	ASSERT_EQ(dirtyIndicesBefore.size(), 1u);
	EXPECT_EQ(dirtyIndicesBefore[0], h0.index);

	// Создаем новый узел Node2 в этом же кадре (через расширение векторов)
	NodeHandle h2 = container.CreateNode("Node2");

	// Проверяем: dirty-бит для Node0 НЕ затерся, и Node2 тоже помечен как dirty
	auto dirtyIndicesAfter = container.GetSecondaryNodes().dirtyTracker.GetDirtyIndices();
	EXPECT_GE(dirtyIndicesAfter.size(), 2u);

	bool foundH0 = false;
	bool foundH2 = false;
	for (uint32_t idx : dirtyIndicesAfter)
	{
		if (idx == h0.index) foundH0 = true;
		if (idx == h2.index) foundH2 = true;
	}
	EXPECT_TRUE(foundH0);
	EXPECT_TRUE(foundH2);
}

TEST(SceneTreeContainerTest, ApplyHandoverBarrierSelectiveSync)
{
	SceneTreeContainer container;

	NodeHandle h0 = container.CreateNode("Node0");
	NodeHandle h1 = container.CreateNode("Node1");
	container.ResolveTransforms();
	container.ApplyHandoverBarrier();

	// Проверяем первичное состояние в Front Buffer
	EXPECT_EQ(container.GetPrimaryNodes().metadata.size(), 2u);
	EXPECT_EQ(container.GetPrimaryNodes().metadata[0].name, "Node0");
	EXPECT_EQ(container.GetPrimaryNodes().metadata[1].name, "Node1");

	// Кадр 2: изменяем только Node1
	container.BeginFrame();
	container.SetLocalPosition(h1, math::Vec3<zF32>{ 50.0f, 0.0f, 0.0f });
	container.SetName(h1, "Node1_Updated");
	container.ResolveTransforms();

	// В Back Buffer Node1 обновлен
	EXPECT_NEAR(container.GetSecondaryNodes().worldMatrices[h1.index]._41, 50.0f, 1e-4f);
	// В Front Buffer до барьера - старое значение
	EXPECT_NEAR(container.GetPrimaryNodes().worldMatrices[h1.index]._41, 0.0f, 1e-4f);

	// Вызываем дифференциальный барьер
	container.ApplyHandoverBarrier();

	// В Front Buffer Node1 обновился, Node0 остался прежним
	EXPECT_NEAR(container.GetPrimaryNodes().worldMatrices[h1.index]._41, 50.0f, 1e-4f);
	EXPECT_EQ(container.GetPrimaryNodes().metadata[h1.index].name, "Node1_Updated");
	EXPECT_EQ(container.GetPrimaryNodes().metadata[h0.index].name, "Node0");
}

#endif // Z_TEST_ENGINE_SCENE_TREE_CONTAINER

