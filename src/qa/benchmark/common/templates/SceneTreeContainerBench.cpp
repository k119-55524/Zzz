#include <benchmark/benchmark.h>
#include "engine/scene/storage/NodeStorage.h"
#include "core/io/package/GameObjectData.h"
#include "math/utils/Types.h"
#include <vector>

using namespace zzz;
using namespace zzz::core;
using namespace zzz::engine;

// 1. Бенчмарк пакетного создания хранилища узлов
static void BM_NodeStorage_Construct(benchmark::State& state)
{
	const size_t count = static_cast<size_t>(state.range(0));
	std::vector<GameObjectData> objects;
	objects.reserve(count);
	for (size_t i = 0; i < count; ++i)
	{
		objects.emplace_back(
			Guid::Generate(),
			"BenchmarkNode",
			false,
			true,
			Vec3<zF32>{ 0.0f, 0.0f, 0.0f },
			Quat<zF32>{ 0.0f, 0.0f, 0.0f, 1.0f },
			Vec3<zF32>{ 1.0f, 1.0f, 1.0f }
		);
	}

	for (auto _ : state)
	{
		NodeStorage container(objects);
		benchmark::DoNotOptimize(container);
	}
}
BENCHMARK(BM_NodeStorage_Construct)->Arg(1000)->Arg(10000);

// 2. Бенчмарк ResolveTransforms на плоской сцене
static void BM_NodeStorage_ResolveFlat(benchmark::State& state)
{
	const size_t count = static_cast<size_t>(state.range(0));
	std::vector<GameObjectData> objects;
	objects.reserve(count);
	for (size_t i = 0; i < count; ++i)
	{
		objects.emplace_back(
			Guid::Generate(),
			"Node",
			false,
			true,
			Vec3<zF32>{ 0.0f, 0.0f, 0.0f },
			Quat<zF32>{ 0.0f, 0.0f, 0.0f, 1.0f },
			Vec3<zF32>{ 1.0f, 1.0f, 1.0f }
		);
	}

	NodeStorage container(objects);

	// Помечаем 1% узлов грязными
	for (size_t i = 0; i < count; i += 100)
	{
		container.SetLocalPosition(static_cast<zU32>(i), ::zzz::math::Vec3<zF32>{ 1.0f, 2.0f, 3.0f });
	}

	for (auto _ : state)
	{
		container.ResolveTransforms();
		benchmark::ClobberMemory();
	}
}
BENCHMARK(BM_NodeStorage_ResolveFlat)->Arg(10000)->Arg(100000);

// 3. Бенчмарк ResolveTransforms на глубоком дереве (цепочка 100 узлов вглубь)
static void BM_NodeStorage_ResolveHierarchy(benchmark::State& state)
{
	constexpr size_t depth = 100;
	std::vector<GameObjectData> objects;
	objects.reserve(depth);

	// Root
	objects.emplace_back(
		Guid::Generate(),
		"Root",
		false,
		true,
		Vec3<zF32>{ 0.0f, 0.0f, 0.0f },
		Quat<zF32>{ 0.0f, 0.0f, 0.0f, 1.0f },
		Vec3<zF32>{ 1.0f, 1.0f, 1.0f }
	);

	// Children chain
	for (size_t i = 1; i < depth; ++i)
	{
		objects.emplace_back(
			Guid::Generate(),
			"Child",
			false,
			true,
			Vec3<zF32>{ 0.0f, 0.0f, 0.0f },
			Quat<zF32>{ 0.0f, 0.0f, 0.0f, 1.0f },
			Vec3<zF32>{ 1.0f, 1.0f, 1.0f },
			std::vector<RenderPairData>{},
			std::vector<Guid>{},
			static_cast<uint32_t>(i - 1)
		);
	}

	NodeStorage container(objects);

	for (auto _ : state)
	{
		container.SetLocalPosition(0, ::zzz::math::Vec3<zF32>{ 5.0f, 0.0f, 0.0f });
		container.ResolveTransforms();
		benchmark::ClobberMemory();
	}
}
BENCHMARK(BM_NodeStorage_ResolveHierarchy);
