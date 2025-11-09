#include "pch.h"

#include "../headers/headerSIMD.h"

export module vector4;

export namespace zzz::math
{
	struct alignas(16) vector4
	{
		simd_float4 data;

		// Конструкторы 
		vector4() noexcept
		{
#if defined(__APPLE__)
			data = ::simd::float4{ 0,0,0,0 };
#elif defined(_M_X64) || defined(__x86_64__)
			data = _mm_setzero_ps();
#elif defined(_M_ARM64) || defined(__aarch64__)
			data = vdupq_n_f32(0.0f);
#endif
		}

		vector4(float v) noexcept
		{
#if defined(__APPLE__)
			data = ::simd::float4{ v,v,v,v };
#elif defined(_M_X64) || defined(__x86_64__)
			data = _mm_set1_ps(v);
#elif defined(_M_ARM64) || defined(__aarch64__)
			data = vdupq_n_f32(v);
#endif
		}

		vector4(float x, float y, float z, float w) noexcept
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

		// Доступ к компонентам 
		float& operator[](size_t i) noexcept
		{
#if defined(__APPLE__)
			return data[i];
#else
			return reinterpret_cast<float*>(&data)[i];
#endif
		}
		const float& operator[](size_t i) const noexcept
		{
#if defined(__APPLE__)
			return data[i];
#else
			return reinterpret_cast<const float*>(&data)[i];
#endif
		}

		// Арифметика 
		vector4 operator+(const vector4& rhs) const noexcept
		{
			vector4 result;
#if defined(__APPLE__)
			result.data = data + rhs.data;
#elif defined(_M_X64) || defined(__x86_64__)
			result.data = _mm_add_ps(data, rhs.data);
#elif defined(_M_ARM64) || defined(__aarch64__)
			result.data = vaddq_f32(data, rhs.data);
#endif
			return result;
		}

		vector4 operator-(const vector4& rhs) const noexcept
		{
			vector4 result;
#if defined(__APPLE__)
			result.data = data - rhs.data;
#elif defined(_M_X64) || defined(__x86_64__)
			result.data = _mm_sub_ps(data, rhs.data);
#elif defined(_M_ARM64) || defined(__aarch64__)
			result.data = vsubq_f32(data, rhs.data);
#endif
			return result;
		}

		vector4 operator*(float s) const noexcept
		{
			vector4 result;
#if defined(__APPLE__)
			result.data = data * s;
#elif defined(_M_X64) || defined(__x86_64__)
			__m128 scalar = _mm_set1_ps(s);
			result.data = _mm_mul_ps(data, scalar);
#elif defined(_M_ARM64) || defined(__aarch64__)
			float32x4_t scalar = vdupq_n_f32(s);
			result.data = vmulq_f32(data, scalar);
#endif
			return result;
		}

		vector4 operator/(float s) const noexcept
		{
			vector4 result;
#if defined(__APPLE__)
			result.data = data / s;
#elif defined(_M_X64) || defined(__x86_64__)
			__m128 scalar = _mm_set1_ps(s);
			result.data = _mm_div_ps(data, scalar);
#elif defined(_M_ARM64) || defined(__aarch64__)
			// Используем обратное умножение для совместимости со старыми версиями NEON
			float32x4_t inv_scalar = vdupq_n_f32(1.0f / s);
			result.data = vmulq_f32(data, inv_scalar);
#endif
			return result;
		}

		// Покомпонентное умножение
		vector4 operator*(const vector4& rhs) const noexcept
		{
			vector4 result;
#if defined(__APPLE__)
			result.data = data * rhs.data;
#elif defined(_M_X64) || defined(__x86_64__)
			result.data = _mm_mul_ps(data, rhs.data);
#elif defined(_M_ARM64) || defined(__aarch64__)
			result.data = vmulq_f32(data, rhs.data);
#endif
			return result;
		}

		// Покомпонентное деление
		vector4 operator/(const vector4& rhs) const noexcept
		{
			vector4 result;
#if defined(__APPLE__)
			result.data = data / rhs.data;
#elif defined(_M_X64) || defined(__x86_64__)
			result.data = _mm_div_ps(data, rhs.data);
#elif defined(_M_ARM64) || defined(__aarch64__)
			// Обратное умножение для каждого компонента
			float32x4_t inv = vrecpeq_f32(rhs.data); // Приближённая обратная величина
			inv = vmulq_f32(vrecpsq_f32(rhs.data, inv), inv); // Уточнение Newton-Raphson
			result.data = vmulq_f32(data, inv);
#endif
			return result;
		}

