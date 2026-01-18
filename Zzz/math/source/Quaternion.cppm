
export module Quaternion;

#include "../headers/HeaderConstants.h"

import Vector;
import Matrix4x4;

namespace zzz::math
{
	export class Quaternion
	{
	public:
		float x, y, z, w;

	public:
		constexpr Quaternion() noexcept : x(0), y(0), z(0), w(1) {}
		constexpr explicit Quaternion(float v) noexcept : x(v), y(v), z(v), w(v) {}
		constexpr Quaternion(float x_, float y_, float z_, float w_) noexcept : x(x_), y(y_), z(z_), w(w_) {}

		static Quaternion identity() noexcept;
		static Quaternion fromAxisAngle(const Vector3& axis, float radians) noexcept;
		static Quaternion fromEulerXYZ(float pitch, float yaw, float roll) noexcept;
		static Quaternion fromLookRotation(const Vector3& forward, const Vector3& up = Vector3{ 0, 1, 0 }) noexcept;
		static Quaternion fromToRotation(const Vector3& from, const Vector3& to) noexcept;

		float length() const noexcept;
		Quaternion normalized() const noexcept;
		void normalize() noexcept;

		Quaternion conjugate() const noexcept;
		Quaternion inverse() const noexcept;

		// Операторы сравнения
		bool operator==(const Quaternion& rhs) const noexcept;
		bool operator!=(const Quaternion& rhs) const noexcept;

		// Арифметические операторы
		Quaternion operator+(const Quaternion& rhs) const noexcept;
		Quaternion operator-(const Quaternion& rhs) const noexcept;
		Quaternion operator-() const noexcept;
		Quaternion operator*(const Quaternion& rhs) const noexcept;
		Quaternion operator*(float scalar) const noexcept;
		Quaternion operator/(float scalar) const noexcept;
		Vector3 operator*(const Vector3& v) const noexcept;

		// Операторы присваивания
		Quaternion& operator+=(const Quaternion& rhs) noexcept;
		Quaternion& operator-=(const Quaternion& rhs) noexcept;
		Quaternion& operator*=(const Quaternion& rhs) noexcept;
		Quaternion& operator*=(float scalar) noexcept;
		Quaternion& operator/=(float scalar) noexcept;

		Matrix4x4 toMatrix4() const noexcept;
		Vector3 toEulerAngles() const noexcept;

		// Получение осей и углов
		Vector3 getAxis() const noexcept;
		float getAngle() const noexcept;
		Vector3 getForward() const noexcept;
		Vector3 getUp() const noexcept;
		Vector3 getRight() const noexcept;

		// Вращения вокруг осей
		static Quaternion rotateX(float radians) noexcept;
		static Quaternion rotateY(float radians) noexcept;
		static Quaternion rotateZ(float radians) noexcept;

		// Интерполяция
		static Quaternion slerp(const Quaternion& a, const Quaternion& b, float t) noexcept;
		static Quaternion nlerp(const Quaternion& a, const Quaternion& b, float t) noexcept;

		// Exp/Log
		static Quaternion exp(const Vector3& v) noexcept;
		Vector3 log() const noexcept;

		// Утилиты
		static Quaternion random() noexcept;
		bool isValid() const noexcept;

		float dot(const Quaternion& rhs) const noexcept { return x * rhs.x + y * rhs.y + z * rhs.z + w * rhs.w; }
		float lengthSq() const noexcept { return x * x + y * y + z * z + w * w; }

		bool equals(const Quaternion& rhs, float eps = 1e-6f) const noexcept
		{
			return
				std::abs(x - rhs.x) < eps &&
				std::abs(y - rhs.y) < eps &&
				std::abs(z - rhs.z) < eps &&
				std::abs(w - rhs.w) < eps;
		}

		bool isNormalized(float eps = 1e-5f) const noexcept
		{
			return std::abs(lengthSq() - 1.0f) < eps;
		}

		static float angleBetween(const Quaternion& a, const Quaternion& b) noexcept
		{
			float d = std::abs(a.dot(b));
			d = std::clamp(d, -1.0f, 1.0f);
			return 2.0f * std::acos(d);
		}

		static Quaternion fromYawPitchRoll(float yaw, float pitch, float roll) noexcept
		{
			float cy = std::cos(yaw * 0.5f);
			float sy = std::sin(yaw * 0.5f);
			float cp = std::cos(pitch * 0.5f);
			float sp = std::sin(pitch * 0.5f);
			float cr = std::cos(roll * 0.5f);
			float sr = std::sin(roll * 0.5f);

			return Quaternion{
				sr * cp * cy - cr * sp * sy,  // x
				cr * sp * cy + sr * cp * sy,  // y
				cr * cp * sy - sr * sp * cy,  // z
				cr * cp * cy + sr * sp * sy   // w
			};
		}
	};

