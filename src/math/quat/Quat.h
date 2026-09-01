#pragma once

#include "math/vector/Vec3.h"
#include "math/matrix/Mat3.h"
#include "math/matrix/Mat4.h"
#include "math/MathIncludes.h"

namespace zzz::math
{
	/**
	 * @struct Quat
	 * @brief Шаблонная структура кватерниона для описания 3D-вращений без Gimbal Lock.
	 *        Система координат: Левосторонняя (Left-Handed: Y-up, Z-forward, X-right).
	 *        Порядок углов Эйлера: Z -> X -> Y (Roll -> Pitch -> Yaw).
	 *        Соглашение умножения векторов: Row-Vector (v * q, v * M).
	 * 
	 * @tparam T Вещественный тип данных с плавающей запятой (по умолчанию zF32).
	 */
	template<FloatingPoint T = zF32>
	struct alignas(sizeof(T) * 4) Quat
	{
		T x{ static_cast<T>(0) };
		T y{ static_cast<T>(0) };
		T z{ static_cast<T>(0) };
		T w{ static_cast<T>(1) };

		/// @brief Конструктор по умолчанию: создает единичный кватернион (Identity: 0, 0, 0, 1).
		constexpr Quat() noexcept = default;

		/// @brief Покомпонентный конструктор.
		constexpr Quat(T inX, T inY, T inZ, T inW) noexcept
			: x{ inX }, y{ inY }, z{ inZ }, w{ inW }
		{
		}

		/// @brief Конструктор из векторной части (x, y, z) и скалярной (w).
		constexpr Quat(const Vec3<T>& v, T inW) noexcept
			: x{ v.x }, y{ v.y }, z{ v.z }, w{ inW }
		{
		}

		/// @brief Конвертирующий конструктор из кватерниона другой точности.
		template<FloatingPoint U>
		constexpr explicit Quat(const Quat<U>& other) noexcept
			: x{ static_cast<T>(other.x) }
			, y{ static_cast<T>(other.y) }
			, z{ static_cast<T>(other.z) }
			, w{ static_cast<T>(other.w) }
		{
		}

		// --- Доступ к данным (Standard Layout POD) ---

		[[nodiscard]] constexpr const T* data() const noexcept { return &x; }
		[[nodiscard]] constexpr T* data() noexcept { return &x; }

		[[nodiscard]] constexpr const T& operator[](size_t index) const noexcept
		{
			assert(index < 4 && "Quat index out of range");
			return (&x)[index];
		}

		[[nodiscard]] constexpr T& operator[](size_t index) noexcept
		{
			assert(index < 4 && "Quat index out of range");
			return (&x)[index];
		}

		[[nodiscard]] constexpr Vec3<T> GetVectorPart() const noexcept { return Vec3<T>{ x, y, z }; }
		[[nodiscard]] constexpr T GetScalarPart() const noexcept { return w; }

		// --- Статические фабричные методы ---

		[[nodiscard]] static constexpr Quat Identity() noexcept
		{
			return Quat{ static_cast<T>(0), static_cast<T>(0), static_cast<T>(0), static_cast<T>(1) };
		}

		[[nodiscard]] static constexpr Quat Zero() noexcept
		{
			return Quat{ static_cast<T>(0), static_cast<T>(0), static_cast<T>(0), static_cast<T>(0) };
		}

		/**
		 * @brief Создает кватернион вращения вокруг заданной оси на заданный угол (в радианах).
		 * @param axis Ось вращения (будет нормализована).
		 * @param angleRadians Угол вращения в радианах.
		 */
		[[nodiscard]] static Quat FromAxisAngle(const Vec3<T>& axis, T angleRadians) noexcept
		{
			const Vec3<T> normAxis = axis.Normalized();
			const T halfAngle = angleRadians * static_cast<T>(0.5);
			const T sinHalf = std::sin(halfAngle);
			const T cosHalf = std::cos(halfAngle);

			return Quat{
				normAxis.x * sinHalf,
				normAxis.y * sinHalf,
				normAxis.z * sinHalf,
				cosHalf
			};
		}

