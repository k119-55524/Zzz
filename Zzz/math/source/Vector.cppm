#include "../headers/headerSIMD.h"

export module Vector;

export namespace zzz::math
{
	struct HasW
	{
		static inline void set_w(simd_float4& d, float v) noexcept
		{
#if defined(__APPLE__)
			d.w = v;
#else
			reinterpret_cast<float*>(&d)[3] = v;
#endif
		}

		static inline float get_w(const simd_float4& d) noexcept
		{
#if defined(__APPLE__)
			return d.w;
#else
			return reinterpret_cast<const float*>(&d)[3];
#endif
		}
	};

	struct NoW
	{
		static inline void set_w(simd_float4&, float) noexcept {}
		static inline float get_w(const simd_float4&) noexcept { return 0.0f; }
	};

	template<typename WPolicy>
	struct alignas(16) VectorBase
	{
		simd_float4 data;

		// ---------------- Constructors ----------------
		VectorBase() noexcept
		{
#if defined(__APPLE__)
			data = ::simd::float4{ 0, 0, 0, 0 };
#elif defined(_M_X64) || defined(__x86_64__)
			data = _mm_setzero_ps();
#elif defined(_M_ARM64) || defined(__aarch64__)
			data = vdupq_n_f32(0.0f);
#endif
		}

		explicit VectorBase(float v) noexcept
		{
#if defined(__APPLE__)
			data = ::simd::float4{ v, v, v, v };
#elif defined(_M_X64) || defined(__x86_64__)
			data = _mm_set_ps(v, v, v, v);
#elif defined(_M_ARM64) || defined(__aarch64__)
			float values[4] = { v, v, v, v };
			data = vld1q_f32(values);
#endif
			if constexpr (std::is_same_v<WPolicy, NoW>)
				WPolicy::set_w(data, 0.0f);
		}

		VectorBase(float x, float y, float z, float w = 0.0f) noexcept
		{
#if defined(__APPLE__)
			data = ::simd::float4{ x, y, z, w };
#elif defined(_M_X64) || defined(__x86_64__)
			data = _mm_set_ps(w, z, y, x);
#elif defined(_M_ARM64) || defined(__aarch64__)
			float values[4] = { x, y, z, w };
			data = vld1q_f32(values);
#endif
			if constexpr (std::is_same_v<WPolicy, NoW>)
				WPolicy::set_w(data, 0.0f);
		}

		explicit VectorBase(simd_float4 v) noexcept : data(v)
		{
			if constexpr (std::is_same_v<WPolicy, NoW>)
				WPolicy::set_w(data, 0.0f);
		}

		// ---------------- Getters / Setters ----------------
		float x() const noexcept { return reinterpret_cast<const float*>(&data)[0]; }
		float y() const noexcept { return reinterpret_cast<const float*>(&data)[1]; }
		float z() const noexcept { return reinterpret_cast<const float*>(&data)[2]; }
		float w() const noexcept { return WPolicy::get_w(data); }
		float r() const noexcept { return reinterpret_cast<const float*>(&data)[0]; }
		float g() const noexcept { return reinterpret_cast<const float*>(&data)[1]; }
		float b() const noexcept { return reinterpret_cast<const float*>(&data)[2]; }
		float a() const noexcept { return WPolicy::get_w(data); }

		void set_x(float v) noexcept { reinterpret_cast<float*>(&data)[0] = v; }
		void set_y(float v) noexcept { reinterpret_cast<float*>(&data)[1] = v; }
		void set_z(float v) noexcept { reinterpret_cast<float*>(&data)[2] = v; }
		void set_w(float v) noexcept { WPolicy::set_w(data, v); }
		void set_r(float v) noexcept { reinterpret_cast<float*>(&data)[0] = v; }
		void set_g(float v) noexcept { reinterpret_cast<float*>(&data)[1] = v; }
		void set_b(float v) noexcept { reinterpret_cast<float*>(&data)[2] = v; }
		void set_a(float v) noexcept { WPolicy::set_w(data, v); }

		float& operator[](size_t i) noexcept { return reinterpret_cast<float*>(&data)[i]; }
		const float& operator[](size_t i) const noexcept { return reinterpret_cast<const float*>(&data)[i]; }

