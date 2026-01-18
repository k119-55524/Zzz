#include "../headers/headerSIMD.h"

export module Vector4;

export namespace zzz::math
{
	struct alignas(16) Vector4
	{
		simd_float4 data;

		// ---------------- Constructors ----------------
		Vector4() noexcept
		{
#if defined(__APPLE__)
			data = ::simd::float4{ 0,0,0,0 };
#elif defined(_M_X64) || defined(__x86_64__)
			data = _mm_setzero_ps();
#elif defined(_M_ARM64) || defined(__aarch64__)
			data = vdupq_n_f32(0.0f);
#endif
		}

		Vector4(float v) noexcept
		{
#if defined(__APPLE__)
			data = ::simd::float4{ v,v,v,v };
#elif defined(_M_X64) || defined(__x86_64__)
			data = _mm_set1_ps(v);
#elif defined(_M_ARM64) || defined(__aarch64__)
			data = vdupq_n_f32(v);
#endif
		}

		Vector4(float x, float y, float z, float w) noexcept
		{
#if defined(__APPLE__)
			data = ::simd::float4{ x,y,z,w };
#elif defined(_M_X64) || defined(__x86_64__)
			data = _mm_set_ps(w, z, y, x);
#elif defined(_M_ARM64) || defined(__aarch64__)
			float values[4] = { x, y, z, w };
			data = vld1q_f32(values);
#endif
		}

		explicit Vector4(simd_float4 simd_data) noexcept : data(simd_data) {}

#pragma region Getters
		inline float x() const noexcept
		{
#if defined(__APPLE__)
			return data.x;
#else
			return reinterpret_cast<const float*>(&data)[0];
#endif
		}
		inline float y() const noexcept
		{
#if defined(__APPLE__)
			return data.y;
#else
			return reinterpret_cast<const float*>(&data)[1];
#endif

		}
		inline float z() const noexcept
		{
#if defined(__APPLE__)
			return data.z;
#else
			return reinterpret_cast<const float*>(&data)[2];
#endif
		}
		inline float w() const noexcept
		{
#if defined(__APPLE__)
			return data.w;
#else
			return reinterpret_cast<const float*>(&data)[3];
#endif
		}

		// RGBA aliases
		inline float r() const noexcept { return x(); }
		inline float g() const noexcept { return y(); }
		inline float b() const noexcept { return z(); }
		inline float a() const noexcept { return w(); }
#pragma endregion Getters

#pragma region Setters
		inline void set_x(float value) noexcept
		{
#if defined(__APPLE__)
			data.x = value;
#else
			reinterpret_cast<float*>(&data)[0] = value;
#endif
		}
		inline void set_y(float value) noexcept
		{
#if defined(__APPLE__)
			data.y = value;
#else
			reinterpret_cast<float*>(&data)[1] = value;
#endif
		}
		inline void set_z(float value) noexcept
		{
#if defined(__APPLE__)
			data.z = value;
#else
			reinterpret_cast<float*>(&data)[2] = value;
#endif
		}
		inline void set_w(float value) noexcept
		{
#if defined(__APPLE__)
			data.w = value;
#else
			reinterpret_cast<float*>(&data)[3] = value;
#endif
		}

		// RGBA setters
		inline void set_r(float value) noexcept { set_x(value); }
		inline void set_g(float value) noexcept { set_y(value); }
		inline void set_b(float value) noexcept { set_z(value); }
		inline void set_a(float value) noexcept { set_w(value); }
#pragma endregion Setters

		// Доступ к компонентам 
		inline float& operator[](size_t i) noexcept { return reinterpret_cast<float*>(&data)[i]; }
		inline const float& operator[](size_t i) const noexcept { return reinterpret_cast<const float*>(&data)[i]; }

		// Арифметика 
		Vector4 operator+(const Vector4& rhs) const noexcept
		{
			Vector4 Result;
#if defined(__APPLE__)
			Result.data = data + rhs.data;
#elif defined(_M_X64) || defined(__x86_64__)
			Result.data = _mm_add_ps(data, rhs.data);
#elif defined(_M_ARM64) || defined(__aarch64__)
			Result.data = vaddq_f32(data, rhs.data);
#endif
			return Result;
		}

