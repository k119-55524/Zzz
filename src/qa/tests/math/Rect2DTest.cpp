#include "qa/tests/TestsConfig.h"

#ifdef Z_TEST_MATH_RECT2D

#include <gtest/gtest.h>
#include "math/Math.h"

using namespace zzz;
using namespace zzz::math;

TEST(Rect2DTest, UnificationAndAccess)
{
	Rect2D<zI32> rect(10, 20, 300, 400);
	EXPECT_EQ(rect.position.x, 10);
	EXPECT_EQ(rect.position.y, 20);
	EXPECT_EQ(rect.size.width, 300u);
	EXPECT_EQ(rect.size.height, 400u);

	EXPECT_EQ(rect.Left(), 10);
	EXPECT_EQ(rect.Top(), 20);
	EXPECT_EQ(rect.Right(), 310);
	EXPECT_EQ(rect.Bottom(), 420);
	EXPECT_EQ(rect.Width(), 300);
	EXPECT_EQ(rect.Height(), 400);

	EXPECT_TRUE(rect.Contains(50, 50));
	EXPECT_FALSE(rect.Contains(500, 500));
}

#endif // Z_TEST_MATH_RECT2D