		/**
		 * @brief Создает кватернион из углов Эйлера в радианах с порядком Z -> X -> Y (Roll -> Pitch -> Yaw).
		 * @param pitchX Вращение вокруг оси X (Pitch) в радианах.
		 * @param yawY Вращение вокруг оси Y (Yaw) в радианах.
		 * @param rollZ Вращение вокруг оси Z (Roll) в радианах.
		 */
		[[nodiscard]] static Quat FromEuler(T pitchX, T yawY, T rollZ) noexcept
		{
			const T halfX = pitchX * static_cast<T>(0.5);
			const T halfY = yawY * static_cast<T>(0.5);
			const T halfZ = rollZ * static_cast<T>(0.5);

			const T cx = std::cos(halfX);
			const T sx = std::sin(halfX);
			const T cy = std::cos(halfY);
			const T sy = std::sin(halfY);
			const T cz = std::cos(halfZ);
			const T sz = std::sin(halfZ);

			// Порядок композиции: Z -> X -> Y (q = qz * qx * qy)
			return Quat{
				cx * sy * sz + sx * cy * cz,
				cx * sy * cz - sx * cy * sz,
				cx * cy * sz - sx * sy * cz,
				cx * cy * cz + sx * sy * sz
			};
		}

		[[nodiscard]] static Quat FromEuler(const Vec3<T>& eulerRadians) noexcept
		{
			return FromEuler(eulerRadians.x, eulerRadians.y, eulerRadians.z);
		}

		/**
		 * @brief Создает кватернион из матрицы ориентации 3x3 (Row-Major).
		 */
		[[nodiscard]] static Quat FromRotationMatrix(const Mat3<T>& m) noexcept
		{
			const T trace = m._11 + m._22 + m._33;
			if (trace > static_cast<T>(0))
			{
				const T s = std::sqrt(trace + static_cast<T>(1)) * static_cast<T>(2); // s = 4 * w
				const T invS = static_cast<T>(1) / s;
				return Quat{
					(m._23 - m._32) * invS,
					(m._31 - m._13) * invS,
					(m._12 - m._21) * invS,
					static_cast<T>(0.25) * s
				}.Normalized();
			}
			else if (m._11 > m._22 && m._11 > m._33)
			{
				const T s = std::sqrt(static_cast<T>(1) + m._11 - m._22 - m._33) * static_cast<T>(2); // s = 4 * x
				const T invS = static_cast<T>(1) / s;
				return Quat{
					static_cast<T>(0.25) * s,
					(m._12 + m._21) * invS,
					(m._31 + m._13) * invS,
					(m._23 - m._32) * invS
				}.Normalized();
			}
			else if (m._22 > m._33)
			{
				const T s = std::sqrt(static_cast<T>(1) + m._22 - m._11 - m._33) * static_cast<T>(2); // s = 4 * y
				const T invS = static_cast<T>(1) / s;
				return Quat{
					(m._12 + m._21) * invS,
					static_cast<T>(0.25) * s,
					(m._23 + m._32) * invS,
					(m._31 - m._13) * invS
				}.Normalized();
			}
			else
			{
				const T s = std::sqrt(static_cast<T>(1) + m._33 - m._11 - m._22) * static_cast<T>(2); // s = 4 * z
				const T invS = static_cast<T>(1) / s;
				return Quat{
					(m._31 + m._13) * invS,
					(m._23 + m._32) * invS,
					static_cast<T>(0.25) * s,
					(m._12 - m._21) * invS
				}.Normalized();
			}
		}

		[[nodiscard]] static Quat FromRotationMatrix(const Mat4<T>& m) noexcept
		{
			return FromRotationMatrix(m.ToMat3());
		}

