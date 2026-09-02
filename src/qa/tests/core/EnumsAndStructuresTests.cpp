#include "qa/tests/TestsConfig.h"

#ifdef Z_TEST_CORE_ENUMS_STRUCTS

#include <gtest/gtest.h>
#include <vector>
#include <cstring>
#include "core/Core.h"
#include "core/enums/eResourceType.h"
#include "core/enums/ePixelFormat.h"
#include "core/enums/eIndexFormat.h"
#include "core/enums/eVertexSemantic.h"
#include "core/enums/eFileLocation.h"
#include "core/enums/ePackage.h"
#include "core/enums/eTargetPlatform.h"
#include "core/enums/eGAPIType.h"
#include "core/enums/eWindowState.h"
#include "core/enums/eLogMessageType.h"
#include "core/enums/eWinResize.h"
#include "math/Math.h"

using namespace zzz;
using namespace zzz::core;
using namespace zzz::math;

// =============================================================================
// 1. Тесты выравнивания AlignUp и констант GAPI
// =============================================================================

TEST(EnumsAndStructuresTests, AlignUpAndGAPIConstants)
{
	static_assert(c_ConstantBufferAlignment == 256, "c_ConstantBufferAlignment must be 256 bytes");

	// Проверка compile-time constexpr
	static_assert(AlignUp(0u, 256u) == 0u);
	static_assert(AlignUp(1u, 256u) == 256u);
	static_assert(AlignUp(64u, 256u) == 256u);
	static_assert(AlignUp(256u, 256u) == 256u);
	static_assert(AlignUp(257u, 256u) == 512u);

	// Проверка в рантайме
	EXPECT_EQ(AlignUp(0ULL, 256ULL), 0ULL);
	EXPECT_EQ(AlignUp(1ULL, 256ULL), 256ULL);
	EXPECT_EQ(AlignUp(128ULL, 256ULL), 256ULL);
	EXPECT_EQ(AlignUp(256ULL, 256ULL), 256ULL);
	EXPECT_EQ(AlignUp(257ULL, 256ULL), 512ULL);
	EXPECT_EQ(AlignUp(500ULL, 256ULL), 512ULL);
	EXPECT_EQ(AlignUp(512ULL, 256ULL), 512ULL);
	EXPECT_EQ(AlignUp(513ULL, 256ULL), 768ULL);

	// Проверка с другими степенями двойки
	EXPECT_EQ(AlignUp(5, 4), 8);
	EXPECT_EQ(AlignUp(12, 16), 16);
	EXPECT_EQ(AlignUp(17, 16), 32);
	EXPECT_EQ(AlignUp(60, 64), 64);
	EXPECT_EQ(AlignUp(64, 64), 64);
	EXPECT_EQ(AlignUp(65, 64), 128);
}

// =============================================================================
// 2. Тесты макета памяти и инвариантов Vertex3D
// =============================================================================

TEST(EnumsAndStructuresTests, Vertex3DLayoutAndInvariants)
{
	static_assert(sizeof(Vertex3D) == 64, "Vertex3D must be exactly 64 bytes (1 CPU cache line)");
	static_assert(alignof(Vertex3D) == 16, "Vertex3D must be 16-byte aligned");
	static_assert(std::is_standard_layout_v<Vertex3D>, "Vertex3D must be standard layout");

	// Дефолтный конструктор
	Vertex3D vDefault;
	EXPECT_EQ(vDefault.position, Vec3f(0.0f, 0.0f, 0.0f));
	EXPECT_EQ(vDefault.normal, Vec3f(0.0f, 0.0f, 1.0f));
	EXPECT_EQ(vDefault.texCoord, Vec2f(0.0f, 0.0f));
	EXPECT_EQ(vDefault.color, Palette4::White);
	EXPECT_EQ(vDefault.tangent, Vec4f(1.0f, 0.0f, 0.0f, 1.0f));

	// Параметризованный конструктор
	Vertex3D vCustom(
		Vec3f(1.0f, 2.0f, 3.0f),
		Vec3f(0.0f, 1.0f, 0.0f),
		Vec2f(0.5f, 0.75f),
		Palette4::Red,
		Vec4f(0.0f, 0.0f, 1.0f, -1.0f)
	);

	EXPECT_EQ(vCustom.position, Vec3f(1.0f, 2.0f, 3.0f));
	EXPECT_EQ(vCustom.normal, Vec3f(0.0f, 1.0f, 0.0f));
	EXPECT_EQ(vCustom.texCoord, Vec2f(0.5f, 0.75f));
	EXPECT_EQ(vCustom.color, Palette4::Red);
	EXPECT_EQ(vCustom.tangent, Vec4f(0.0f, 0.0f, 1.0f, -1.0f));

	// Сравнение
	Vertex3D vCopy = vCustom;
	EXPECT_EQ(vCustom, vCopy);
	EXPECT_NE(vDefault, vCustom);
}

