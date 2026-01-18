//
//#include "../headers/headerSIMD.h"
//
//export module Vector3;
//
//export namespace zzz::math
//{
//	struct alignas(16) Vector3
//	{
//		simd_float4 data; // xyz + w = 0
//
//		// ---------------- Constructors ----------------
//		Vector3() noexcept
//		{
//#if defined(__APPLE__)
//			data = ::simd::float4{ 0,0,0,0 };
//#elif defined(_M_X64) || defined(__x86_64__)
//			data = _mm_setzero_ps();
//#elif defined(_M_ARM64) || defined(__aarch64__)
//			data = vdupq_n_f32(0.0f);
//#endif
//		}
//
//		explicit Vector3(float v) noexcept
//		{
//#if defined(__APPLE__)
//			data = ::simd::float4{ v,v,v,0.0f };
//#elif defined(_M_X64) || defined(__x86_64__)
//			data = _mm_set_ps(0.0f, v, v, v);
//#elif defined(_M_ARM64) || defined(__aarch64__)
//			float values[4] = { v, v, v, 0.0f };
//			data = vld1q_f32(values);
//#endif
//		}
//
//		Vector3(float x, float y, float z) noexcept
//		{
//#if defined(__APPLE__)
//			data = ::simd::float4{ x,y,z,0.0f };
//#elif defined(_M_X64) || defined(__x86_64__)
//			data = _mm_set_ps(0.0f, z, y, x);
//#elif defined(_M_ARM64) || defined(__aarch64__)
//			float values[4] = { x, y, z, 0.0f };
//			data = vld1q_f32(values);
//#endif
//		}
//
//		explicit Vector3(simd_float4 v) noexcept : data(v)
//		{
//			set_w(0.0f);
//		}
//
//		// ---------------- Static SIMD Mask ----------------
//#if defined(_M_X64) || defined(__x86_64__)
//		static inline __m128 mask_xyz() noexcept
//		{
//			return _mm_castsi128_ps(_mm_set_epi32(0, -1, -1, -1));
//		}
//#elif defined(_M_ARM64) || defined(__aarch64__)
//		static inline float32x4_t mask_xyz() noexcept
//		{
//			return { 1.0f, 1.0f, 1.0f, 0.0f };
//		}
//#endif
//
//		// ---------------- Getters ----------------
//		inline float x() const noexcept
//		{
//#if defined(__APPLE__)
//			return data.x;
//#else
//			return reinterpret_cast<const float*>(&data)[0];
//#endif
//		}
//
//		inline float y() const noexcept
//		{
//#if defined(__APPLE__)
//			return data.y;
//#else
//			return reinterpret_cast<const float*>(&data)[1];
//#endif
//		}
//
//		inline float z() const noexcept
//		{
//#if defined(__APPLE__)
//			return data.z;
//#else
//			return reinterpret_cast<const float*>(&data)[2];
//#endif
//		}
//
//		// ---------------- Setters ----------------
//		inline void set_x(float v) noexcept
//		{
//#if defined(__APPLE__)
//			data.x = v;
//#else
//			reinterpret_cast<float*>(&data)[0] = v;
//#endif
//		}
//
//		inline void set_y(float v) noexcept
//		{
//#if defined(__APPLE__)
//			data.y = v;
//#else
//			reinterpret_cast<float*>(&data)[1] = v;
//#endif
//		}
//
//		inline void set_z(float v) noexcept
//		{
//#if defined(__APPLE__)
//			data.z = v;
//#else
//			reinterpret_cast<float*>(&data)[2] = v;
//#endif
//		}
//
//		inline void set_w(float v) noexcept
//		{
//#if defined(__APPLE__)
//			data.w = v;
//#else
//			reinterpret_cast<float*>(&data)[3] = v;
//#endif
//		}
//
//		// ---------------- Indexing ----------------
//		inline float& operator[](size_t i) noexcept { return reinterpret_cast<float*>(&data)[i]; }
//		inline const float& operator[](size_t i) const noexcept { return reinterpret_cast<const float*>(&data)[i]; }
//
//		// ---------------- Arithmetic ----------------
//		Vector3 operator+(const Vector3& rhs) const noexcept
//		{
//			Vector3 r;
//#if defined(__APPLE__)
//			r.data = data + rhs.data;
//#elif defined(_M_X64) || defined(__x86_64__)
//			r.data = _mm_and_ps(_mm_add_ps(data, rhs.data), mask_xyz());
//#elif defined(_M_ARM64) || defined(__aarch64__)
//			r.data = vmulq_f32(vaddq_f32(data, rhs.data), mask_xyz());
//#endif
//			return r;
//		}
//
//		Vector3 operator-(const Vector3& rhs) const noexcept
//		{
//			Vector3 r;
//#if defined(__APPLE__)
//			r.data = data - rhs.data;
//#elif defined(_M_X64) || defined(__x86_64__)
//			r.data = _mm_and_ps(_mm_sub_ps(data, rhs.data), mask_xyz());
//#elif defined(_M_ARM64) || defined(__aarch64__)
//			r.data = vmulq_f32(vsubq_f32(data, rhs.data), mask_xyz());
//#endif
//			return r;
//		}
//
//		Vector3 operator*(float s) const noexcept
//		{
//			Vector3 r;
//#if defined(__APPLE__)
//			r.data = data * s;
//#elif defined(_M_X64) || defined(__x86_64__)
//			r.data = _mm_and_ps(_mm_mul_ps(data, _mm_set1_ps(s)), mask_xyz());
//#elif defined(_M_ARM64) || defined(__aarch64__)
//			r.data = vmulq_f32(data, vdupq_n_f32(s));
//			r.data = vmulq_f32(r.data, mask_xyz());
//#endif
//			return r;
//		}
//
//		Vector3 operator/(float s) const noexcept { return (*this) * (1.0f / s); }
//
//		Vector3& operator+=(const Vector3& rhs) noexcept { return *this = *this + rhs; }
//		Vector3& operator-=(const Vector3& rhs) noexcept { return *this = *this - rhs; }
//		Vector3& operator*=(float s) noexcept { return *this = *this * s; }
//		Vector3& operator/=(float s) noexcept { return *this = *this * (1.0f / s); }
//		inline bool operator==(const Vector3& rhs) const noexcept { return x() == rhs.x() && y() == rhs.y() && z() == rhs.z(); }
//		inline bool operator!=(const Vector3& rhs) const noexcept { return !(*this == rhs); }
//
//		// ---------------- Vector math ----------------
//		float dot(const Vector3& rhs) const noexcept
//		{
//#if defined(__APPLE__)
//			simd_float3 a = simd_make_float3(data.x, data.y, data.z);
//			simd_float3 b = simd_make_float3(rhs.data.x, rhs.data.y, rhs.data.z);
//			return ::simd::dot(a, b);
//#else
//			return x() * rhs.x() + y() * rhs.y() + z() * rhs.z();
//#endif
//		}
//
//		Vector3 cross(const Vector3& rhs) const noexcept
//		{
//			return Vector3(
//				y() * rhs.z() - z() * rhs.y(),
//				z() * rhs.x() - x() * rhs.z(),
//				x() * rhs.y() - y() * rhs.x()
//			);
//		}
//
//		float lengthSq() const noexcept { return dot(*this); }
//		float length() const noexcept { return std::sqrt(lengthSq()); }
//
//		Vector3 normalized() const noexcept
//		{
//			float len = length();
//			constexpr float eps = 1e-8f;
//			return (len < eps) ? Vector3{} : (*this / len);
//		}
//
//		void normalize() noexcept { *this = normalized(); }
//
//		// ---------------- Distance ----------------
//		float distanceSq(const Vector3& rhs) const noexcept { return (*this - rhs).lengthSq(); }
//		float distance(const Vector3& rhs) const noexcept { return std::sqrt(distanceSq(rhs)); }
//
//		// ---------------- Utilities ----------------
//		std::string to_string() const
//		{
//			return "(" + std::to_string(x()) + ", " +
//				std::to_string(y()) + ", " +
//				std::to_string(z()) + ")";
//		}
//	};
//
//	inline Vector3 operator*(float s, const Vector3& v) noexcept { return v * s; }
//	inline std::ostream& operator<<(std::ostream& os, const Vector3& v) { return os << v.to_string(); }
//}