	// Внешний оператор
	export inline Quaternion operator*(float scalar, const Quaternion& q) noexcept
	{
		return q * scalar;
	}

	// ============ РЕАЛИЗАЦИИ ============

	inline Quaternion Quaternion::identity() noexcept { return Quaternion{}; }

	inline float Quaternion::length() const noexcept { return std::sqrt(x * x + y * y + z * z + w * w); }

	inline Quaternion Quaternion::normalized() const noexcept
	{
		float len = length();
		if (len < 1e-8f)
			return Quaternion{};
		float inv = 1.0f / len;
		return { x * inv, y * inv, z * inv, w * inv };
	}

	inline void Quaternion::normalize() noexcept { *this = normalized(); }

	inline Quaternion Quaternion::fromAxisAngle(const Vector3& axis, float radians) noexcept
	{
		Vector3 n = axis.normalized();
		float half = radians * 0.5f;
		float s = std::sin(half);

		return {
			n.x() * s,
			n.y() * s,
			n.z() * s,
			std::cos(half)
		};
	}

	// Операторы сравнения
	inline bool Quaternion::operator==(const Quaternion& rhs) const noexcept
	{
		return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
	}

	inline bool Quaternion::operator!=(const Quaternion& rhs) const noexcept
	{
		return !(*this == rhs);
	}

	// Арифметические операторы
	inline Quaternion Quaternion::operator+(const Quaternion& rhs) const noexcept
	{
		return { x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w };
	}

	inline Quaternion Quaternion::operator-(const Quaternion& rhs) const noexcept
	{
		return { x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w };
	}

	inline Quaternion Quaternion::operator-() const noexcept
	{
		return { -x, -y, -z, -w };
	}

	inline Quaternion Quaternion::operator*(const Quaternion& r) const noexcept
	{
		return {
			w * r.x + x * r.w + y * r.z - z * r.y,
			w * r.y - x * r.z + y * r.w + z * r.x,
			w * r.z + x * r.y - y * r.x + z * r.w,
			w * r.w - x * r.x - y * r.y - z * r.z
		};
	}

	inline Quaternion Quaternion::operator*(float scalar) const noexcept
	{
		return { x * scalar, y * scalar, z * scalar, w * scalar };
	}

	inline Quaternion Quaternion::operator/(float scalar) const noexcept
	{
		float inv = 1.0f / scalar;
		return { x * inv, y * inv, z * inv, w * inv };
	}

	inline Vector3 Quaternion::operator*(const Vector3& v) const noexcept
	{
		Vector3 qv{ x, y, z };
		Vector3 t = 2.0f * qv.cross(v);
		return v + w * t + qv.cross(t);
	}

	// Операторы присваивания
	inline Quaternion& Quaternion::operator+=(const Quaternion& rhs) noexcept
	{
		x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w;
		return *this;
	}

	inline Quaternion& Quaternion::operator-=(const Quaternion& rhs) noexcept
	{
		x -= rhs.x; y -= rhs.y; z -= rhs.z; w -= rhs.w;
		return *this;
	}

	inline Quaternion& Quaternion::operator*=(const Quaternion& rhs) noexcept
	{
		*this = *this * rhs;
		return *this;
	}

	inline Quaternion& Quaternion::operator*=(float scalar) noexcept
	{
		x *= scalar; y *= scalar; z *= scalar; w *= scalar;
		return *this;
	}

	inline Quaternion& Quaternion::operator/=(float scalar) noexcept
	{
		float inv = 1.0f / scalar;
		x *= inv; y *= inv; z *= inv; w *= inv;
		return *this;
	}

	inline Quaternion Quaternion::conjugate() const noexcept
	{
		return { -x, -y, -z, w };
	}

	inline Quaternion Quaternion::inverse() const noexcept
	{
		float len2 = x * x + y * y + z * z + w * w;
		if (len2 < 1e-8f)
			return Quaternion{};
		float inv = 1.0f / len2;
		return { -x * inv, -y * inv, -z * inv, w * inv };
	}