		/**
		 * @brief Создает вращение кратчайшей дуги от вектора 'from' к вектору 'to'.
		 */
		[[nodiscard]] static Quat FromToRotation(const Vec3<T>& from, const Vec3<T>& to) noexcept
		{
			const Vec3<T> v1 = from.Normalized();
			const Vec3<T> v2 = to.Normalized();

			const T d = v1.Dot(v2);

			if (d >= static_cast<T>(1) - static_cast<T>(1e-6))
			{
				return Identity();
			}

			if (d <= static_cast<T>(-1) + static_cast<T>(1e-6))
			{
				Vec3<T> ortho = Vec3<T>(static_cast<T>(1), static_cast<T>(0), static_cast<T>(0)).Cross(v1);
				if (ortho.LengthSquared() < static_cast<T>(1e-4))
				{
					ortho = Vec3<T>(static_cast<T>(0), static_cast<T>(1), static_cast<T>(0)).Cross(v1);
				}
				return FromAxisAngle(ortho.Normalized(), Constants::PI<T>);
			}

			const Vec3<T> cross = v1.Cross(v2);
			return Quat{ cross.x, cross.y, cross.z, static_cast<T>(1) + d }.Normalized();
		}

		/**
		 * @brief Создает ориентацию взгляда в направлении 'forward' с вектором верха 'up' (Left-Handed: Z-forward, Y-up).
		 */
		[[nodiscard]] static Quat LookRotation(const Vec3<T>& forward, const Vec3<T>& up = Vec3<T>::Up()) noexcept
		{
			const Vec3<T> f = forward.Normalized();
			if (f.LengthSquared() < static_cast<T>(1e-6))
			{
				return Identity();
			}

			Vec3<T> r = up.Cross(f);
			if (r.LengthSquared() < static_cast<T>(1e-6))
			{
				// up и forward коллинеарны
				r = Vec3<T>::Right().Cross(f);
				if (r.LengthSquared() < static_cast<T>(1e-6))
				{
					r = Vec3<T>::Up().Cross(f);
				}
			}
			r.Normalize();
			const Vec3<T> u = f.Cross(r);

			// В row-major матрице базиса: Row 0 = Right, Row 1 = Up, Row 2 = Forward
			const Mat3<T> m(
				r.x, r.y, r.z,
				u.x, u.y, u.z,
				f.x, f.y, f.z
			);

			return FromRotationMatrix(m);
		}

		// --- Базовая алгебра и операции ---

		[[nodiscard]] constexpr Quat operator+(const Quat& other) const noexcept
		{
			return Quat{ x + other.x, y + other.y, z + other.z, w + other.w };
		}

		[[nodiscard]] constexpr Quat operator-(const Quat& other) const noexcept
		{
			return Quat{ x - other.x, y - other.y, z - other.z, w - other.w };
		}

		constexpr Quat& operator+=(const Quat& other) noexcept
		{
			x += other.x;
			y += other.y;
			z += other.z;
			w += other.w;
			return *this;
		}

		constexpr Quat& operator-=(const Quat& other) noexcept
		{
			x -= other.x;
			y -= other.y;
			z -= other.z;
			w -= other.w;
			return *this;
		}

		[[nodiscard]] constexpr Quat operator*(T scalar) const noexcept
		{
			return Quat{ x * scalar, y * scalar, z * scalar, w * scalar };
		}

		constexpr Quat& operator*=(T scalar) noexcept
		{
			x *= scalar;
			y *= scalar;
			z *= scalar;
			w *= scalar;
			return *this;
		}

		[[nodiscard]] constexpr Quat operator/(T scalar) const noexcept
		{
			assert(std::abs(scalar) > static_cast<T>(0) && "Division by zero in Quat");
			const T inv = static_cast<T>(1) / scalar;
			return *this * inv;
		}

		constexpr Quat& operator/=(T scalar) noexcept
		{
			assert(std::abs(scalar) > static_cast<T>(0) && "Division by zero in Quat");
			const T inv = static_cast<T>(1) / scalar;
			return *this *= inv;
		}

