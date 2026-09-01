#include "qa/tests/TestsConfig.h"

#ifdef Z_TEST_CORE_TEMPLATES

#include <gtest/gtest.h>
#include "math/Math.h"
#include "core/serialize/Serializer.h"

using namespace zzz;
using namespace zzz::math;

TEST(ColorTest, DefaultConstructors)
{
	Color3<zF32> c3;
	EXPECT_FLOAT_EQ(c3.R, 0.0f);
	EXPECT_FLOAT_EQ(c3.G, 0.0f);
	EXPECT_FLOAT_EQ(c3.B, 0.0f);

	Color4<zF32> c4;
	EXPECT_FLOAT_EQ(c4.R, 0.0f);
	EXPECT_FLOAT_EQ(c4.G, 0.0f);
	EXPECT_FLOAT_EQ(c4.B, 0.0f);
	EXPECT_FLOAT_EQ(c4.A, 1.0f);

	Color4<zU8> c4u;
	EXPECT_EQ(c4u.A, 255);
}

TEST(ColorTest, ConvertChannelLinearScaling)
{
	// 255 -> 1.0f via ConvertTo
	Color4<zU8> cByte(255, 128, 0, 255);
	Color4<zF32> cFloat = cByte.ConvertTo<zF32>();
	EXPECT_FLOAT_EQ(cFloat.R, 1.0f);
	EXPECT_NEAR(cFloat.G, 128.0f / 255.0f, 0.001f);
	EXPECT_FLOAT_EQ(cFloat.B, 0.0f);
	EXPECT_FLOAT_EQ(cFloat.A, 1.0f);

	// 510 -> 2.0f without clamping via SetFrom
	Color4<zF32> cFloat2;
	cFloat2.SetFrom(510, 255, 0, 255);
	EXPECT_FLOAT_EQ(cFloat2.R, 2.0f);
	EXPECT_FLOAT_EQ(cFloat2.G, 1.0f);
}

TEST(ColorTest, Color3Color4Conversions)
{
	Color3<zF32> rgb(1.0f, 0.5f, 0.25f);
	Color4<zF32> rgba(rgb, 0.75f);

	EXPECT_FLOAT_EQ(rgba.R, 1.0f);
	EXPECT_FLOAT_EQ(rgba.G, 0.5f);
	EXPECT_FLOAT_EQ(rgba.B, 0.25f);
	EXPECT_FLOAT_EQ(rgba.A, 0.75f);

	Color3<zF32> extracted = rgba.GetRGB();
	EXPECT_EQ(extracted, rgb);
}

TEST(ColorTest, ClampAndLerp)
{
	Color4<zF32> overshooting(1.5f, -0.2f, 0.5f, 1.0f);
	Color4<zF32> clamped = overshooting.Clamped(0.0f, 1.0f);

	EXPECT_FLOAT_EQ(clamped.R, 1.0f);
	EXPECT_FLOAT_EQ(clamped.G, 0.0f);
	EXPECT_FLOAT_EQ(clamped.B, 0.5f);

	Color4<zF32> start(0.0f, 0.0f, 0.0f, 1.0f);
	Color4<zF32> end(1.0f, 1.0f, 1.0f, 1.0f);
	Color4<zF32> mid = start.Lerp(end, 0.5f);

	EXPECT_FLOAT_EQ(mid.R, 0.5f);
	EXPECT_FLOAT_EQ(mid.G, 0.5f);
	EXPECT_FLOAT_EQ(mid.B, 0.5f);
}

TEST(ColorTest, Palette)
{
	EXPECT_FLOAT_EQ(Palette3::Red.R, 1.0f);
	EXPECT_FLOAT_EQ(Palette3::Red.G, 0.0f);
	EXPECT_FLOAT_EQ(Palette3::Red.B, 0.0f);

	EXPECT_FLOAT_EQ(Palette4::Transparent.A, 0.0f);
}

TEST(ColorTest, Serialization)
{
	core::Serializer serializer;
	std::vector<std::byte> buffer;

	Color4<zF32> originalColor(0.25f, 0.5f, 0.75f, 1.0f);
	auto serRes = serializer.Serialize(buffer, originalColor);
	EXPECT_TRUE(serRes.has_value());

	Color4<zF32> restoredColor;
	std::size_t offset = 0;
	auto deserRes = serializer.Deserialize(buffer, offset, restoredColor);
	EXPECT_TRUE(deserRes.has_value());

	EXPECT_EQ(originalColor, restoredColor);
}

#endif // Z_TEST_CORE_TEMPLATES
