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

#endif // Z_TEST_CORE_SERIALIZATION
