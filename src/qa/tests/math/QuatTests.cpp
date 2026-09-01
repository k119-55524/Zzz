#include "qa/tests/TestsConfig.h"

#ifdef Z_TEST_MATH_QUAT

#include <gtest/gtest.h>
#include "math/Math.h"
#include "core/serialize/Serializer.h"

using namespace zzz;
using namespace zzz::math;
using namespace zzz::core;

TEST(QuatTest, MemoryLayoutAndIdentity)
{
	static_assert(sizeof(Quatf) == 16, "Quatf must be exactly 16 bytes");
	static_assert(alignof(Quatf) == 16, "Quatf must be 16-byte aligned");
	static_assert(sizeof(Quatd) == 32, "Quatd must be exactly 32 bytes");
	static_assert(alignof(Quatd) == 32, "Quatd must be 32-byte aligned");
	static_assert(std::is_standard_layout_v<Quatf>, "Quatf must be standard layout");
	static_assert(Quatf::Identity().IsNormalized(), "Identity quaternion must be normalized at compile time");
	static_assert(Quatf::Identity() == Quatf(0.0f, 0.0f, 0.0f, 1.0f), "Identity comparison must work at compile time");

	Quatf defaultQuat;
	EXPECT_FLOAT_EQ(defaultQuat.x, 0.0f);
	EXPECT_FLOAT_EQ(defaultQuat.y, 0.0f);
	EXPECT_FLOAT_EQ(defaultQuat.z, 0.0f);
	EXPECT_FLOAT_EQ(defaultQuat.w, 1.0f);

	Quatf identity = Quatf::Identity();
	EXPECT_EQ(defaultQuat, identity);

	Quatf zero = Quatf::Zero();
	EXPECT_FLOAT_EQ(zero.x, 0.0f);
	EXPECT_FLOAT_EQ(zero.y, 0.0f);
	EXPECT_FLOAT_EQ(zero.z, 0.0f);
	EXPECT_FLOAT_EQ(zero.w, 0.0f);
}

TEST(QuatTest, AccessorsAndData)
{
	Quatf q(1.0f, 2.0f, 3.0f, 4.0f);
	EXPECT_FLOAT_EQ(q.x, 1.0f);
	EXPECT_FLOAT_EQ(q.y, 2.0f);
	EXPECT_FLOAT_EQ(q.z, 3.0f);
	EXPECT_FLOAT_EQ(q.w, 4.0f);

	EXPECT_FLOAT_EQ(q[0], 1.0f);
	EXPECT_FLOAT_EQ(q[1], 2.0f);
	EXPECT_FLOAT_EQ(q[2], 3.0f);
	EXPECT_FLOAT_EQ(q[3], 4.0f);

	q[1] = 20.0f;
	EXPECT_FLOAT_EQ(q.y, 20.0f);

	const float* ptr = q.data();
	EXPECT_FLOAT_EQ(ptr[0], 1.0f);
	EXPECT_FLOAT_EQ(ptr[1], 20.0f);
	EXPECT_FLOAT_EQ(ptr[2], 3.0f);
	EXPECT_FLOAT_EQ(ptr[3], 4.0f);

	Vec3f vPart = q.GetVectorPart();
	EXPECT_FLOAT_EQ(vPart.x, 1.0f);
	EXPECT_FLOAT_EQ(vPart.y, 20.0f);
	EXPECT_FLOAT_EQ(vPart.z, 3.0f);
	EXPECT_FLOAT_EQ(q.GetScalarPart(), 4.0f);
}