		// ---------------- Arithmetic ----------------
		VectorBase operator+(const VectorBase& rhs) const noexcept
		{
			VectorBase r;
#if defined(__APPLE__)
			r.data = data + rhs.data;
#elif defined(_M_X64) || defined(__x86_64__)
			r.data = _mm_add_ps(data, rhs.data);
#elif defined(_M_ARM64) || defined(__aarch64__)
			r.data = vaddq_f32(data, rhs.data);
#endif

			if constexpr (std::is_same_v<WPolicy, NoW>)
				WPolicy::set_w(r.data, 0.0f);

			return r;
		}

		VectorBase operator-(const VectorBase& rhs) const noexcept
		{
			VectorBase r;
#if defined(__APPLE__)
			r.data = data - rhs.data;
#elif defined(_M_X64) || defined(__x86_64__)
			r.data = _mm_sub_ps(data, rhs.data);
#elif defined(_M_ARM64) || defined(__aarch64__)
			r.data = vsubq_f32(data, rhs.data);
#endif

			if constexpr (std::is_same_v<WPolicy, NoW>)
				WPolicy::set_w(r.data, 0.0f);

			return r;
		}

		VectorBase operator-() const noexcept
		{
			return VectorBase{} - *this;
		}

		VectorBase operator*(float s) const noexcept
		{
			VectorBase r;
#if defined(__APPLE__)
			r.data = data * s;
#elif defined(_M_X64) || defined(__x86_64__)
			r.data = _mm_mul_ps(data, _mm_set1_ps(s));
#elif defined(_M_ARM64) || defined(__aarch64__)
			r.data = vmulq_f32(data, vdupq_n_f32(s));
#endif

			if constexpr (std::is_same_v<WPolicy, NoW>)
				WPolicy::set_w(r.data, 0.0f);

			return r;
		}

		VectorBase operator/(float s) const noexcept
		{
			return *this * (1.0f / s);
		}

		VectorBase operator*(const VectorBase& rhs) const noexcept
		{
			VectorBase r;
#if defined(__APPLE__)
			r.data = data * rhs.data;
#elif defined(_M_X64) || defined(__x86_64__)
			r.data = _mm_mul_ps(data, rhs.data);
#elif defined(_M_ARM64) || defined(__aarch64__)
			r.data = vmulq_f32(data, rhs.data);
#endif

			if constexpr (std::is_same_v<WPolicy, NoW>)
				WPolicy::set_w(r.data, 0.0f);

			return r;
		}

		VectorBase operator/(const VectorBase& rhs) const noexcept
		{
			VectorBase r;
#if defined(__APPLE__)
			r.data = data / rhs.data;
#elif defined(_M_X64) || defined(__x86_64__)
			r.data = _mm_div_ps(data, rhs.data);
#elif defined(_M_ARM64) || defined(__aarch64__)
			float32x4_t inv = vrecpeq_f32(rhs.data);
			inv = vmulq_f32(vrecpsq_f32(rhs.data, inv), inv);
			r.data = vmulq_f32(data, inv);
#endif

			if constexpr (std::is_same_v<WPolicy, NoW>)
				WPolicy::set_w(r.data, 0.0f);

			return r;
		}

		// ---------------- Assignment ----------------
		VectorBase& operator+=(const VectorBase& rhs) noexcept { *this = *this + rhs; return *this; }
		VectorBase& operator-=(const VectorBase& rhs) noexcept { *this = *this - rhs; return *this; }
		VectorBase& operator*=(float s) noexcept { *this = *this * s; return *this; }
		VectorBase& operator/=(float s) noexcept { *this = *this / s; return *this; }
		VectorBase& operator*=(const VectorBase& rhs) noexcept { *this = *this * rhs; return *this; }
		VectorBase& operator/=(const VectorBase& rhs) noexcept { *this = *this / rhs; return *this; }

		// ---------------- Comparison ----------------
		bool operator==(const VectorBase& rhs) const noexcept
		{
			return x() == rhs.x() && y() == rhs.y() && z() == rhs.z();
		}
		bool operator!=(const VectorBase& rhs) const noexcept { return !(*this == rhs); }