		[[nodiscard]] constexpr Quat operator-() const noexcept
		{
			return Quat{ -x, -y, -z, -w };
		}

		/**
		 * @brief Композиция вращений (Гамильтоново произведение кватернионов q1 * q2).
		 * @details Внимание о порядке применения к вектору:
		 *          При вычислении (q1 * q2).RotateVector(v) математически первым применяется правый операнд (rhs / q2),
		 *          а вторым — левый операнд (this / q1), т.е. q1.RotateVector(q2.RotateVector(v)).
		 *          В иерархиях сцены: q_world = q_parent * q_local (локальное вращение применяется раньше родительского).
		 */
		[[nodiscard]] constexpr Quat operator*(const Quat& rhs) const noexcept
		{
			return Quat{
				w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y,
				w * rhs.y - x * rhs.z + y * rhs.w + z * rhs.x,
				w * rhs.z + x * rhs.y - y * rhs.x + z * rhs.w,
				w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z
			};
		}

		constexpr Quat& operator*=(const Quat& rhs) noexcept
		{
			return *this = *this * rhs;
		}

		[[nodiscard]] constexpr bool operator==(const Quat& other) const noexcept
		{
			const T eps = Math::Epsilon<T>();
			// Кватернионы q и -q представляют одно и то же вращение (двойное покрытие)
			const bool direct = Math::Abs(x - other.x) <= eps &&
			                    Math::Abs(y - other.y) <= eps &&
			                    Math::Abs(z - other.z) <= eps &&
			                    Math::Abs(w - other.w) <= eps;

			const bool inverse = Math::Abs(x + other.x) <= eps &&
			                     Math::Abs(y + other.y) <= eps &&
			                     Math::Abs(z + other.z) <= eps &&
			                     Math::Abs(w + other.w) <= eps;

			return direct || inverse;
		}

		[[nodiscard]] constexpr bool operator!=(const Quat& other) const noexcept
		{
			return !(*this == other);
		}

		// --- Геометрические методы ---

		[[nodiscard]] constexpr T Dot(const Quat& other) const noexcept
		{
			return x * other.x + y * other.y + z * other.z + w * other.w;
		}

		[[nodiscard]] constexpr T LengthSquared() const noexcept
		{
			return Dot(*this);
		}

		[[nodiscard]] T Length() const noexcept
		{
			return std::sqrt(LengthSquared());
		}

		Quat& Normalize() noexcept
		{
			const T len = Length();
			if (len > static_cast<T>(1e-6))
			{
				const T inv = static_cast<T>(1) / len;
				x *= inv;
				y *= inv;
				z *= inv;
				w *= inv;
			}
			else
			{
				*this = Identity();
			}
			return *this;
		}

		[[nodiscard]] Quat Normalized() const noexcept
		{
			Quat copy = *this;
			return copy.Normalize();
		}

		[[nodiscard]] constexpr bool IsNormalized(T tolerance = Math::Epsilon<T>()) const noexcept
		{
			return Math::Abs(LengthSquared() - static_cast<T>(1)) <= tolerance;
		}

		[[nodiscard]] constexpr Quat Conjugate() const noexcept
		{
			return Quat{ -x, -y, -z, w };
		}

		[[nodiscard]] Quat Inverse() const noexcept
		{
			const T lenSq = LengthSquared();
			if (lenSq > static_cast<T>(1e-6))
			{
				return Conjugate() / lenSq;
			}
			return Identity();
		}

		// --- Вращение векторов (Row-Vector v * q) ---

		/**
		 * @brief Поворачивает 3D-вектор v кватернионом.
		 *        Использует оптимизированную формулу: v' = v + 2w * (q_v x v) + 2 * (q_v x (q_v x v)).
		 */
		[[nodiscard]] Vec3<T> RotateVector(const Vec3<T>& v) const noexcept
		{
			const Vec3<T> qv{ x, y, z };
			const Vec3<T> t = static_cast<T>(2) * qv.Cross(v);
			return v + (w * t) + qv.Cross(t);
		}