// =============================================================================
// 3. Тесты легковесного диапазона AttributeRange<T>
// =============================================================================

TEST(EnumsAndStructuresTests, AttributeRangeOperations)
{
	// 1. Пустой диапазон
	AttributeRange<Vec3f> emptyRange;
	EXPECT_TRUE(emptyRange.empty());
	EXPECT_EQ(emptyRange.size(), 0u);
	EXPECT_EQ(emptyRange.begin(), emptyRange.end());

	int loopCount = 0;
	for (const auto& item : emptyRange)
	{
		(void)item;
		++loopCount;
	}
	EXPECT_EQ(loopCount, 0);

	// 2. Инициализация буфера с 3 вершинами Vertex3D (интерливнутые данные)
	std::vector<Vertex3D> vertices = {
		Vertex3D(Vec3f(1.f, 0.f, 0.f), Vec3f(0.f, 1.f, 0.f), Vec2f(0.1f, 0.2f), Palette4::Red),
		Vertex3D(Vec3f(2.f, 0.f, 0.f), Vec3f(0.f, 0.f, 1.f), Vec2f(0.3f, 0.4f), Palette4::Green),
		Vertex3D(Vec3f(3.f, 0.f, 0.f), Vec3f(1.f, 0.f, 0.f), Vec2f(0.5f, 0.6f), Palette4::Blue)
	};

	const std::byte* rawBytes = reinterpret_cast<const std::byte*>(vertices.data());
	const std::size_t stride = sizeof(Vertex3D); // 64 байта

	// Диапазон позиций (offset = 0)
	AttributeRange<Vec3f> posRange(rawBytes + offsetof(Vertex3D, position), stride, vertices.size());
	EXPECT_FALSE(posRange.empty());
	EXPECT_EQ(posRange.size(), 3u);
	EXPECT_EQ(posRange[0], Vec3f(1.f, 0.f, 0.f));
	EXPECT_EQ(posRange[1], Vec3f(2.f, 0.f, 0.f));
	EXPECT_EQ(posRange[2], Vec3f(3.f, 0.f, 0.f));

	std::vector<Vec3f> collectedPositions;
	for (const auto& pos : posRange)
	{
		collectedPositions.push_back(pos);
	}
	EXPECT_EQ(collectedPositions.size(), 3u);
	EXPECT_EQ(collectedPositions[0], Vec3f(1.f, 0.f, 0.f));
	EXPECT_EQ(collectedPositions[1], Vec3f(2.f, 0.f, 0.f));
	EXPECT_EQ(collectedPositions[2], Vec3f(3.f, 0.f, 0.f));

	// Диапазон UV-координат (offset = offsetof(Vertex3D, texCoord))
	AttributeRange<Vec2f> uvRange(rawBytes + offsetof(Vertex3D, texCoord), stride, vertices.size());
	EXPECT_EQ(uvRange.size(), 3u);
	EXPECT_EQ(uvRange[0], Vec2f(0.1f, 0.2f));
	EXPECT_EQ(uvRange[1], Vec2f(0.3f, 0.4f));
	EXPECT_EQ(uvRange[2], Vec2f(0.5f, 0.6f));

	// Диапазон цветов (offset = offsetof(Vertex3D, color))
	AttributeRange<Color4<zF32>> colorRange(rawBytes + offsetof(Vertex3D, color), stride, vertices.size());
	EXPECT_EQ(colorRange.size(), 3u);
	EXPECT_EQ(colorRange[0], Palette4::Red);
	EXPECT_EQ(colorRange[1], Palette4::Green);
	EXPECT_EQ(colorRange[2], Palette4::Blue);
}

// =============================================================================
// 4. Тесты строковых конвертеров ToString
// =============================================================================

