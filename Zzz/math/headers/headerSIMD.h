#pragma once

// -------------------------------------------------------------
// zMath: simd_float4 — кроссплатформенный SIMD (4 x float)
// Только 64-bit: x86_64, aarch64, Apple Silicon
// -------------------------------------------------------------

#if !defined(__x86_64__) && !defined(_M_X64) && !defined(__aarch64__) && !defined(_M_ARM64)
	#error ">>>>> zMath: simd_float4: Only 64-bit platforms supported!"
#endif

// ------------------- Windows (MSVC) -------------------
#if defined(_MSC_VER)
	#if defined(_M_ARM64)
		#include <arm_neon.h>
		using simd_float4 = float32x4_t;
	#else
		#include <immintrin.h>
		using simd_float4 = __m128;
	#endif

// ------------------- Apple (Metal, macOS/iOS) -------------------
#elif defined(__APPLE__)
	#include <TargetConditionals.h>
	#if TARGET_OS_MAC || TARGET_OS_IPHONE || TARGET_OS_SIMULATOR
		#include <simd/simd.h>
		using simd_float4 = ::simd::float4;
	#else
		#error ">>>>> zMath: Apple platform not supported"
	#endif

// ------------------- Linux / Android (Clang/GCC) -------------------
#elif defined(__clang__) || defined(__GNUC__)
	#if defined(__x86_64__)
		#include <immintrin.h>
		using simd_float4 = __m128;
	#elif defined(__aarch64__)
		#include <arm_neon.h>
		using simd_float4 = float32x4_t;
	#else
		#error ">>>>> zMath: Unsupported architecture"
	#endif

#else
	#error ">>>>> zMath: Unsupported compiler"
#endif

// Дополнительные типы для Metal)
//#if defined(__APPLE__)
//	using simd_float3 = ::simd::float3;
//	using simd_float4x4 = ::simd::float4x4;
//#endif