		Vector4 operator-(const Vector4& rhs) const noexcept
		{
			Vector4 Result;
#if defined(__APPLE__)
			Result.data = data - rhs.data;
#elif defined(_M_X64) || defined(__x86_64__)
			Result.data = _mm_sub_ps(data, rhs.data);
#elif defined(_M_ARM64) || defined(__aarch64__)
			Result.data = vsubq_f32(data, rhs.data);
#endif
			return Result;
		}

		Vector4 operator*(float s) const noexcept
		{
			Vector4 Result;
#if defined(__APPLE__)
			Result.data = data * s;
#elif defined(_M_X64) || defined(__x86_64__)
			__m128 scalar = _mm_set1_ps(s);
			Result.data = _mm_mul_ps(data, scalar);
#elif defined(_M_ARM64) || defined(__aarch64__)
			float32x4_t scalar = vdupq_n_f32(s);
			Result.data = vmulq_f32(data, scalar);
#endif
			return Result;
		}

		Vector4 operator/(float s) const noexcept
		{
			Vector4 Result;
#if defined(__APPLE__)
			Result.data = data / s;
#elif defined(_M_X64) || defined(__x86_64__)
			__m128 scalar = _mm_set1_ps(s);
			Result.data = _mm_div_ps(data, scalar);
#elif defined(_M_ARM64) || defined(__aarch64__)
			// Используем обратное умножение для совместимости со старыми версиями NEON
			float32x4_t inv_scalar = vdupq_n_f32(1.0f / s);
			Result.data = vmulq_f32(data, inv_scalar);
#endif
			return Result;
		}

		// Покомпонентное умножение
		Vector4 operator*(const Vector4& rhs) const noexcept
		{
			Vector4 Result;
#if defined(__APPLE__)
			Result.data = data * rhs.data;
#elif defined(_M_X64) || defined(__x86_64__)
			Result.data = _mm_mul_ps(data, rhs.data);
#elif defined(_M_ARM64) || defined(__aarch64__)
			Result.data = vmulq_f32(data, rhs.data);
#endif
			return Result;
		}

		// Покомпонентное деление
		Vector4 operator/(const Vector4& rhs) const noexcept
		{
			Vector4 Result;
#if defined(__APPLE__)
			Result.data = data / rhs.data;
#elif defined(_M_X64) || defined(__x86_64__)
			Result.data = _mm_div_ps(data, rhs.data);
#elif defined(_M_ARM64) || defined(__aarch64__)
			// Обратное умножение для каждого компонента
			float32x4_t inv = vrecpeq_f32(rhs.data); // Приближённая обратная величина
			inv = vmulq_f32(vrecpsq_f32(rhs.data, inv), inv); // Уточнение Newton-Raphson
			Result.data = vmulq_f32(data, inv);
#endif
			return Result;
		}

		// Оптимизированные операции присваивания
		Vector4& operator+=(const Vector4& rhs) noexcept
		{
#if defined(__APPLE__)
			data += rhs.data;
#elif defined(_M_X64) || defined(__x86_64__)
			data = _mm_add_ps(data, rhs.data);
#elif defined(_M_ARM64) || defined(__aarch64__)
			data = vaddq_f32(data, rhs.data);
#endif
			return *this;
		}

		Vector4& operator-=(const Vector4& rhs) noexcept
		{
#if defined(__APPLE__)
			data -= rhs.data;
#elif defined(_M_X64) || defined(__x86_64__)
			data = _mm_sub_ps(data, rhs.data);
#elif defined(_M_ARM64) || defined(__aarch64__)
			data = vsubq_f32(data, rhs.data);
#endif
			return *this;
		}