TEST(EnumsAndStructuresTests, ToStringAllValues)
{
	// eResourceType
	EXPECT_EQ(ToString(eResourceType::Unknown), "Unknown");
	EXPECT_EQ(ToString(eResourceType::Texture2D), "Texture2D");
	EXPECT_EQ(ToString(eResourceType::Mesh), "Mesh");
	EXPECT_EQ(ToString(eResourceType::Material), "Material");
	EXPECT_EQ(ToString(eResourceType::Shader), "Shader");
	EXPECT_EQ(ToString(eResourceType::AudioClip), "AudioClip");
	EXPECT_EQ(ToString(eResourceType::Font), "Font");
	EXPECT_EQ(ToString(eResourceType::Scene), "Scene");
	EXPECT_EQ(ToString(eResourceType::Prefab), "Prefab");
	EXPECT_EQ(ToString(eResourceType::BinaryData), "BinaryData");

	// ePixelFormat
	EXPECT_EQ(ToString(ePixelFormat::Unknown), "Unknown");
	EXPECT_EQ(ToString(ePixelFormat::R8_UNORM), "R8_UNORM");
	EXPECT_EQ(ToString(ePixelFormat::RGBA8_UNORM), "RGBA8_UNORM");
	EXPECT_EQ(ToString(ePixelFormat::RGBA8_SRGB), "RGBA8_SRGB");
	EXPECT_EQ(ToString(ePixelFormat::BGRA8_UNORM), "BGRA8_UNORM");
	EXPECT_EQ(ToString(ePixelFormat::BGRA8_SRGB), "BGRA8_SRGB");
	EXPECT_EQ(ToString(ePixelFormat::RGBA16_FLOAT), "RGBA16_FLOAT");
	EXPECT_EQ(ToString(ePixelFormat::R32_FLOAT), "R32_FLOAT");
	EXPECT_EQ(ToString(ePixelFormat::D32_FLOAT), "D32_FLOAT");
	EXPECT_EQ(ToString(ePixelFormat::D24_UNORM_S8_UINT), "D24_UNORM_S8_UINT");
	EXPECT_EQ(ToString(ePixelFormat::D32_FLOAT_S8_UINT), "D32_FLOAT_S8_UINT");
	EXPECT_EQ(ToString(ePixelFormat::BC1_UNORM), "BC1_UNORM");
	EXPECT_EQ(ToString(ePixelFormat::BC3_UNORM), "BC3_UNORM");
	EXPECT_EQ(ToString(ePixelFormat::BC7_UNORM), "BC7_UNORM");
	EXPECT_EQ(ToString(ePixelFormat::ASTC_4x4_UNORM), "ASTC_4x4_UNORM");
	EXPECT_EQ(ToString(ePixelFormat::ETC2_RGBA8_UNORM), "ETC2_RGBA8_UNORM");

	// eIndexFormat
	EXPECT_EQ(ToString(eIndexFormat::UInt16), "UInt16");
	EXPECT_EQ(ToString(eIndexFormat::UInt32), "UInt32");

	// eVertexSemantic
	EXPECT_EQ(ToString(eVertexSemantic::Position), "Position");
	EXPECT_EQ(ToString(eVertexSemantic::Normal), "Normal");
	EXPECT_EQ(ToString(eVertexSemantic::TexCoord), "TexCoord");
	EXPECT_EQ(ToString(eVertexSemantic::Color), "Color");
	EXPECT_EQ(ToString(eVertexSemantic::Tangent), "Tangent");
	EXPECT_EQ(ToString(eVertexSemantic::Bitangent), "Bitangent");
	EXPECT_EQ(ToString(eVertexSemantic::BlendWeight), "BlendWeight");
	EXPECT_EQ(ToString(eVertexSemantic::BlendIndices), "BlendIndices");
	EXPECT_EQ(ToString(eVertexSemantic::Count), "Count");
}

// =============================================================================
// 5. Тесты утилит PixelFormatUtils и IndexFormatUtils
// =============================================================================