TEST(QuatTest, BasicAlgebraAndNorms)
{
	Quatf q1(1.0f, 2.0f, 3.0f, 4.0f);
	Quatf q2(5.0f, 6.0f, 7.0f, 8.0f);

	Quatf sum = q1 + q2;
	EXPECT_FLOAT_EQ(sum.x, 6.0f);
	EXPECT_FLOAT_EQ(sum.y, 8.0f);
	EXPECT_FLOAT_EQ(sum.z, 10.0f);
	EXPECT_FLOAT_EQ(sum.w, 12.0f);

	Quatf diff = q2 - q1;
	EXPECT_FLOAT_EQ(diff.x, 4.0f);
	EXPECT_FLOAT_EQ(diff.y, 4.0f);
	EXPECT_FLOAT_EQ(diff.z, 4.0f);
	EXPECT_FLOAT_EQ(diff.w, 4.0f);

	Quatf scaled = q1 * 2.0f;
	EXPECT_FLOAT_EQ(scaled.x, 2.0f);
	EXPECT_FLOAT_EQ(scaled.y, 4.0f);
	EXPECT_FLOAT_EQ(scaled.z, 6.0f);
	EXPECT_FLOAT_EQ(scaled.w, 8.0f);

	Quatf div = scaled / 2.0f;
	EXPECT_FLOAT_EQ(div.x, 1.0f);
	EXPECT_FLOAT_EQ(div.y, 2.0f);
	EXPECT_FLOAT_EQ(div.z, 3.0f);
	EXPECT_FLOAT_EQ(div.w, 4.0f);

	EXPECT_FLOAT_EQ(q1.Dot(q2), 1.0f * 5.0f + 2.0f * 6.0f + 3.0f * 7.0f + 4.0f * 8.0f);

	Quatf unitQ(0.0f, 0.0f, 0.0f, 1.0f);
	EXPECT_FLOAT_EQ(unitQ.Length(), 1.0f);
	EXPECT_FLOAT_EQ(unitQ.LengthSquared(), 1.0f);
	EXPECT_TRUE(unitQ.IsNormalized());

	Quatf nonUnit(1.0f, 1.0f, 1.0f, 1.0f);
	EXPECT_FLOAT_EQ(nonUnit.LengthSquared(), 4.0f);
	EXPECT_FLOAT_EQ(nonUnit.Length(), 2.0f);
	Quatf norm = nonUnit.Normalized();
	EXPECT_FLOAT_EQ(norm.Length(), 1.0f);
	EXPECT_TRUE(norm.IsNormalized());
}

TEST(QuatTest, QuaternionMultiplication)
{
	Quatf q1 = Quatf::FromAxisAngle(Vec3f::Up(), 3.14159265f * 0.5f); // 90 deg around Y
	Quatf identity = Quatf::Identity();

	Quatf res = q1 * identity;
	EXPECT_EQ(res, q1);

	res = identity * q1;
	EXPECT_EQ(res, q1);

	Quatf inv = q1.Inverse();
	Quatf multInv = q1 * inv;
	EXPECT_NEAR(multInv.x, 0.0f, 1e-5f);
	EXPECT_NEAR(multInv.y, 0.0f, 1e-5f);
	EXPECT_NEAR(multInv.z, 0.0f, 1e-5f);
	EXPECT_NEAR(multInv.w, 1.0f, 1e-5f);
}

TEST(QuatTest, VectorRotation)
{
	// В левосторонней системе (Left-Handed: Y-up, Z-forward, X-right):
	// Вращение на 90 градусов вокруг оси Y (Up) по правилу левой руки
	// переводит ось +Z (Forward) в +X (Right).
	Quatf rotY90 = Quatf::FromAxisAngle(Vec3f::Up(), 3.141592653589793f * 0.5f);
	Vec3f forward = Vec3f::Forward(); // (0, 0, 1)

	Vec3f rotated = rotY90.RotateVector(forward);
	EXPECT_NEAR(rotated.x, 1.0f, 1e-5f);
	EXPECT_NEAR(rotated.y, 0.0f, 1e-5f);
	EXPECT_NEAR(rotated.z, 0.0f, 1e-5f);

	// Проверка оператора умножения строки-вектора: v * q
	Vec3f rotatedOp = forward * rotY90;
	EXPECT_NEAR(rotatedOp.x, 1.0f, 1e-5f);
	EXPECT_NEAR(rotatedOp.y, 0.0f, 1e-5f);
	EXPECT_NEAR(rotatedOp.z, 0.0f, 1e-5f);

	// Вращение на 90 градусов вокруг оси X (Right): +Y (Up) -> +Z (Forward)
	Quatf rotX90 = Quatf::FromAxisAngle(Vec3f::Right(), 3.141592653589793f * 0.5f);
	Vec3f up = Vec3f::Up(); // (0, 1, 0)
	Vec3f rotatedUp = up * rotX90;
	EXPECT_NEAR(rotatedUp.x, 0.0f, 1e-5f);
	EXPECT_NEAR(rotatedUp.y, 0.0f, 1e-5f);
	EXPECT_NEAR(rotatedUp.z, 1.0f, 1e-5f);
}

TEST(QuatTest, MatrixConversions)
{
	Quatf q = Quatf::FromEuler(0.2f, 0.5f, 0.3f);
	Mat3f m3 = q.ToMat3();
	Mat4f m4 = q.ToMat4();

	// Проверка эквивалентности поворота вектора через кватернион и через матрицы
	Vec3f v(1.0f, 2.0f, 3.0f);
	Vec3f vFromQuat = v * q;
	Vec3f vFromMat3 = v * m3;
	Vec3f vFromMat4 = m4.TransformVector(v);

	EXPECT_NEAR(vFromQuat.x, vFromMat3.x, 1e-5f);
	EXPECT_NEAR(vFromQuat.y, vFromMat3.y, 1e-5f);
	EXPECT_NEAR(vFromQuat.z, vFromMat3.z, 1e-5f);

	EXPECT_NEAR(vFromQuat.x, vFromMat4.x, 1e-5f);
	EXPECT_NEAR(vFromQuat.y, vFromMat4.y, 1e-5f);
	EXPECT_NEAR(vFromQuat.z, vFromMat4.z, 1e-5f);

	// Обратная конвертация из матрицы в кватернион
	Quatf qFromM3 = Quatf::FromRotationMatrix(m3);
	EXPECT_TRUE(q == qFromM3);
}

