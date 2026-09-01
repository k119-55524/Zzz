#include "qa/tests/TestsConfig.h"

#ifdef Z_TEST_MATH_SIZE2D

#include <gtest/gtest.h>
#include "math/Math.h"

using namespace zzz;
using namespace zzz::math;

TEST(Size2DTest, UnificationAndAccess)
{
	Size2D<zU32> sz(800, 600);
	EXPECT_EQ(sz.width, 800u);
	EXPECT_EQ(sz.height, 600u);
	EXPECT_EQ(sz.GetWidth(), 800u);
	EXPECT_EQ(sz.GetHeight(), 600u);

	sz.width = 1024;
	sz.height = 768;
	EXPECT_EQ(sz.GetWidth(), 1024u);
	EXPECT_EQ(sz.GetHeight(), 768u);

	EXPECT_EQ(sz[0], 1024u);
	EXPECT_EQ(sz[1], 768u);
	EXPECT_EQ(sz.data(), &sz.width);
}

#endif // Z_TEST_MATH_SIZE2D