TEST(EnumsAndStructuresTests, FormatUtilsFunctions)
{
	using namespace PixelFormatUtils;

	// Размеры в байтах
	EXPECT_EQ(GetPixelFormatBytesPerPixel(ePixelFormat::R8_UNORM), 1u);
	EXPECT_EQ(GetPixelFormatBytesPerPixel(ePixelFormat::RGBA8_UNORM), 4u);
	EXPECT_EQ(GetPixelFormatBytesPerPixel(ePixelFormat::RGBA8_SRGB), 4u);
	EXPECT_EQ(GetPixelFormatBytesPerPixel(ePixelFormat::BGRA8_UNORM), 4u);
	EXPECT_EQ(GetPixelFormatBytesPerPixel(ePixelFormat::BGRA8_SRGB), 4u);
	EXPECT_EQ(GetPixelFormatBytesPerPixel(ePixelFormat::R32_FLOAT), 4u);
	EXPECT_EQ(GetPixelFormatBytesPerPixel(ePixelFormat::D32_FLOAT), 4u);
	EXPECT_EQ(GetPixelFormatBytesPerPixel(ePixelFormat::D24_UNORM_S8_UINT), 4u);
	EXPECT_EQ(GetPixelFormatBytesPerPixel(ePixelFormat::RGBA16_FLOAT), 8u);
	EXPECT_EQ(GetPixelFormatBytesPerPixel(ePixelFormat::D32_FLOAT_S8_UINT), 8u);
	EXPECT_EQ(GetPixelFormatBytesPerPixel(ePixelFormat::BC1_UNORM), 0u);
	EXPECT_EQ(GetPixelFormatBytesPerPixel(ePixelFormat::BC7_UNORM), 0u);

	// Проверки свойств
	EXPECT_TRUE(IsDepthFormat(ePixelFormat::D32_FLOAT));
	EXPECT_TRUE(IsDepthFormat(ePixelFormat::D24_UNORM_S8_UINT));
	EXPECT_TRUE(IsDepthFormat(ePixelFormat::D32_FLOAT_S8_UINT));
	EXPECT_FALSE(IsDepthFormat(ePixelFormat::RGBA8_UNORM));

	EXPECT_TRUE(IsStencilFormat(ePixelFormat::D24_UNORM_S8_UINT));
	EXPECT_TRUE(IsStencilFormat(ePixelFormat::D32_FLOAT_S8_UINT));
	EXPECT_FALSE(IsStencilFormat(ePixelFormat::D32_FLOAT));

	EXPECT_TRUE(IsSRGBFormat(ePixelFormat::RGBA8_SRGB));
	EXPECT_TRUE(IsSRGBFormat(ePixelFormat::BGRA8_SRGB));
	EXPECT_FALSE(IsSRGBFormat(ePixelFormat::RGBA8_UNORM));

	EXPECT_TRUE(IsCompressedFormat(ePixelFormat::BC1_UNORM));
	EXPECT_TRUE(IsCompressedFormat(ePixelFormat::BC3_UNORM));
	EXPECT_TRUE(IsCompressedFormat(ePixelFormat::BC7_UNORM));
	EXPECT_TRUE(IsCompressedFormat(ePixelFormat::ASTC_4x4_UNORM));
	EXPECT_TRUE(IsCompressedFormat(ePixelFormat::ETC2_RGBA8_UNORM));
	EXPECT_FALSE(IsCompressedFormat(ePixelFormat::RGBA8_UNORM));

	// IndexFormatUtils
	EXPECT_EQ(IndexFormatUtils::GetIndexFormatBytes(eIndexFormat::UInt16), 2u);
	EXPECT_EQ(IndexFormatUtils::GetIndexFormatBytes(eIndexFormat::UInt32), 4u);
}

// =============================================================================
// 6. Тесты конвертера ConverterGAPITypes
// =============================================================================

TEST(EnumsAndStructuresTests, ConverterGAPITypesRoundTrip)
{
	// Проверка двусторонней конвертации форматов пикселей
	const std::vector<ePixelFormat> testPixelFormats = {
		ePixelFormat::R8_UNORM,
		ePixelFormat::RGBA8_UNORM,
		ePixelFormat::RGBA8_SRGB,
		ePixelFormat::BGRA8_UNORM,
		ePixelFormat::BGRA8_SRGB,
		ePixelFormat::RGBA16_FLOAT,
		ePixelFormat::R32_FLOAT,
		ePixelFormat::D32_FLOAT,
		ePixelFormat::D24_UNORM_S8_UINT,
		ePixelFormat::D32_FLOAT_S8_UINT,
		ePixelFormat::BC1_UNORM,
		ePixelFormat::BC3_UNORM,
		ePixelFormat::BC7_UNORM
	};

	for (const auto fmt : testPixelFormats)
	{
		NativePixelFormat nativeFmt = ConverterGAPITypes::ToNative(fmt);
		ePixelFormat engineFmt = ConverterGAPITypes::ToEnginePixelFormat(nativeFmt);
		EXPECT_EQ(engineFmt, fmt);
		EXPECT_EQ(ConverterGAPITypes::ToEngine<ePixelFormat>(nativeFmt), fmt);
	}

	// Проверка двусторонней конвертации форматов индексов
	NativeIndexFormat nativeIdx16 = ConverterGAPITypes::ToNative(eIndexFormat::UInt16);
	EXPECT_EQ(ConverterGAPITypes::ToEngineIndexFormat(nativeIdx16), eIndexFormat::UInt16);
	EXPECT_EQ(ConverterGAPITypes::ToEngine<eIndexFormat>(nativeIdx16), eIndexFormat::UInt16);

	NativeIndexFormat nativeIdx32 = ConverterGAPITypes::ToNative(eIndexFormat::UInt32);
	EXPECT_EQ(ConverterGAPITypes::ToEngineIndexFormat(nativeIdx32), eIndexFormat::UInt32);
	EXPECT_EQ(ConverterGAPITypes::ToEngine<eIndexFormat>(nativeIdx32), eIndexFormat::UInt32);
}

