#include "qa/tests/TestsConfig.h"

#ifdef Z_TEST_MATH_MAT3

#include <gtest/gtest.h>
#include "math/Math.h"
#include "core/serialize/Serializer.h"

using namespace zzz;
using namespace zzz::math;
using namespace zzz::core;

TEST(Mat3Test, DefaultAndIdentity)
{
	Mat3f identity;
	for (size_t r = 0; r < 3; ++r)
	{
		for (size_t c = 0; c < 3; ++c)
		{
			if (r == c)
				EXPECT_FLOAT_EQ(identity(r, c), 1.0f);
			else
				EXPECT_FLOAT_EQ(identity(r, c), 0.0f);
		}
	}

	EXPECT_FLOAT_EQ(identity.Determinant(), 1.0f);
	EXPECT_EQ(identity, Mat3f::Identity());
}

TEST(Mat3Test, MultiplicationAndTranspose)
{
	Mat3f a = Mat3f::Scaling(2.0f, 3.0f, 4.0f);
	Mat3f b = Mat3f::RotationZ(3.14159265f / 2.0f);

	Mat3f ab = a * b;
	Mat3f t = ab.Transpose();

	EXPECT_FLOAT_EQ(t._11, ab._11);
	EXPECT_FLOAT_EQ(t._12, ab._21);
	EXPECT_FLOAT_EQ(t._21, ab._12);
	EXPECT_EQ(t.Transpose(), ab);
}

TEST(Mat3Test, InverseAndDeterminant)
{
	Mat3f rot = Mat3f::RotationY(0.785398f) * Mat3f::Scaling(2.0f, 3.0f, 1.5f);
	EXPECT_NE(rot.Determinant(), 0.0f);

	bool invertible = false;
	Mat3f inv = rot.Inverse(&invertible);
	EXPECT_TRUE(invertible);

	Mat3f identityCheck = rot * inv;
	for (size_t r = 0; r < 3; ++r)
	{
		for (size_t c = 0; c < 3; ++c)
		{
			const float expected = (r == c) ? 1.0f : 0.0f;
			EXPECT_NEAR(identityCheck(r, c), expected, 1e-4f);
		}
	}
}

TEST(Mat3Test, VectorTransformation)
{
	Mat3f scale = Mat3f::Scaling(2.0f, 3.0f, 4.0f);
	Vec3f v(1.0f, 1.0f, 1.0f);
	Vec3f scaled = v * scale;

	EXPECT_FLOAT_EQ(scaled.x, 2.0f);
	EXPECT_FLOAT_EQ(scaled.y, 3.0f);
	EXPECT_FLOAT_EQ(scaled.z, 4.0f);
}

TEST(Mat3Test, Mat4ToMat3BasisAndNormalMatrix)
{
	// Non-uniform scaling + rotation + translation
	Mat4f world = Mat4f::TRS(Vec3f(10.0f, 20.0f, 30.0f), Vec3f(0.0f, 0.0f, 0.0f), Vec3f(2.0f, 1.0f, 1.0f));

	// Извлечение базиса
	Mat3f basis = world.ToMat3();
	EXPECT_FLOAT_EQ(basis._11, 2.0f);
	EXPECT_FLOAT_EQ(basis._22, 1.0f);
	EXPECT_FLOAT_EQ(basis._33, 1.0f);

	// Вычисление Normal Matrix: Transpose(Inverse(basis))
	Mat3f normalMat = world.GetNormalMatrix();
	// Для масштаба (2, 1, 1), нормаль вдоль X {1, 0, 0} должна масштабироваться с фактором 1/2 = 0.5
	Vec3f normalX(1.0f, 0.0f, 0.0f);
	Vec3f transformedNormal = (normalX * normalMat).Normalized();
	EXPECT_NEAR(transformedNormal.x, 1.0f, 1e-5f);
	EXPECT_NEAR(transformedNormal.y, 0.0f, 1e-5f);
	EXPECT_NEAR(transformedNormal.z, 0.0f, 1e-5f);
}

TEST(Mat3Test, SerializationRoundTrip)
{
	Mat3f original(
		1.0f, 2.0f, 3.0f,
		4.0f, 5.0f, 6.0f,
		7.0f, 8.0f, 9.0f
	);

	Serializer serializer;
	std::vector<std::byte> buffer;
	auto serRes = serializer.Serialize(buffer, original);
	EXPECT_TRUE(serRes.has_value());
	EXPECT_EQ(buffer.size(), sizeof(Mat3f));

	Mat3f deserialized = Mat3f::Zero();
	std::size_t offset = 0;
	auto deserRes = serializer.Deserialize(buffer, offset, deserialized);
	EXPECT_TRUE(deserRes.has_value());
	EXPECT_EQ(offset, buffer.size());

	for (size_t i = 0; i < 9; ++i)
	{
		EXPECT_FLOAT_EQ(deserialized[i], original[i]);
	}
}

#endif // Z_TEST_MATH_MAT3
