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
			data = ::simd::float4{ 0,0,0,0 };
#elif defined(_M_X64) || defined(__x86_64__)
			data = _mm_setzero_ps();
#elif defined(_M_ARM64) || defined(__aarch64__)
			data = vdupq_n_f32(0.0f);
#endif
		}

		explicit VectorBase(float v) noexcept
		{
#if defined(__APPLE__)
			data = ::simd::float4{ v,v,v,0 };
#elif defined(_M_X64) || defined(__x86_64__)
			data = _mm_set_ps(0.0f, v, v, v);
#elif defined(_M_ARM64) || defined(__aarch64__)
			float values[4] = { v,v,v,0.0f };
			data = vld1q_f32(values);
#endif
		}

		VectorBase(float x, float y, float z, float w = 0.0f) noexcept
		{
#if defined(__APPLE__)
			data = ::simd::float4{ x,y,z,0 };
#elif defined(_M_X64) || defined(__x86_64__)
			data = _mm_set_ps(0.0f, z, y, x);
#elif defined(_M_ARM64) || defined(__aarch64__)
			float values[4] = { x,y,z,0.0f };
			data = vld1q_f32(values);
#endif
			WPolicy::set_w(data, w);
		}

		// ---------------- Getters / Setters ----------------
		float x() const noexcept { return reinterpret_cast<const float*>(&data)[0]; }
		float y() const noexcept { return reinterpret_cast<const float*>(&data)[1]; }
		float z() const noexcept { return reinterpret_cast<const float*>(&data)[2]; }
		float w() const noexcept { return WPolicy::get_w(data); }

		void set_x(float v) noexcept { reinterpret_cast<float*>(&data)[0] = v; }
		void set_y(float v) noexcept { reinterpret_cast<float*>(&data)[1] = v; }
		void set_z(float v) noexcept { reinterpret_cast<float*>(&data)[2] = v; }
		void set_w(float v) noexcept { WPolicy::set_w(data, v); }

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
			WPolicy::set_w(r.data, 0.0f);
			return r;
		}

		VectorBase operator-() const noexcept { return VectorBase{} - *this; }

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
			WPolicy::set_w(r.data, 0.0f);
			return r;
		}

		VectorBase operator/(float s) const noexcept { return *this * (1.0f / s); }

		VectorBase& operator+=(const VectorBase& rhs) noexcept { *this = *this + rhs; return *this; }
		VectorBase& operator-=(const VectorBase& rhs) noexcept { *this = *this - rhs; return *this; }
		VectorBase& operator*=(float s) noexcept { *this = *this * s; return *this; }
		VectorBase& operator/=(float s) noexcept { *this = *this * (1.0f / s); return *this; }

		// ---------------- Comparison ----------------
		bool operator==(const VectorBase& rhs) const noexcept
		{
			return x() == rhs.x() && y() == rhs.y() && z() == rhs.z();
		}
		bool operator!=(const VectorBase& rhs) const noexcept { return !(*this == rhs); }

		// ---------------- Vector math ----------------
		float dot(const VectorBase& rhs) const noexcept
		{
			return x() * rhs.x() + y() * rhs.y() + z() * rhs.z();
		}

		VectorBase cross(const VectorBase& rhs) const noexcept
		{
			return VectorBase(
				y() * rhs.z() - z() * rhs.y(),
				z() * rhs.x() - x() * rhs.z(),
				x() * rhs.y() - y() * rhs.x()
			);
		}

		float lengthSq() const noexcept { return dot(*this); }
		float length() const noexcept { return std::sqrt(lengthSq()); }

		VectorBase normalized() const noexcept
		{
			float len = length();
			constexpr float eps = 1e-8f;
			return (len < eps) ? VectorBase{} : (*this / len);
		}

		void normalize() noexcept { *this = normalized(); }

		float distanceSq(const VectorBase& rhs) const noexcept { return (*this - rhs).lengthSq(); }
		float distance(const VectorBase& rhs) const noexcept { return std::sqrt(distanceSq(rhs)); }

		static VectorBase lerp(const VectorBase& a, const VectorBase& b, float t) noexcept
		{
			return a + (b - a) * t;
		}

		std::string to_string() const
		{
			return "(" + std::to_string(x()) + ", " + std::to_string(y()) + ", " + std::to_string(z()) +
				(WPolicy::get_w(data) != 0.0f ? ", " + std::to_string(WPolicy::get_w(data)) : "") + ")";
		}

		// Маска для SIMD (w=0)
#if defined(_M_X64) || defined(__x86_64__)
		static inline __m128 mask_xyz() noexcept
		{
			return _mm_castsi128_ps(_mm_set_epi32(0, -1, -1, -1));
		}
#elif defined(_M_ARM64) || defined(__aarch64__)
		static inline float32x4_t mask_xyz() noexcept
		{
			return { 1.0f,1.0f,1.0f,0.0f };
		}
#endif
	};

	using Vector3 = VectorBase<NoW>;
	using Vector4 = VectorBase<HasW>;

	inline Vector3 operator*(float s, const Vector3& v) noexcept { return v * s; }
	inline Vector4 operator*(float s, const Vector4& v) noexcept { return v * s; }

	inline std::ostream& operator<<(std::ostream& os, const Vector3& v) { return os << v.to_string(); }
	inline std::ostream& operator<<(std::ostream& os, const Vector4& v) { return os << v.to_string(); }
}