		// Оптимизированные операции присваивания
		vector4& operator+=(const vector4& rhs) noexcept
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

		vector4& operator-=(const vector4& rhs) noexcept
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

		vector4& operator*=(float s) noexcept
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

		vector4& operator*=(const vector4& rhs) noexcept
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

		vector4& operator/=(float s) noexcept
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

		vector4& operator/=(const vector4& rhs) noexcept
		{
			*this = *this / rhs;
			return *this;
		}

		vector4 operator-() const noexcept
		{
			vector4 result;
#if defined(__APPLE__)
			result.data = -data;
#elif defined(_M_X64) || defined(__x86_64__)
			__m128 neg = _mm_set1_ps(-0.0f);
			result.data = _mm_xor_ps(data, neg);
#elif defined(_M_ARM64) || defined(__aarch64__)
			result.data = vnegq_f32(data);
#endif
			return result;
		}

		// Сравнение 
		bool operator==(const vector4& rhs) const noexcept
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

		bool operator!=(const vector4& rhs) const noexcept { return !(*this == rhs); }

		// Длина и нормализация 
		float lengthSq() const noexcept
		{
#if defined(__APPLE__)
			return ::simd::dot(data, data);
#else
			return this->dot(*this);
#endif
		}

		float length() const noexcept { return std::sqrt(lengthSq()); }

		vector4 normalized() const noexcept
		{
			float len = length();
			constexpr float epsilon = 1e-8f;
			return (len < epsilon) ? vector4{} : *this / len;
		}

		void normalize() noexcept
		{
			*this = normalized();
		}

		// Скалярное произведение 
		float dot(const vector4& rhs) const noexcept
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

		// Линейная интерполяция 
		static vector4 lerp(const vector4& a, const vector4& b, float t) noexcept
		{
			return a + (b - a) * t;
		}

		// Ограничение значений по компонентно(SIMD-оптимизированная) 
		vector4 clamp(const vector4& minVal, const vector4& maxVal) const noexcept
		{
			vector4 result;
#if defined(__APPLE__)
			result.data = ::simd::clamp(data, minVal.data, maxVal.data);
#elif defined(_M_X64) || defined(__x86_64__)
			result.data = _mm_min_ps(_mm_max_ps(data, minVal.data), maxVal.data);
#elif defined(_M_ARM64) || defined(__aarch64__)
			result.data = vminq_f32(vmaxq_f32(data, minVal.data), maxVal.data);
#endif
			return result;
		}

		vector4 clamp(float minVal, float maxVal) const noexcept
		{
			return clamp(vector4(minVal), vector4(maxVal));
		}

		// Покомпонентный min/max (SIMD-оптимизированные) 
		vector4 compMin(const vector4& rhs) const noexcept
		{
			vector4 result;
#if defined(__APPLE__)
			result.data = ::simd::min(data, rhs.data);
#elif defined(_M_X64) || defined(__x86_64__)
			result.data = _mm_min_ps(data, rhs.data);
#elif defined(_M_ARM64) || defined(__aarch64__)
			result.data = vminq_f32(data, rhs.data);
#endif
			return result;
		}

		vector4 compMax(const vector4& rhs) const noexcept
		{
			vector4 result;
#if defined(__APPLE__)
			result.data = ::simd::max(data, rhs.data);
#elif defined(_M_X64) || defined(__x86_64__)
			result.data = _mm_max_ps(data, rhs.data);
#elif defined(_M_ARM64) || defined(__aarch64__)
			result.data = vmaxq_f32(data, rhs.data);
#endif
			return result;
		}