TEST(QuatTest, EulerRoundTrip)
{
	const float pitch = 0.35f;
	const float yaw = 0.65f;
	const float roll = -0.25f;

	Quatf q = Quatf::FromEuler(pitch, yaw, roll);
	Vec3f euler = q.ToEuler();

	EXPECT_NEAR(euler.x, pitch, 1e-4f);
	EXPECT_NEAR(euler.y, yaw, 1e-4f);
	EXPECT_NEAR(euler.z, roll, 1e-4f);
}

TEST(QuatTest, SlerpInterpolation)
{
	Quatf q0 = Quatf::Identity();
	Quatf q1 = Quatf::FromAxisAngle(Vec3f::Up(), 3.141592653589793f * 0.5f); // 90 deg

	Quatf slerp0 = Quatf::Slerp(q0, q1, 0.0f);
	EXPECT_EQ(slerp0, q0);

	Quatf slerp1 = Quatf::Slerp(q0, q1, 1.0f);
	EXPECT_EQ(slerp1, q1);

	Quatf slerpHalf = Quatf::Slerp(q0, q1, 0.5f);
	Vec3f rotated = Vec3f::Forward() * slerpHalf;

	// При повороте на 45 градусов вокруг Y (LH): X = sin(45) = sqrt(2)/2, Z = cos(45) = sqrt(2)/2
	const float expected = std::sqrt(2.0f) * 0.5f;
	EXPECT_NEAR(rotated.x, expected, 1e-5f);
	EXPECT_NEAR(rotated.y, 0.0f, 1e-5f);
	EXPECT_NEAR(rotated.z, expected, 1e-5f);
}

TEST(QuatTest, LookRotationAndFromTo)
{
	// LookRotation в сторону +X (Right) с Up=+Y должен повернуть Forward (+Z) в +X
	Quatf lookRight = Quatf::LookRotation(Vec3f::Right(), Vec3f::Up());
	Vec3f rotatedFwd = Vec3f::Forward() * lookRight;
	EXPECT_NEAR(rotatedFwd.x, 1.0f, 1e-5f);
	EXPECT_NEAR(rotatedFwd.y, 0.0f, 1e-5f);
	EXPECT_NEAR(rotatedFwd.z, 0.0f, 1e-5f);

	// FromToRotation от +Z к +X
	Quatf fromTo = Quatf::FromToRotation(Vec3f::Forward(), Vec3f::Right());
	Vec3f res = Vec3f::Forward() * fromTo;
	EXPECT_NEAR(res.x, 1.0f, 1e-5f);
	EXPECT_NEAR(res.y, 0.0f, 1e-5f);
	EXPECT_NEAR(res.z, 0.0f, 1e-5f);
}

TEST(QuatTest, BinarySerialization)
{
	Serializer serializer;
	std::vector<std::byte> buffer;

	Quatf original(0.1f, 0.2f, 0.3f, 0.9f);
	auto serRes = serializer.Serialize(buffer, original);
	EXPECT_TRUE(serRes.has_value());
	EXPECT_EQ(buffer.size(), sizeof(float) * 4);

	Quatf deserialized;
	std::size_t offset = 0;
	auto deserRes = serializer.Deserialize(buffer, offset, deserialized);
	EXPECT_TRUE(deserRes.has_value());
	EXPECT_EQ(offset, buffer.size());

	EXPECT_FLOAT_EQ(deserialized.x, original.x);
	EXPECT_FLOAT_EQ(deserialized.y, original.y);
	EXPECT_FLOAT_EQ(deserialized.z, original.z);
	EXPECT_FLOAT_EQ(deserialized.w, original.w);
}

TEST(QuatTest, FormattingAndToString)
{
	Quatf q(1.0f, 2.0f, 3.0f, 4.0f);
	std::string s = q.ToString();
	EXPECT_EQ(s, "Quat(1, 2, 3, 4)");

	std::string formatted = std::format("Orientation: {}", q);
	EXPECT_EQ(formatted, "Orientation: Quat(1, 2, 3, 4)");
}

#endif // Z_TEST_MATH_QUAT