		Vector4& operator*=(float s) noexcept
		{
#if defined(__APPLE__)
			data *= s;
#elif defined(_M_X64) || defined(__x86_64__)
			__m128 scalar = _mm_set1_ps(s);
			data = _mm_mul_ps(data, scalar);
#elif defined(_M_ARM64) || defined(__aarch64__)
			float32x4_t scalar = vdupq_n_f32(s);
			data = vmulq_f32(data, scalar);
#endif
			return *this;
		}

		Vector4& operator*=(const Vector4& rhs) noexcept
		{
#if defined(__APPLE__)
			data *= rhs.data;
#elif defined(_M_X64) || defined(__x86_64__)
			data = _mm_mul_ps(data, rhs.data);
#elif defined(_M_ARM64) || defined(__aarch64__)
			data = vmulq_f32(data, rhs.data);
#endif
			return *this;
		}

		Vector4& operator/=(float s) noexcept
		{
#if defined(__APPLE__)
			data /= s;
#elif defined(_M_X64) || defined(__x86_64__)
			__m128 scalar = _mm_set1_ps(s);
			data = _mm_div_ps(data, scalar);
#elif defined(_M_ARM64) || defined(__aarch64__)
			float32x4_t inv_scalar = vdupq_n_f32(1.0f / s);
			data = vmulq_f32(data, inv_scalar);
#endif
			return *this;
		}

		Vector4& operator/=(const Vector4& rhs) noexcept
		{
			*this = *this / rhs;
			return *this;
		}

		Vector4 operator-() const noexcept
		{
			Vector4 Result;
#if defined(__APPLE__)
			Result.data = -data;
#elif defined(_M_X64) || defined(__x86_64__)
			__m128 neg = _mm_set1_ps(-0.0f);
			Result.data = _mm_xor_ps(data, neg);
#elif defined(_M_ARM64) || defined(__aarch64__)
			Result.data = vnegq_f32(data);
#endif
			return Result;
		}

		// Сравнение 
		bool operator==(const Vector4& rhs) const noexcept
		{
#if defined(__APPLE__)
			return ::simd::all(data == rhs.data);
#elif defined(_M_X64) || defined(__x86_64__)
			__m128 cmp = _mm_cmpeq_ps(data, rhs.data);
			return _mm_movemask_ps(cmp) == 0x0F;
#elif defined(_M_ARM64) || defined(__aarch64__)
			uint32x4_t cmp = vceqq_f32(data, rhs.data);
			uint32x2_t tmp = vand_u32(vget_low_u32(cmp), vget_high_u32(cmp));
			return vget_lane_u32(tmp, 0) && vget_lane_u32(tmp, 1);
#endif
		}

		bool operator!=(const Vector4& rhs) const noexcept { return !(*this == rhs); }

		// Длина и нормализация 
		float lengthSq() const noexcept
		{
#if defined(__APPLE__)
			return ::simd::dot(data, data);
#else
			return this->dot(*this);
#endif
		}

		float lengthSq3() const noexcept
		{
#if defined(__APPLE__)
			simd_float3 xyz = ::simd::make_float3(data.x, data.y, data.z);
			return ::simd::dot(xyz, xyz);
#else
			return x() * x() + y() * y() + z() * z();
#endif
		}

		float length() const noexcept { return std::sqrt(lengthSq()); }

		float length3() const noexcept { return std::sqrt(lengthSq3()); }

		Vector4 normalized() const noexcept
		{
			float len = length();
			constexpr float epsilon = 1e-8f;
			return (len < epsilon) ? Vector4{} : *this / len;
		}

		void normalize() noexcept
		{
			*this = normalized();
		}

		// Скалярное произведение 
		float dot(const Vector4& rhs) const noexcept
		{
#if defined(__APPLE__)
			return ::simd::dot(data, rhs.data);
#elif defined(_M_X64) || defined(__x86_64__)
			__m128 mul = _mm_mul_ps(data, rhs.data);
			__m128 shuf1 = _mm_movehdup_ps(mul);
			__m128 sum1 = _mm_add_ps(mul, shuf1);
			__m128 shuf2 = _mm_movehl_ps(sum1, sum1);
			__m128 sum2 = _mm_add_ss(sum1, shuf2);
			return _mm_cvtss_f32(sum2);
#elif defined(_M_ARM64) || defined(__aarch64__)
			float32x4_t mul = vmulq_f32(data, rhs.data);
			float32x2_t sum1 = vadd_f32(vget_low_f32(mul), vget_high_f32(mul));
			return vget_lane_f32(vpadd_f32(sum1, sum1), 0);
#endif
		}

