#include "qa/tests/TestsConfig.h"

#ifdef Z_TEST_MATH_MAT4

#include <gtest/gtest.h>
#include "math/Math.h"
#include "core/serialize/Serializer.h"

using namespace zzz;
using namespace zzz::math;
using namespace zzz::core;

TEST(Mat4Test, DefaultAndIdentity)
{
	Mat4f identity;
	for (size_t r = 0; r < 4; ++r)
	{
		for (size_t c = 0; c < 4; ++c)
		{
			if (r == c)
				EXPECT_FLOAT_EQ(identity(r, c), 1.0f);
			else
				EXPECT_FLOAT_EQ(identity(r, c), 0.0f);
		}
	}

	EXPECT_FLOAT_EQ(identity.Determinant(), 1.0f);
	EXPECT_EQ(identity, Mat4f::Identity());
}

TEST(Mat4Test, ZeroAndDiagonal)
{
	Mat4f zero = Mat4f::Zero();
	for (size_t i = 0; i < 16; ++i)
		EXPECT_FLOAT_EQ(zero[i], 0.0f);

	EXPECT_FLOAT_EQ(zero.Determinant(), 0.0f);

	Mat4f diag(5.0f);
	EXPECT_FLOAT_EQ(diag._11, 5.0f);
	EXPECT_FLOAT_EQ(diag._22, 5.0f);
	EXPECT_FLOAT_EQ(diag._33, 5.0f);
	EXPECT_FLOAT_EQ(diag._44, 5.0f);
	EXPECT_FLOAT_EQ(diag._12, 0.0f);
}

TEST(Mat4Test, AccessorsAndRowsCols)
{
	Mat4f m;
	m.SetRow(0, Vec4f(1.0f, 2.0f, 3.0f, 4.0f));
	m.SetRow(1, Vec4f(5.0f, 6.0f, 7.0f, 8.0f));
	m.SetRow(2, Vec4f(9.0f, 10.0f, 11.0f, 12.0f));
	m.SetRow(3, Vec4f(13.0f, 14.0f, 15.0f, 16.0f));

	Vec4f r0 = m.GetRow(0);
	EXPECT_FLOAT_EQ(r0.x, 1.0f);
	EXPECT_FLOAT_EQ(r0.y, 2.0f);
	EXPECT_FLOAT_EQ(r0.z, 3.0f);
	EXPECT_FLOAT_EQ(r0.w, 4.0f);

	Vec4f c1 = m.GetColumn(1);
	EXPECT_FLOAT_EQ(c1.x, 2.0f);
	EXPECT_FLOAT_EQ(c1.y, 6.0f);
	EXPECT_FLOAT_EQ(c1.z, 10.0f);
	EXPECT_FLOAT_EQ(c1.w, 14.0f);

	EXPECT_EQ(m.data(), &m._11);
}

TEST(Mat4Test, MultiplicationAndAssociativity)
{
	Mat4f a = Mat4f::Translation(1.0f, 2.0f, 3.0f);
	Mat4f b = Mat4f::Scaling(2.0f, 3.0f, 4.0f);
	Mat4f c = Mat4f::RotationZ(3.14159265f / 4.0f);

	Mat4f ab = a * b;
	Mat4f ab_c = ab * c;
	Mat4f bc = b * c;
	Mat4f a_bc = a * bc;

	for (size_t i = 0; i < 16; ++i)
	{
		EXPECT_NEAR(ab_c[i], a_bc[i], 1e-4f);
	}
}

TEST(Mat4Test, Transpose)
{
	Mat4f m(
		1.0f, 2.0f, 3.0f, 4.0f,
		5.0f, 6.0f, 7.0f, 8.0f,
		9.0f, 10.0f, 11.0f, 12.0f,
		13.0f, 14.0f, 15.0f, 16.0f
	);

	Mat4f t = m.Transpose();
	EXPECT_FLOAT_EQ(t._12, 5.0f);
	EXPECT_FLOAT_EQ(t._21, 2.0f);
	EXPECT_FLOAT_EQ(t._14, 13.0f);
	EXPECT_FLOAT_EQ(t._41, 4.0f);

	Mat4f tt = t.Transpose();
	EXPECT_EQ(m, tt);
}