		// --- Интерполяции (Lerp, Slerp) ---

		/**
		 * @brief Линейная интерполяция с нормализацией (NLerp).
		 */
		[[nodiscard]] static Quat Lerp(const Quat& a, const Quat& b, T t) noexcept
		{
			Quat bTarget = b;
			if (a.Dot(b) < static_cast<T>(0))
			{
				bTarget = -b;
			}

			const T oneMinusT = static_cast<T>(1) - t;
			return Quat{
				a.x * oneMinusT + bTarget.x * t,
				a.y * oneMinusT + bTarget.y * t,
				a.z * oneMinusT + bTarget.z * t,
				a.w * oneMinusT + bTarget.w * t
			}.Normalized();
		}

		/**
		 * @brief Сферическая линейная интерполяция (Slerp) с выбором кратчайшей дуги.
		 */
		[[nodiscard]] static Quat Slerp(const Quat& a, const Quat& b, T t) noexcept
		{
			T cosHalfTheta = a.Dot(b);
			Quat bTarget = b;

			if (cosHalfTheta < static_cast<T>(0))
			{
				bTarget = -b;
				cosHalfTheta = -cosHalfTheta;
			}

			if (cosHalfTheta >= static_cast<T>(0.9995))
			{
				// Если кватернионы почти совпадают, линейная интерполяция точнее и быстрее
				return Lerp(a, bTarget, t);
			}

			const T halfTheta = std::acos(cosHalfTheta);
			const T sinHalfTheta = std::sqrt(static_cast<T>(1) - cosHalfTheta * cosHalfTheta);

			const T ratioA = std::sin((static_cast<T>(1) - t) * halfTheta) / sinHalfTheta;
			const T ratioB = std::sin(t * halfTheta) / sinHalfTheta;

			return (a * ratioA + bTarget * ratioB).Normalized();
		}

		// --- Конвертация в матрицы и углы Эйлера ---

		/**
		 * @brief Преобразует кватернион в матрицу вращения 3x3 для Row-Vector конвенции (v * M).
		 */
		[[nodiscard]] Mat3<T> ToMat3() const noexcept
		{
			const T xx = x * x;
			const T yy = y * y;
			const T zz = z * z;
			const T xy = x * y;
			const T xz = x * z;
			const T yz = y * z;
			const T wx = w * x;
			const T wy = w * y;
			const T wz = w * z;

			// Row-Vector матрица (транспонированная относительно column-vector)
			return Mat3<T>(
				static_cast<T>(1) - static_cast<T>(2) * (yy + zz),
				static_cast<T>(2) * (xy + wz),
				static_cast<T>(2) * (xz - wy),

				static_cast<T>(2) * (xy - wz),
				static_cast<T>(1) - static_cast<T>(2) * (xx + zz),
				static_cast<T>(2) * (yz + wx),

				static_cast<T>(2) * (xz + wy),
				static_cast<T>(2) * (yz - wx),
				static_cast<T>(1) - static_cast<T>(2) * (xx + yy)
			);
		}

		/**
		 * @brief Преобразует кватернион в аффинную матрицу вращения 4x4 (Row-Vector v * M).
		 */
		[[nodiscard]] Mat4<T> ToMat4() const noexcept
		{
			return Mat4<T>(ToMat3());
		}