// =============================================================================
// 7. Тесты бинарной сериализации Serializer
// =============================================================================

TEST(EnumsAndStructuresTests, BinarySerializationEnumsAndVertex3D)
{
	Serializer serializer;

	// 1. Сериализация перечислений
	std::vector<std::byte> enumBuffer;
	ASSERT_TRUE(serializer.Serialize(enumBuffer, eResourceType::Texture2D).has_value());
	ASSERT_TRUE(serializer.Serialize(enumBuffer, ePixelFormat::RGBA8_SRGB).has_value());
	ASSERT_TRUE(serializer.Serialize(enumBuffer, eIndexFormat::UInt16).has_value());
	ASSERT_TRUE(serializer.Serialize(enumBuffer, eVertexSemantic::Normal).has_value());

	// Десериализация перечислений
	std::size_t offset = 0;
	eResourceType resType = eResourceType::Unknown;
	ePixelFormat pixFormat = ePixelFormat::Unknown;
	eIndexFormat idxFormat = eIndexFormat::UInt32;
	eVertexSemantic vSemantic = eVertexSemantic::Position;

	ASSERT_TRUE(serializer.Deserialize(enumBuffer, offset, resType).has_value());
	ASSERT_TRUE(serializer.Deserialize(enumBuffer, offset, pixFormat).has_value());
	ASSERT_TRUE(serializer.Deserialize(enumBuffer, offset, idxFormat).has_value());
	ASSERT_TRUE(serializer.Deserialize(enumBuffer, offset, vSemantic).has_value());

	EXPECT_EQ(resType, eResourceType::Texture2D);
	EXPECT_EQ(pixFormat, ePixelFormat::RGBA8_SRGB);
	EXPECT_EQ(idxFormat, eIndexFormat::UInt16);
	EXPECT_EQ(vSemantic, eVertexSemantic::Normal);

	// 2. Сериализация и десериализация Vertex3D
	Vertex3D originalVertex(
		Vec3f(10.5f, -20.25f, 30.125f),
		Vec3f(0.0f, 1.0f, 0.0f),
		Vec2f(0.25f, 0.75f),
		Palette4::Cyan,
		Vec4f(0.5f, 0.5f, 0.0f, 1.0f)
	);

	std::vector<std::byte> vertexBuffer;
	ASSERT_TRUE(serializer.Serialize(vertexBuffer, originalVertex).has_value());

	std::size_t vertexOffset = 0;
	Vertex3D deserializedVertex;
	ASSERT_TRUE(serializer.Deserialize(vertexBuffer, vertexOffset, deserializedVertex).has_value());

	EXPECT_EQ(deserializedVertex, originalVertex);
	EXPECT_EQ(deserializedVertex.position, Vec3f(10.5f, -20.25f, 30.125f));
	EXPECT_EQ(deserializedVertex.normal, Vec3f(0.0f, 1.0f, 0.0f));
	EXPECT_EQ(deserializedVertex.texCoord, Vec2f(0.25f, 0.75f));
	EXPECT_EQ(deserializedVertex.color, Palette4::Cyan);
	EXPECT_EQ(deserializedVertex.tangent, Vec4f(0.5f, 0.5f, 0.0f, 1.0f));
}

#endif // Z_TEST_CORE_ENUMS_STRUCTS