	inline Quaternion Quaternion::fromLookRotation(const Vector3& forward, const Vector3& up) noexcept
	{
		Vector3 f = forward.normalized();
		Vector3 r = up.cross(f).normalized();
		Vector3 u = f.cross(r);

		float m00 = r.x(), m01 = u.x(), m02 = f.x();
		float m10 = r.y(), m11 = u.y(), m12 = f.y();
		float m20 = r.z(), m21 = u.z(), m22 = f.z();

		float trace = m00 + m11 + m22;

		Quaternion q;
		if (trace > 0.0f)
		{
			float s = std::sqrt(trace + 1.0f) * 2.0f;
			q.w = 0.25f * s;
			q.x = (m21 - m12) / s;
			q.y = (m02 - m20) / s;
			q.z = (m10 - m01) / s;
		}
		else if (m00 > m11 && m00 > m22)
		{
			float s = std::sqrt(1.0f + m00 - m11 - m22) * 2.0f;
			q.w = (m21 - m12) / s;
			q.x = 0.25f * s;
			q.y = (m01 + m10) / s;
			q.z = (m02 + m20) / s;
		}
		else if (m11 > m22)
		{
			float s = std::sqrt(1.0f + m11 - m00 - m22) * 2.0f;
			q.w = (m02 - m20) / s;
			q.x = (m01 + m10) / s;
			q.y = 0.25f * s;
			q.z = (m12 + m21) / s;
		}
		else
		{
			float s = std::sqrt(1.0f + m22 - m00 - m11) * 2.0f;
			q.w = (m10 - m01) / s;
			q.x = (m02 + m20) / s;
			q.y = (m12 + m21) / s;
			q.z = 0.25f * s;
		}

		return q.normalized();
	}

	inline Quaternion Quaternion::fromToRotation(const Vector3& from, const Vector3& to) noexcept
	{
		Vector3 f = from.normalized();
		Vector3 t = to.normalized();

		float d = f.dot(t);

		if (d >= 1.0f - 1e-6f)
			return Quaternion::identity();

		if (d <= -1.0f + 1e-6f)
		{
			Vector3 axis = Vector3{ 1, 0, 0 }.cross(f);
			if (axis.lengthSq() < 1e-6f)
				axis = Vector3{ 0, 1, 0 }.cross(f);
			return fromAxisAngle(axis.normalized(), 3.14159265359f);
		}

		Vector3 axis = f.cross(t);
		float s = std::sqrt((1.0f + d) * 2.0f);
		float invs = 1.0f / s;

		return Quaternion{
			axis.x() * invs,
			axis.y() * invs,
			axis.z() * invs,
			s * 0.5f
		}.normalized();
	}

	inline Quaternion Quaternion::fromEulerXYZ(float pitch, float yaw, float roll) noexcept
	{
		float cx = std::cos(pitch * 0.5f);
		float sx = std::sin(pitch * 0.5f);
		float cy = std::cos(yaw * 0.5f);
		float sy = std::sin(yaw * 0.5f);
		float cz = std::cos(roll * 0.5f);
		float sz = std::sin(roll * 0.5f);

		// q = qz * qy * qx
		return {
			cz * cy * sx + sz * sy * cx,   // x
			cz * sy * cx - sz * cy * sx,   // y
			sz * cy * cx + cz * sy * sx,   // z
			cz * cy * cx - sz * sy * sx    // w
		};
	}

	Vector3 Quaternion::toEulerAngles() const noexcept
	{
		Quaternion q = normalized();

		// Проверка на gimbal lock
		float sinp = 2.0f * (q.w * q.y - q.z * q.x);
		sinp = std::clamp(sinp, -1.0f, 1.0f);

		Vector3 angles;
		if (std::abs(sinp) >= 1.0f)
		{
			// Gimbal lock: pitch = ±90°
			angles.set_y(std::copysign(PI_2, sinp)); // pitch
			angles.set_z(0.0f);                      // roll = 0
			angles.set_x(std::atan2(
				2.0f * (q.x * q.y + q.w * q.z),
				1.0f - 2.0f * (q.y * q.y + q.z * q.z)
			)); // yaw
		}
		else
		{
			angles.set_y(std::asin(sinp)); // pitch
			angles.set_x(std::atan2(
				2.0f * (q.w * q.z + q.x * q.y),
				1.0f - 2.0f * (q.y * q.y + q.z * q.z)
			)); // yaw
			angles.set_z(std::atan2(
				2.0f * (q.w * q.x + q.y * q.z),
				1.0f - 2.0f * (q.x * q.x + q.z * q.z)
			)); // roll
		}

		// Возвращаем в порядке: { yaw, pitch, roll }
		return angles;
	}

	inline Matrix4x4 Quaternion::toMatrix4() const noexcept
	{
		Quaternion q = normalized();

		float xx = q.x * q.x;
		float yy = q.y * q.y;
		float zz = q.z * q.z;

		float xy = q.x * q.y;
		float xz = q.x * q.z;
		float yz = q.y * q.z;

		float wx = q.w * q.x;
		float wy = q.w * q.y;
		float wz = q.w * q.z;

		return Matrix4x4(
			1.0f - 2.0f * (yy + zz), 2.0f * (xy + wz), 2.0f * (xz - wy), 0.0f,
			2.0f * (xy - wz), 1.0f - 2.0f * (xx + zz), 2.0f * (yz + wx), 0.0f,
			2.0f * (xz + wy), 2.0f * (yz - wx), 1.0f - 2.0f * (xx + yy), 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		);
	}

