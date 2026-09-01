#include "qa/tests/TestsConfig.h"

#ifdef Z_TEST_MATH_POINT2D

#include <gtest/gtest.h>
#include "math/Math.h"

using namespace zzz;
using namespace zzz::math;

TEST(Point2DTest, UnificationAndAccess)
{
	Point2D<zI32> pt(100, 200);
	EXPECT_EQ(pt.x, 100);
	EXPECT_EQ(pt.y, 200);

	pt.x = 300;
	pt.y = 400;
	EXPECT_EQ(pt.x, 300);
	EXPECT_EQ(pt.y, 400);

	EXPECT_EQ(pt[0], 300);
	EXPECT_EQ(pt[1], 400);
	EXPECT_EQ(pt.data(), &pt.x);

	pt.Offset(10, -20);
	EXPECT_EQ(pt.x, 310);
	EXPECT_EQ(pt.y, 380);

	Point2D<zI32> pt2{ 10, 20 };
	auto added = pt + pt2;
	EXPECT_EQ(added.x, 320);
	EXPECT_EQ(added.y, 400);
}

#endif // Z_TEST_MATH_POINT2D