TEST(Mat4Test, InverseAndDeterminant)
{
	Mat4f m = Mat4f::TRS(Vec3f(3.0f, -5.0f, 2.0f), Vec3f(0.2f, 0.5f, -0.3f), Vec3f(2.0f, 1.5f, 0.5f));
	EXPECT_NE(m.Determinant(), 0.0f);

	bool invertible = false;
	Mat4f inv = m.Inverse(&invertible);
	EXPECT_TRUE(invertible);

	Mat4f identityCheck = m * inv;
	for (size_t r = 0; r < 4; ++r)
	{
		for (size_t c = 0; c < 4; ++c)
		{
			const float expected = (r == c) ? 1.0f : 0.0f;
			EXPECT_NEAR(identityCheck(r, c), expected, 1e-4f);
		}
	}

	// Тест вырожденной матрицы
	Mat4f singular = Mat4f::Zero();
	bool singularInvertible = true;
	Mat4f singularInv = singular.Inverse(&singularInvertible);
	EXPECT_FALSE(singularInvertible);
	EXPECT_EQ(singularInv, Mat4f::Identity());
}

TEST(Mat4Test, VectorAndPointTransformation)
{
	Mat4f t = Mat4f::Translation(10.0f, 20.0f, 30.0f);

	// Точка должна смещаться
	Vec3f pt(1.0f, 2.0f, 3.0f);
	Vec3f ptTransformed = t.TransformPoint(pt);
	EXPECT_FLOAT_EQ(ptTransformed.x, 11.0f);
	EXPECT_FLOAT_EQ(ptTransformed.y, 22.0f);
	EXPECT_FLOAT_EQ(ptTransformed.z, 33.0f);

	// Вектор/нормаль не должен смещаться
	Vec3f dir(1.0f, 2.0f, 3.0f);
	Vec3f dirTransformed = t.TransformVector(dir);
	EXPECT_FLOAT_EQ(dirTransformed.x, 1.0f);
	EXPECT_FLOAT_EQ(dirTransformed.y, 2.0f);
	EXPECT_FLOAT_EQ(dirTransformed.z, 3.0f);

	// Умножение вектора на матрицу через v * M
	Vec4f v4(1.0f, 2.0f, 3.0f, 1.0f);
	Vec4f v4Res = v4 * t;
	EXPECT_FLOAT_EQ(v4Res.x, 11.0f);
	EXPECT_FLOAT_EQ(v4Res.y, 22.0f);
	EXPECT_FLOAT_EQ(v4Res.z, 33.0f);
	EXPECT_FLOAT_EQ(v4Res.w, 1.0f);
}

TEST(Mat4Test, Rotations)
{
	const float pi = 3.14159265358979323846f;
	Mat4f rotZ = Mat4f::RotationZ(pi * 0.5f); // 90 deg around Z
	Vec3f v(1.0f, 0.0f, 0.0f);
	Vec3f resZ = rotZ.TransformVector(v);
	EXPECT_NEAR(resZ.x, 0.0f, 1e-5f);
	EXPECT_NEAR(resZ.y, 1.0f, 1e-5f);
	EXPECT_NEAR(resZ.z, 0.0f, 1e-5f);

	Mat4f rotY = Mat4f::RotationY(pi * 0.5f); // 90 deg around Y
	Vec3f resY = rotY.TransformVector(Vec3f(0.0f, 0.0f, 1.0f));
	EXPECT_NEAR(resY.x, 1.0f, 1e-5f);
	EXPECT_NEAR(resY.y, 0.0f, 1e-5f);
	EXPECT_NEAR(resY.z, 0.0f, 1e-5f);
}