	inline Vector3 Quaternion::getAxis() const noexcept
	{
		float s = std::sqrt(1.0f - w * w);
		if (s < 1e-6f)
			return Vector3{ 1, 0, 0 };
		return Vector3{ x / s, y / s, z / s };
	}

	inline float Quaternion::getAngle() const noexcept
	{
		return 2.0f * std::acos(std::clamp(w, -1.0f, 1.0f));
	}

	inline Vector3 Quaternion::getForward() const noexcept
	{
		return *this * Vector3{ 0, 0, 1 };
	}

	inline Vector3 Quaternion::getUp() const noexcept
	{
		return *this * Vector3{ 0, 1, 0 };
	}

	inline Vector3 Quaternion::getRight() const noexcept
	{
		return *this * Vector3{ 1, 0, 0 };
	}

	inline Quaternion Quaternion::rotateX(float radians) noexcept
	{
		float half = radians * 0.5f;
		return { std::sin(half), 0, 0, std::cos(half) };
	}

	inline Quaternion Quaternion::rotateY(float radians) noexcept
	{
		float half = radians * 0.5f;
		return { 0, std::sin(half), 0, std::cos(half) };
	}

	inline Quaternion Quaternion::rotateZ(float radians) noexcept
	{
		float half = radians * 0.5f;
		return { 0, 0, std::sin(half), std::cos(half) };
	}

	inline Quaternion Quaternion::slerp(const Quaternion& a, const Quaternion& b, float t) noexcept
	{
		float cosTheta = a.dot(b);

		Quaternion end = b;

		if (cosTheta < 0.0f)
		{
			cosTheta = -cosTheta;
			end = { -b.x, -b.y, -b.z, -b.w };
		}

		// Линейная интерполяция для почти параллельных кватернионов
		if (cosTheta > 0.9995f)
		{
			return Quaternion{
				a.x + (end.x - a.x) * t,
				a.y + (end.y - a.y) * t,
				a.z + (end.z - a.z) * t,
				a.w + (end.w - a.w) * t
			}.normalized();
		}

		float angle = std::acos(cosTheta);
		float sinAngle = std::sin(angle);

		float w1 = std::sin((1.0f - t) * angle) / sinAngle;
		float w2 = std::sin(t * angle) / sinAngle;

		return {
			a.x * w1 + end.x * w2,
			a.y * w1 + end.y * w2,
			a.z * w1 + end.z * w2,
			a.w * w1 + end.w * w2
		};
	}

	inline Quaternion Quaternion::nlerp(const Quaternion& a, const Quaternion& b, float t) noexcept
	{
		float cosTheta = a.dot(b);
		Quaternion end = (cosTheta < 0.0f) ? -b : b;

		return Quaternion{
			a.x + (end.x - a.x) * t,
			a.y + (end.y - a.y) * t,
			a.z + (end.z - a.z) * t,
			a.w + (end.w - a.w) * t
		}.normalized();
	}

	inline Quaternion Quaternion::exp(const Vector3& v) noexcept
	{
		float theta = v.length();
		if (theta < 1e-6f)
			return Quaternion::identity();

		float s = std::sin(theta) / theta;
		return { v.x() * s, v.y() * s, v.z() * s, std::cos(theta) };
	}

	inline Vector3 Quaternion::log() const noexcept
	{
		float len = std::sqrt(x * x + y * y + z * z);
		if (len < 1e-6f)
			return Vector3{ 0, 0, 0 };

		float theta = std::atan2(len, w);
		float s = theta / len;
		return Vector3{ x * s, y * s, z * s };
	}

	inline Quaternion Quaternion::random() noexcept
	{
		float u1 = static_cast<float>(rand()) / RAND_MAX;
		float u2 = static_cast<float>(rand()) / RAND_MAX;
		float u3 = static_cast<float>(rand()) / RAND_MAX;

		float sqrt1u1 = std::sqrt(1.0f - u1);
		float sqrtu1 = std::sqrt(u1);

		return Quaternion{
			sqrt1u1 * std::sin(2.0f * 3.14159265359f * u2),
			sqrt1u1 * std::cos(2.0f * 3.14159265359f * u2),
			sqrtu1 * std::sin(2.0f * 3.14159265359f * u3),
			sqrtu1 * std::cos(2.0f * 3.14159265359f * u3)
		};
	}

	inline bool Quaternion::isValid() const noexcept
	{
		return std::isfinite(x) && std::isfinite(y) &&
			std::isfinite(z) && std::isfinite(w);
	}
}