		float dot(const VectorBase& rhs) const noexcept
		{
			float sum = x() * rhs.x() + y() * rhs.y() + z() * rhs.z();

			if constexpr (!std::is_same_v<WPolicy, NoW>)
				sum += w() * rhs.w();

			return sum;
		}


		float dot3(const VectorBase& rhs) const noexcept
		{
#if defined(__APPLE__)
			simd_float3 a = simd_make_float3(data.x, data.y, data.z);
			simd_float3 b = simd_make_float3(rhs.data.x, rhs.data.y, rhs.data.z);
			return ::simd::dot(a, b);
#else
			return x() * rhs.x() + y() * rhs.y() + z() * rhs.z();
#endif
		}

		inline VectorBase normalized() const noexcept
		{
			float len = length();
			constexpr float eps = 1e-8f;

			return (len < eps) ? VectorBase{} : (*this / len);
		}

		inline void normalize() noexcept { *this = normalized(); }

		inline float distanceSq(const VectorBase& rhs) const noexcept { return (*this - rhs).lengthSq(); }

		inline float distance(const VectorBase& rhs) const noexcept { return std::sqrt(distanceSq(rhs)); }

		static VectorBase lerp(const VectorBase& a, const VectorBase& b, float t) noexcept { return a + (b - a) * t; }

		inline std::string to_string() const
		{
			return "(" + std::to_string(x()) + ", " + std::to_string(y()) + ", " + std::to_string(z()) +
				(WPolicy::get_w(data) != 0.0f ? ", " + std::to_string(WPolicy::get_w(data)) : "") + ")";
		}

		VectorBase cross(const VectorBase& rhs) const noexcept
		{
			return VectorBase(
				y() * rhs.z() - z() * rhs.y(),
				z() * rhs.x() - x() * rhs.z(),
				x() * rhs.y() - y() * rhs.x()
			);
		}

		VectorBase cross3(const VectorBase& rhs) const noexcept
		{
#if defined(__APPLE__)
			simd_float3 a = simd_make_float3(data.x, data.y, data.z);
			simd_float3 b = simd_make_float3(rhs.data.x, rhs.data.y, rhs.data.z);
			simd_float3 c = ::simd::cross(a, b);
			return VectorBase(c.x, c.y, c.z, WPolicy::get_w(data));
#else
			return VectorBase(
				y() * rhs.z() - z() * rhs.y(),
				z() * rhs.x() - x() * rhs.z(),
				x() * rhs.y() - y() * rhs.x(),
				WPolicy::get_w(data)
			);
#endif
		}

		float length() const noexcept { return std::sqrt(lengthSq()); }

		float length3() const noexcept
		{
#if defined(__APPLE__)
			simd_float3 v3 = simd_make_float3(data.x, data.y, data.z);
			return ::simd::length(v3);
#else
			return std::sqrt(x() * x() + y() * y() + z() * z());
#endif
		}

		float lengthSq() const noexcept
		{
			float sum = x() * x() + y() * y() + z() * z();
			if constexpr (!std::is_same_v<WPolicy, NoW>)
				sum += w() * w(); // учитываем w только если есть WPolicy
			return sum;
		}

		// ---------------- Utility ----------------
// Clamp с VectorBase
		VectorBase clamp(const VectorBase& minVal, const VectorBase& maxVal) const noexcept
		{
#if defined(__APPLE__)
			return VectorBase(::simd::clamp(data, minVal.data, maxVal.data));
#elif defined(_M_X64) || defined(__x86_64__)
			__m128 vmin = minVal.data;
			__m128 vmax = maxVal.data;
			__m128 r = _mm_min_ps(_mm_max_ps(data, vmin), vmax);
			return VectorBase(r);
#elif defined(_M_ARM64) || defined(__aarch64__)
			float32x4_t r = vminq_f32(vmaxq_f32(data, minVal.data), maxVal.data);

			return VectorBase(r);
#endif
		}