		float dot3(const Vector4& rhs) const noexcept
		{
#if defined(__APPLE__)
			// simd_dot включает все 4 компонента, нужно обнулить w
			simd::float4 v1 = simd_make_float4(data.x, data.y, data.z, 0.0f);
			simd::float4 v2 = simd_make_float4(rhs.data.x, rhs.data.y, rhs.data.z, 0.0f);
			return ::simd::dot(v1, v2);
#elif defined(_M_X64) || defined(__x86_64__)
			// Обнуляем w компоненту через маску
			__m128 mask = _mm_castsi128_ps(_mm_setr_epi32(-1, -1, -1, 0));
			__m128 v1 = _mm_and_ps(data, mask);
			__m128 v2 = _mm_and_ps(rhs.data, mask);

			__m128 mul = _mm_mul_ps(v1, v2);
			__m128 shuf1 = _mm_movehdup_ps(mul);
			__m128 sum1 = _mm_add_ps(mul, shuf1);
			__m128 shuf2 = _mm_movehl_ps(sum1, sum1);
			__m128 sum2 = _mm_add_ss(sum1, shuf2);
			return _mm_cvtss_f32(sum2);
#elif defined(_M_ARM64) || defined(__aarch64__)
			// Используем только первые 3 компонента
			float32x2_t low1 = vget_low_f32(data);
			float32x2_t low2 = vget_low_f32(rhs.data);
			float32x2_t high1 = vget_high_f32(data);
			float32x2_t high2 = vget_high_f32(rhs.data);

			// mul.xy = low1 * low2, mul.z = high1[0] * high2[0]
			float32x2_t mul_low = vmul_f32(low1, low2);
			float z_mul = vget_lane_f32(high1, 0) * vget_lane_f32(high2, 0);

			// sum = x + y + z
			float32x2_t sum_xy = vpadd_f32(mul_low, mul_low);
			return vget_lane_f32(sum_xy, 0) + z_mul;
#endif
		}

		// Линейная интерполяция 
		static Vector4 lerp(const Vector4& a, const Vector4& b, float t) noexcept
		{
			return a + (b - a) * t;
		}

		// Ограничение значений по компонентно(SIMD-оптимизированная) 
		Vector4 clamp(const Vector4& minVal, const Vector4& maxVal) const noexcept
		{
			Vector4 Result;
#if defined(__APPLE__)
			Result.data = ::simd::clamp(data, minVal.data, maxVal.data);
#elif defined(_M_X64) || defined(__x86_64__)
			Result.data = _mm_min_ps(_mm_max_ps(data, minVal.data), maxVal.data);
#elif defined(_M_ARM64) || defined(__aarch64__)
			Result.data = vminq_f32(vmaxq_f32(data, minVal.data), maxVal.data);
#endif
			return Result;
		}

		Vector4 clamp(float minVal, float maxVal) const noexcept
		{
			return clamp(Vector4(minVal), Vector4(maxVal));
		}

		// Покомпонентный min/max (SIMD-оптимизированные) 
		Vector4 compMin(const Vector4& rhs) const noexcept
		{
			Vector4 Result;
#if defined(__APPLE__)
			Result.data = ::simd::min(data, rhs.data);
#elif defined(_M_X64) || defined(__x86_64__)
			Result.data = _mm_min_ps(data, rhs.data);
#elif defined(_M_ARM64) || defined(__aarch64__)
			Result.data = vminq_f32(data, rhs.data);
#endif
			return Result;
		}

