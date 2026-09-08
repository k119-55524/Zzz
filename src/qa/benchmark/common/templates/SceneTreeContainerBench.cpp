#include <benchmark/benchmark.h>
#include "engine/scene/storage/SceneTreeContainer.h"
#include "math/utils/Types.h"
#include <vector>

using namespace zzz;
using namespace zzz::engine;

// 1. Бенчмарк пакетного создания узлов
static void BM_SceneTreeContainer_CreateNode(benchmark::State& state)
{
	const size_t count = static_cast<size_t>(state.range(0));

	for (auto _ : state)
	{
		SceneTreeContainer container;
		for (size_t i = 0; i < count; ++i)
		{
			auto h = container.CreateNode("BenchmarkNode");
			benchmark::DoNotOptimize(h);
		}
	}
}
BENCHMARK(BM_SceneTreeContainer_CreateNode)->Arg(1000)->Arg(10000);

// 2. Бенчмарк ResolveTransforms на плоской сцене
static void BM_SceneTreeContainer_ResolveFlat(benchmark::State& state)
{
	const size_t count = static_cast<size_t>(state.range(0));
	SceneTreeContainer container;

	std::vector<NodeHandle> handles;
	handles.reserve(count);
	for (size_t i = 0; i < count; ++i)
	{
		handles.push_back(container.CreateNode("Node"));
	}

	container.ResolveTransforms();

	// Помечаем 1% узлов грязными
	for (size_t i = 0; i < count; i += 100)
	{
		container.SetLocalPosition(handles[i], ::zzz::math::Vec3<zF32>{ 1.0f, 2.0f, 3.0f });
	}

	for (auto _ : state)
	{
		container.ResolveTransforms();
		benchmark::ClobberMemory();
	}
}
BENCHMARK(BM_SceneTreeContainer_ResolveFlat)->Arg(10000)->Arg(100000);

// 3. Бенчмарк ResolveTransforms на глубоком дереве (цепочка 100 узлов вглубь)
static void BM_SceneTreeContainer_ResolveHierarchy(benchmark::State& state)
{
	SceneTreeContainer container;
	constexpr size_t depth = 100;

	NodeHandle parent = container.CreateNode("Root");
	for (size_t i = 1; i < depth; ++i)
	{
		NodeHandle child = container.CreateNode("Child");
		container.SetParent(child, parent, false);
		parent = child;
	}

	container.ResolveTransforms();

	for (auto _ : state)
	{
		container.SetLocalPosition(NodeHandle{ 0, 1 }, ::zzz::math::Vec3<zF32>{ 5.0f, 0.0f, 0.0f });
		container.ResolveTransforms();
		benchmark::ClobberMemory();
	}
}
BENCHMARK(BM_SceneTreeContainer_ResolveHierarchy);