TEST(Mat4Test, LookAtLH)
{
	Vec3f eye(0.0f, 0.0f, -5.0f);
	Vec3f target(0.0f, 0.0f, 0.0f);
	Vec3f up(0.0f, 1.0f, 0.0f);

	Mat4f view = Mat4f::LookAtLH(eye, target, up);

	// Позиция камеры в пространстве вида должна быть (0, 0, 0)
	Vec3f eyeView = view.TransformPoint(eye);
	EXPECT_NEAR(eyeView.x, 0.0f, 1e-5f);
	EXPECT_NEAR(eyeView.y, 0.0f, 1e-5f);
	EXPECT_NEAR(eyeView.z, 0.0f, 1e-5f);

	// Таргет должен быть на расстоянии +5 по Z
	Vec3f targetView = view.TransformPoint(target);
	EXPECT_NEAR(targetView.x, 0.0f, 1e-5f);
	EXPECT_NEAR(targetView.y, 0.0f, 1e-5f);
	EXPECT_NEAR(targetView.z, 5.0f, 1e-5f);
}

TEST(Mat4Test, PerspectiveFovLH_NDCDepthRange)
{
	const float fovY = 3.14159265f / 3.0f; // 60 deg
	const float aspect = 16.0f / 9.0f;
	const float nearZ = 0.1f;
	const float farZ = 1000.0f;

	Mat4f proj = Mat4f::PerspectiveFovLH(fovY, aspect, nearZ, farZ);

	// Точка на ближней плоскости Z = nearZ должна проецироваться в NDC Z = 0.0
	Vec3f ptNear(0.0f, 0.0f, nearZ);
	Vec3f ndcNear = proj.TransformPoint(ptNear);
	EXPECT_NEAR(ndcNear.x, 0.0f, 1e-4f);
	EXPECT_NEAR(ndcNear.y, 0.0f, 1e-4f);
	EXPECT_NEAR(ndcNear.z, 0.0f, 1e-4f);

	// Точка на дальней плоскости Z = farZ должна проецироваться в NDC Z = 1.0
	Vec3f ptFar(0.0f, 0.0f, farZ);
	Vec3f ndcFar = proj.TransformPoint(ptFar);
	EXPECT_NEAR(ndcFar.x, 0.0f, 1e-4f);
	EXPECT_NEAR(ndcFar.y, 0.0f, 1e-4f);
	EXPECT_NEAR(ndcFar.z, 1.0f, 1e-4f);
}

TEST(Mat4Test, OrthographicLH_NDCDepthRange)
{
	const float width = 800.0f;
	const float height = 600.0f;
	const float nearZ = 0.0f;
	const float farZ = 100.0f;

	Mat4f ortho = Mat4f::OrthographicLH(width, height, nearZ, farZ);

	Vec3f ptNear(0.0f, 0.0f, nearZ);
	Vec3f ndcNear = ortho.TransformPoint(ptNear);
	EXPECT_NEAR(ndcNear.z, 0.0f, 1e-5f);

	Vec3f ptFar(0.0f, 0.0f, farZ);
	Vec3f ndcFar = ortho.TransformPoint(ptFar);
	EXPECT_NEAR(ndcFar.z, 1.0f, 1e-5f);
}

TEST(Mat4Test, SerializationRoundTrip)
{
	Mat4f original(
		1.5f, 2.5f, 3.5f, 4.5f,
		5.5f, 6.5f, 7.5f, 8.5f,
		9.5f, 10.5f, 11.5f, 12.5f,
		13.5f, 14.5f, 15.5f, 16.5f
	);

	Serializer serializer;
	std::vector<std::byte> buffer;
	auto serRes = serializer.Serialize(buffer, original);
	EXPECT_TRUE(serRes.has_value());
	EXPECT_EQ(buffer.size(), sizeof(Mat4f));

	Mat4f deserialized = Mat4f::Zero();
	std::size_t offset = 0;
	auto deserRes = serializer.Deserialize(buffer, offset, deserialized);
	EXPECT_TRUE(deserRes.has_value());
	EXPECT_EQ(offset, buffer.size());

	for (size_t i = 0; i < 16; ++i)
	{
		EXPECT_FLOAT_EQ(deserialized[i], original[i]);
	}
}

#endif // Z_TEST_MATH_MAT4