		Vector4 compMax(const Vector4& rhs) const noexcept
		{
			Vector4 Result;
#if defined(__APPLE__)
			Result.data = ::simd::max(data, rhs.data);
#elif defined(_M_X64) || defined(__x86_64__)
			Result.data = _mm_max_ps(data, rhs.data);
#elif defined(_M_ARM64) || defined(__aarch64__)
			Result.data = vmaxq_f32(data, rhs.data);
#endif
			return Result;
		}

		// Абсолютное значение 
		Vector4 abs() const noexcept
		{
			Vector4 Result;
#if defined(__APPLE__)
			Result.data = ::simd::abs(data);
#elif defined(_M_X64) || defined(__x86_64__)
			__m128 mask = _mm_castsi128_ps(_mm_set1_epi32(0x7FFFFFFF));
			Result.data = _mm_and_ps(data, mask);
#elif defined(_M_ARM64) || defined(__aarch64__)
			Result.data = vabsq_f32(data);
#endif
			return Result;
		}

		// Минимальный/максимальный компонент 
		float minComponent() const noexcept
		{
#if defined(__APPLE__)
			return ::simd::reduce_min(data);
#elif defined(_M_X64) || defined(__x86_64__)
			__m128 temp = _mm_min_ps(data, _mm_shuffle_ps(data, data, _MM_SHUFFLE(2, 3, 0, 1)));
			temp = _mm_min_ps(temp, _mm_shuffle_ps(temp, temp, _MM_SHUFFLE(1, 0, 3, 2)));
			return _mm_cvtss_f32(temp);
#elif defined(_M_ARM64) || defined(__aarch64__)
			float32x2_t min1 = vpmin_f32(vget_low_f32(data), vget_high_f32(data));
			return vget_lane_f32(vpmin_f32(min1, min1), 0);
#endif
		}

		float maxComponent() const noexcept
		{
#if defined(__APPLE__)
			return ::simd::reduce_max(data);
#elif defined(_M_X64) || defined(__x86_64__)
			__m128 temp = _mm_max_ps(data, _mm_shuffle_ps(data, data, _MM_SHUFFLE(2, 3, 0, 1)));
			temp = _mm_max_ps(temp, _mm_shuffle_ps(temp, temp, _MM_SHUFFLE(1, 0, 3, 2)));
			return _mm_cvtss_f32(temp);
#elif defined(_M_ARM64) || defined(__aarch64__)
			float32x2_t max1 = vpmax_f32(vget_low_f32(data), vget_high_f32(data));
			return vget_lane_f32(vpmax_f32(max1, max1), 0);
#endif
		}

		// Покомпонентная обратная величина(1/x[0], ...)
		Vector4 reciprocal() const noexcept
		{
			Vector4 Result;
#if defined(__APPLE__)
			Result.data = ::simd::recip(data);
#elif defined(_M_X64) || defined(__x86_64__)
			Result.data = _mm_div_ps(_mm_set1_ps(1.0f), data);
#elif defined(_M_ARM64) || defined(__aarch64__)
			// Используем приближённую обратную величину с уточнением
			float32x4_t inv = vrecpeq_f32(data);
			inv = vmulq_f32(vrecpsq_f32(data, inv), inv);
			Result.data = inv;
#endif
			return Result;
		}

		// Квадратный корень
		Vector4 sqrt() const noexcept
		{
			Vector4 Result;
#if defined(__APPLE__)
			Result.data = ::simd::sqrt(data);
#elif defined(_M_X64) || defined(__x86_64__)
			Result.data = _mm_sqrt_ps(data);
#elif defined(_M_ARM64) || defined(__aarch64__)
			// NEON не имеет прямой sqrt, используем приближение
			float32x4_t rsqrt = vrsqrteq_f32(data);
			rsqrt = vmulq_f32(vrsqrtsq_f32(vmulq_f32(data, rsqrt), rsqrt), rsqrt);
			Result.data = vmulq_f32(data, rsqrt);
#endif
			return Result;
		}