		/**
		 * @brief Извлекает углы Эйлера в радианах с порядком Z -> X -> Y (Roll -> Pitch -> Yaw).
		 * @return Vec3<T>(pitchX, yawY, rollZ) в радианах.
		 */
		[[nodiscard]] Vec3<T> ToEuler() const noexcept
		{
			const Mat3<T> m = ToMat3();
			Vec3<T> euler{};

			// В Row-Major матрице R = Rz * Rx * Ry:
			// m._32 = 2 * (yz - wx) = -sin(pitchX)
			const T sinPitch = -m._32;

			if (sinPitch <= static_cast<T>(-1) + static_cast<T>(1e-5))
			{
				// Pitch = -PI / 2 (Gimbal Lock)
				euler.x = -Constants::HalfPI<T>;
				euler.y = std::atan2(-m._13, m._11);
				euler.z = static_cast<T>(0);
			}
			else if (sinPitch >= static_cast<T>(1) - static_cast<T>(1e-5))
			{
				// Pitch = +PI / 2 (Gimbal Lock)
				euler.x = Constants::HalfPI<T>;
				euler.y = std::atan2(m._13, m._11);
				euler.z = static_cast<T>(0);
			}
			else
			{
				euler.x = std::asin(std::clamp(sinPitch, static_cast<T>(-1), static_cast<T>(1)));
				euler.y = std::atan2(m._31, m._33);
				euler.z = std::atan2(m._12, m._22);
			}

			return euler;
		}

		/**
		 * @brief Извлекает ось и угол вращения (в радианах).
		 */
		void ToAxisAngle(Vec3<T>& outAxis, T& outAngleRadians) const noexcept
		{
			const Quat q = (w > static_cast<T>(1) || w < static_cast<T>(-1)) ? Normalized() : *this;
			outAngleRadians = static_cast<T>(2) * std::acos(std::clamp(q.w, static_cast<T>(-1), static_cast<T>(1)));
			const T s = std::sqrt(static_cast<T>(1) - q.w * q.w);

			if (s < static_cast<T>(1e-5))
			{
				outAxis = Vec3<T>(static_cast<T>(1), static_cast<T>(0), static_cast<T>(0));
			}
			else
			{
				outAxis = Vec3<T>(q.x / s, q.y / s, q.z / s);
			}
		}

		[[nodiscard]] std::string ToString() const
		{
			return std::format("Quat({}, {}, {}, {})", x, y, z, w);
		}
	};

	// --- Внешние операторы ---

	template<FloatingPoint T>
	[[nodiscard]] constexpr Quat<T> operator*(T scalar, const Quat<T>& q) noexcept
	{
		return q * scalar;
	}

	template<FloatingPoint T>
	[[nodiscard]] constexpr Vec3<T> operator*(const Vec3<T>& v, const Quat<T>& q) noexcept
	{
		return q.RotateVector(v);
	}

	template<FloatingPoint T>
	[[nodiscard]] constexpr T Dot(const Quat<T>& a, const Quat<T>& b) noexcept
	{
		return a.Dot(b);
	}

	template<FloatingPoint T>
	[[nodiscard]] inline Quat<T> Slerp(const Quat<T>& a, const Quat<T>& b, T t) noexcept
	{
		return Quat<T>::Slerp(a, b, t);
	}

	template<FloatingPoint T>
	[[nodiscard]] inline Quat<T> Lerp(const Quat<T>& a, const Quat<T>& b, T t) noexcept
	{
		return Quat<T>::Lerp(a, b, t);
	}

	// --- Псевдонимы типов (Typedefs) ---
	using Quatf = Quat<zF32>;
	using Quatd = Quat<zF64>;

	// Гарантии размера, выравнивания и раскладки памяти (Standard Layout POD)
	static_assert(sizeof(Quatf) == 16, "Quatf must be exactly 16 bytes");
	static_assert(alignof(Quatf) == 16, "Quatf must be 16-byte aligned");
	static_assert(sizeof(Quatd) == 32, "Quatd must be exactly 32 bytes");
	static_assert(alignof(Quatd) == 32, "Quatd must be 32-byte aligned");
	static_assert(std::is_standard_layout_v<Quatf>, "Quatf must be standard layout");
	static_assert(std::is_standard_layout_v<Quatd>, "Quatd must be standard layout");
}

// Специализация std::formatter
template<zzz::math::FloatingPoint T>
struct std::formatter<zzz::math::Quat<T>> : std::formatter<std::string>
{
	auto format(const zzz::math::Quat<T>& q, std::format_context& ctx) const
	{
		return std::formatter<std::string>::format(q.ToString(), ctx);
	}
};