		// Абсолютное значение 
		vector4 abs() const noexcept
		{
			vector4 result;
#if defined(__APPLE__)
			result.data = ::simd::abs(data);
#elif defined(_M_X64) || defined(__x86_64__)
			__m128 mask = _mm_castsi128_ps(_mm_set1_epi32(0x7FFFFFFF));
			result.data = _mm_and_ps(data, mask);
#elif defined(_M_ARM64) || defined(__aarch64__)
			result.data = vabsq_f32(data);
#endif
			return result;
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
		vector4 reciprocal() const noexcept
		{
			vector4 result;
#if defined(__APPLE__)
			result.data = ::simd::recip(data);
#elif defined(_M_X64) || defined(__x86_64__)
			result.data = _mm_div_ps(_mm_set1_ps(1.0f), data);
#elif defined(_M_ARM64) || defined(__aarch64__)
			// Используем приближённую обратную величину с уточнением
			float32x4_t inv = vrecpeq_f32(data);
			inv = vmulq_f32(vrecpsq_f32(data, inv), inv);
			result.data = inv;
#endif
			return result;
		}

		// Квадратный корень
		vector4 sqrt() const noexcept
		{
			vector4 result;
#if defined(__APPLE__)
			result.data = ::simd::sqrt(data);
#elif defined(_M_X64) || defined(__x86_64__)
			result.data = _mm_sqrt_ps(data);
#elif defined(_M_ARM64) || defined(__aarch64__)
			// NEON не имеет прямой sqrt, используем приближение
			float32x4_t rsqrt = vrsqrteq_f32(data);
			rsqrt = vmulq_f32(vrsqrtsq_f32(vmulq_f32(data, rsqrt), rsqrt), rsqrt);
			result.data = vmulq_f32(data, rsqrt);
#endif
			return result;
		}

		// Обратный квадратный корень (быстрый)(1/sqrt(x[0], ...)
		vector4 rsqrt() const noexcept
		{
			vector4 result;
#if defined(__APPLE__)
			result.data = ::simd::rsqrt(data);
#elif defined(_M_X64) || defined(__x86_64__)
			result.data = _mm_rsqrt_ps(data);
#elif defined(_M_ARM64) || defined(__aarch64__)
			float32x4_t rsqrt = vrsqrteq_f32(data);
			rsqrt = vmulq_f32(vrsqrtsq_f32(vmulq_f32(data, rsqrt), rsqrt), rsqrt);
			result.data = rsqrt;
#endif
			return result;
		}

		// Расстояние между векторами 
		float distance(const vector4& rhs) const noexcept
		{
			return (*this - rhs).length();
		}

		// Квадрат расстояния между векторами
		float distanceSq(const vector4& rhs) const noexcept
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
		vector4 cross3(const vector4& rhs) const noexcept
		{
			vector4 result;
#if defined(__APPLE__)
			// Metal simd::cross работает с float4, но использует только xyz
			simd_float3 a = simd_make_float3(data[0], data[1], data[2]);
			simd_float3 b = simd_make_float3(rhs[0], rhs[1], rhs[2]);
			simd_float3 c = simd_cross(a, b);
			result = vector4(c.x, c.y, c.z, 0.0f);
#elif defined(_M_X64) || defined(__x86_64__)
			// SSE оптимизация
			__m128 a_yzx = _mm_shuffle_ps(data, data, _MM_SHUFFLE(3, 0, 2, 1));
			__m128 b_yzx = _mm_shuffle_ps(rhs.data, rhs.data, _MM_SHUFFLE(3, 0, 2, 1));
			__m128 c = _mm_sub_ps(
				_mm_mul_ps(data, b_yzx),
				_mm_mul_ps(a_yzx, rhs.data)
			);
			result.data = _mm_shuffle_ps(c, c, _MM_SHUFFLE(3, 0, 2, 1));
			result[3] = 0.0f;
#elif defined(_M_ARM64) || defined(__aarch64__)
			// NEON версия
			result = vector4(
				data[1] * rhs[2] - data[2] * rhs[1],
				data[2] * rhs[0] - data[0] * rhs[2],
				data[0] * rhs[1] - data[1] * rhs[0],
				0.0f
			);
#endif
			return result;
		}
	};

	// Операции с float слева
	inline vector4 operator*(float s, const vector4& v) noexcept { return v * s; }

	// Вывод в поток
	inline std::ostream& operator<<(std::ostream& os, const vector4& v)
	{
		return os << v.to_string();
	}

	//inline float dot(const vector4& a, const vector4& b) noexcept { return a.dot(b); }
	//inline float distance(const vector4& a, const vector4& b) noexcept { return a.distance(b); }
	//inline float distanceSq(const vector4& a, const vector4& b) noexcept { return a.distanceSq(b); }
	//inline vector4 normalize(const vector4& v) noexcept { return v.normalized(); }
	//inline vector4 abs(const vector4& v) noexcept { return v.abs(); }
	//inline vector4 min(const vector4& a, const vector4& b) noexcept { return a.compMin(b); }
	//inline vector4 max(const vector4& a, const vector4& b) noexcept { return a.compMax(b); }
	//inline vector4 clamp(const vector4& v, const vector4& minVal, const vector4& maxVal) noexcept { return v.clamp(minVal, maxVal); }
	//inline vector4 lerp(const vector4& a, const vector4& b, float t) noexcept { return vector4::lerp(a, b, t); }
	//inline vector4 cross3(const vector4& a, const vector4& b) noexcept { return a.cross3(b);
}