		// Обратный квадратный корень (быстрый)(1/sqrt(x[0], ...)
		Vector4 rsqrt() const noexcept
		{
			Vector4 Result;
#if defined(__APPLE__)
			Result.data = ::simd::rsqrt(data);
#elif defined(_M_X64) || defined(__x86_64__)
			Result.data = _mm_rsqrt_ps(data);
#elif defined(_M_ARM64) || defined(__aarch64__)
			float32x4_t rsqrt = vrsqrteq_f32(data);
			rsqrt = vmulq_f32(vrsqrtsq_f32(vmulq_f32(data, rsqrt), rsqrt), rsqrt);
			Result.data = rsqrt;
#endif
			return Result;
		}

		// Расстояние между векторами 
		float distance(const Vector4& rhs) const noexcept
		{
			return (*this - rhs).length();
		}

		// Квадрат расстояния между векторами
		float distanceSq(const Vector4& rhs) const noexcept
		{
			return (*this - rhs).lengthSq();
		}

		// Строковое представление
		std::string to_string() const
		{
			return "(" + std::to_string((*this)[0]) + ", " + std::to_string((*this)[1]) +
				", " + std::to_string((*this)[2]) + ", " + std::to_string((*this)[3]) + ")";
		}

		// 3D cross product (игнорирует W, результат имеет W=0)
		Vector4 cross3(const Vector4& rhs) const noexcept
		{
			Vector4 Result;
#if defined(__APPLE__)
			// Metal simd::cross работает с float4, но использует только xyz
			simd_float3 a = simd_make_float3(data[0], data[1], data[2]);
			simd_float3 b = simd_make_float3(rhs[0], rhs[1], rhs[2]);
			simd_float3 c = simd_cross(a, b);
			Result = Vector4(c.x, c.y, c.z, 0.0f);
#elif defined(_M_X64) || defined(__x86_64__)
			// SSE оптимизация
			__m128 a_yzx = _mm_shuffle_ps(data, data, _MM_SHUFFLE(3, 0, 2, 1));
			__m128 b_yzx = _mm_shuffle_ps(rhs.data, rhs.data, _MM_SHUFFLE(3, 0, 2, 1));
			__m128 c = _mm_sub_ps(
				_mm_mul_ps(data, b_yzx),
				_mm_mul_ps(a_yzx, rhs.data)
			);
			Result.data = _mm_shuffle_ps(c, c, _MM_SHUFFLE(3, 0, 2, 1));
			Result[3] = 0.0f;
#elif defined(_M_ARM64) || defined(__aarch64__)
			// NEON версия
			Result = Vector4(
				data[1] * rhs[2] - data[2] * rhs[1],
				data[2] * rhs[0] - data[0] * rhs[2],
				data[0] * rhs[1] - data[1] * rhs[0],
				0.0f
			);
#endif
			return Result;
		}
	};

	// Операции с float слева
	inline Vector4 operator*(float s, const Vector4& v) noexcept { return v * s; }

	// Вывод в поток
	inline std::ostream& operator<<(std::ostream& os, const Vector4& v) { return os << v.to_string(); }

	//inline float dot(const Vector4& a, const Vector4& b) noexcept { return a.dot(b); }
	//inline float distance(const Vector4& a, const Vector4& b) noexcept { return a.distance(b); }
	//inline float distanceSq(const Vector4& a, const Vector4& b) noexcept { return a.distanceSq(b); }
	//inline Vector4 normalize(const Vector4& v) noexcept { return v.normalized(); }
	//inline Vector4 abs(const Vector4& v) noexcept { return v.abs(); }
	//inline Vector4 min(const Vector4& a, const Vector4& b) noexcept { return a.compMin(b); }
	//inline Vector4 max(const Vector4& a, const Vector4& b) noexcept { return a.compMax(b); }
	//inline Vector4 clamp(const Vector4& v, const Vector4& minVal, const Vector4& maxVal) noexcept { return v.clamp(minVal, maxVal); }
	//inline Vector4 lerp(const Vector4& a, const Vector4& b, float t) noexcept { return Vector4::lerp(a, b, t); }
	//inline Vector4 cross3(const Vector4& a, const Vector4& b) noexcept { return a.cross3(b);
}