		// Clamp с float
		VectorBase clamp(float minVal, float maxVal) const noexcept
		{
#if defined(__APPLE__)
			simd_float4 minV = ::simd::float4{ minVal, minVal, minVal, minVal };
			simd_float4 maxV = ::simd::float4{ maxVal, maxVal, maxVal, maxVal };
			return VectorBase(::simd::clamp(data, minV, maxV));
#elif defined(_M_X64) || defined(__x86_64__)
			__m128 minV = _mm_set1_ps(minVal);
			__m128 maxV = _mm_set1_ps(maxVal);
			__m128 r = _mm_min_ps(_mm_max_ps(data, minV), maxV);
			return VectorBase(r);
#elif defined(_M_ARM64) || defined(__aarch64__)
			float32x4_t minV = vdupq_n_f32(minVal);
			float32x4_t maxV = vdupq_n_f32(maxVal);
			float32x4_t r = vminq_f32(vmaxq_f32(data, minV), maxV);

			return VectorBase(r);
#endif
		}

		VectorBase compMin(const VectorBase& rhs) const noexcept
		{
#if defined(__APPLE__)
			return VectorBase(::simd::min(data, rhs.data));
#else
			return VectorBase(
				std::min(x(), rhs.x()),
				std::min(y(), rhs.y()),
				std::min(z(), rhs.z()),
				std::min(w(), rhs.w())   // <- исправлено
			);
#endif
		}

		VectorBase compMax(const VectorBase& rhs) const noexcept
		{
#if defined(__APPLE__)
			return VectorBase(::simd::max(data, rhs.data));
#else
			return VectorBase(
				std::max(x(), rhs.x()),
				std::max(y(), rhs.y()),
				std::max(z(), rhs.z()),
				std::max(w(), rhs.w())   // <- исправлено
			);
#endif
		}

		// Absolute value of each component
		VectorBase abs() const noexcept
		{
#if defined(__APPLE__)
			return VectorBase(::simd::abs(data));
#else
			return VectorBase(
				std::fabs(x()),
				std::fabs(y()),
				std::fabs(z()),
				WPolicy::get_w(data)
			);
#endif
		}

		// Min / Max component
		float minComponent() const noexcept
		{
			if constexpr (std::is_same_v<WPolicy, NoW>)
				return std::min({ x(), y(), z() });
			else
				return std::min({ x(), y(), z(), w() });
		}

		float maxComponent() const noexcept
		{
			if constexpr (std::is_same_v<WPolicy, NoW>)
				return std::max({ x(), y(), z() });
			else
				return std::max({ x(), y(), z(), w() });
		}

		// Reciprocal
		VectorBase reciprocal() const noexcept
		{
#if defined(__APPLE__)
			return VectorBase(::simd::reciprocal(data));
#else
			return VectorBase(
				1.0f / x(),
				1.0f / y(),
				1.0f / z(),
				(std::is_same_v<WPolicy, NoW> ? 0.0f : 1.0f / w())
			);
#endif
		}

		// Square root
		VectorBase sqrt() const noexcept
		{
#if defined(__APPLE__)
			return VectorBase(::simd::sqrt(data));
#else
			return VectorBase(
				std::sqrt(x()),
				std::sqrt(y()),
				std::sqrt(z()),
				(std::is_same_v<WPolicy, NoW> ? 0.0f : std::sqrt(w()))
			);
#endif
		}

		// Reciprocal square root
		VectorBase rsqrt() const noexcept
		{
#if defined(__APPLE__)
			return VectorBase(::simd::rsqrt(data));
#else
			return VectorBase(
				1.0f / std::sqrt(x()),
				1.0f / std::sqrt(y()),
				1.0f / std::sqrt(z()),
				(std::is_same_v<WPolicy, NoW> ? 0.0f : 1.0f / std::sqrt(w()))
			);
#endif
		}

	};

	using Vector3 = VectorBase<NoW>;
	using Vector4 = VectorBase<HasW>;

	inline Vector3 operator*(float s, const Vector3& v) noexcept { return v * s; }
	inline Vector4 operator*(float s, const Vector4& v) noexcept { return v * s; }

	inline std::ostream& operator<<(std::ostream& os, const Vector3& v) { return os << v.to_string(); }
	inline std::ostream& operator<<(std::ostream& os, const Vector4& v) { return os << v.to_string(); }
}