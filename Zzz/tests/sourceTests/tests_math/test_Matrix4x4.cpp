
#include "test_Matrix4x4.h"

import Math;
import Vector4;
import Matrix4x4;

using namespace zzz;
using namespace zzz::math;
using namespace zzz::ztests;

test_Matrix4x4::test_Matrix4x4()
{
}

bool test_Matrix4x4::Initialize()
{
	return true;
}

Result<std::string> test_Matrix4x4::Run()
{
	auto res = TestForGAPI_DirectX();
	if (!res)
		return res;

	return TestForGAPI_MetalVulkan();
}

bool test_Matrix4x4::Kill()
{
	return true;
}

#pragma region DirectX
Result<std::string> test_Matrix4x4::TestForGAPI_DirectX()
{
	auto res = ValidateMatrixVectorMultiply();
	if (!res)
		return res;

	res = ValidatePerspective_DirectX();
	if (!res)
		return res;

	res = ValidateLookAt_DirectX();
	if (!res)
		return res;

	return { "Success" };
}

// --- Тест умножения матриц ---
Result<std::string> test_Matrix4x4::ValidateMatrixVectorMultiply()
{
	struct MatrixVectorTest
	{
		Matrix4x4 matrix;
		Vector4 vector;
		Vector4 expected;
		const wchar_t* description;
	};

	MatrixVectorTest testCases[] = {
		// 1. Единичная матрица * вектор = вектор
		{
			Matrix4x4(), // identity
			Vector4(1.0f, 2.0f, 3.0f, 1.0f),
			Vector4(1.0f, 2.0f, 3.0f, 1.0f),
			L"Identity matrix"
		},

		// 2. Translation матрица
		{
			Matrix4x4::translation(5.0f, 10.0f, 15.0f),
			Vector4(1.0f, 2.0f, 3.0f, 1.0f),
			Vector4(6.0f, 12.0f, 18.0f, 1.0f),
			L"Translation matrix"
		},

		// 3. Scale матрица
		{
			Matrix4x4::scale(2.0f, 3.0f, 4.0f),
			Vector4(1.0f, 2.0f, 3.0f, 1.0f),
			Vector4(2.0f, 6.0f, 12.0f, 1.0f),
			L"Scale matrix"
		},

		// 4. Rotation X на 90 градусов
		{
			Matrix4x4::rotationX(3.14159f / 2.0f),
			Vector4(0.0f, 1.0f, 0.0f, 1.0f),
			Vector4(0.0f, 0.0f, 1.0f, 1.0f),
			L"Rotation X 90 degrees"
		},

		// 5. Rotation Y на 90 градусов
		{
			Matrix4x4::rotationY(3.14159f / 2.0f),
			Vector4(0.0f, 0.0f, 1.0f, 1.0f),
			Vector4(1.0f, 0.0f, 0.0f, 1.0f),
			L"Rotation Y 90 degrees"
		},

		// 6. Rotation Z на 90 градусов
		{
			Matrix4x4::rotationZ(3.14159f / 2.0f),
			Vector4(1.0f, 0.0f, 0.0f, 1.0f),
			Vector4(0.0f, 1.0f, 0.0f, 1.0f),
			L"Rotation Z 90 degrees"
		},

		// 7. Translation * Scale (сначала scale, потом translation)
		{
			Matrix4x4::scale(2.0f)* Matrix4x4::translation(1.0f, 2.0f, 3.0f),
			Vector4(1.0f, 1.0f, 1.0f, 1.0f),
			Vector4(3.0f, 4.0f, 5.0f, 1.0f),
			L"Translation * Scale"
		},

		// 8. Вектор с w=0 (направление) - translation не влияет
		{
			Matrix4x4::translation(10.0f, 20.0f, 30.0f),
			Vector4(1.0f, 2.0f, 3.0f, 0.0f),
			Vector4(1.0f, 2.0f, 3.0f, 0.0f),
			L"Translation with direction vector (w=0)"
		},

		// 9. Произвольная матрица
		{
			Matrix4x4(
				2.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 3.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 4.0f, 0.0f,
				1.0f, 2.0f, 3.0f, 1.0f),
				Vector4(1.0f, 1.0f, 1.0f, 1.0f),
				Vector4(3.0f, 5.0f, 7.0f, 1.0f),
				L"Custom matrix"
		},

		// 10. Нулевой вектор
		{
			Matrix4x4::scale(5.0f),
			Vector4(0.0f, 0.0f, 0.0f, 0.0f),
			Vector4(0.0f, 0.0f, 0.0f, 0.0f),
			L"Zero vector"
		}
	};

	// Прогоняем все тест-кейсы
	for (const auto& testCase : testCases)
	{
		auto result = ValidateMatrixVectorMultiply(testCase.matrix, testCase.vector, testCase.expected, testCase.description);

		if (!result)
			return result;
	}

	return { "Success" };
}

Result<std::string> test_Matrix4x4::ValidateMatrixVectorMultiply(
	const Matrix4x4& matrix,
	const Vector4& vector,
	const Vector4& expectedResult,
	const wchar_t* testName)
{
	const float epsilon = 0.0001f;

	Vector4 result = vector * matrix;

	// Проверяем каждую компоненту
	if (std::abs(result[0] - expectedResult[0]) > epsilon)
	{
		std::wstring msg = L"Matrix * Vector (";
		msg += testName;
		msg += L"): X component incorrect. Expected: ";
		msg += std::to_wstring(expectedResult[0]);
		msg += L", Got: ";
		msg += std::to_wstring(result[0]);
		return Unexpected(msg);
	}

	if (std::abs(result[1] - expectedResult[1]) > epsilon)
	{
		std::wstring msg = L"Matrix * Vector (";
		msg += testName;
		msg += L"): Y component incorrect. Expected: ";
		msg += std::to_wstring(expectedResult[1]);
		msg += L", Got: ";
		msg += std::to_wstring(result[1]);
		return Unexpected(msg);
	}

	if (std::abs(result[2] - expectedResult[2]) > epsilon)
	{
		std::wstring msg = L"Matrix * Vector (";
		msg += testName;
		msg += L"): Z component incorrect. Expected: ";
		msg += std::to_wstring(expectedResult[2]);
		msg += L", Got: ";
		msg += std::to_wstring(result[2]);
		return Unexpected(msg);
	}

	if (std::abs(result[3] - expectedResult[3]) > epsilon)
	{
		std::wstring msg = L"Matrix * Vector (";
		msg += testName;
		msg += L"): W component incorrect. Expected: ";
		msg += std::to_wstring(expectedResult[3]);
		msg += L", Got: ";
		msg += std::to_wstring(result[3]);
		return Unexpected(msg);
	}

	return { "Success" };
}

// --- Тест формирования perspective матрицы --- 
Result<std::string> test_Matrix4x4::ValidatePerspective_DirectX()
{
	struct PerspectiveParams
	{
		float fovY;
		float aspect;
		float nearZ;
		float farZ;
		const wchar_t* description;
	};

	PerspectiveParams testCases[] =
	{
		{ 3.14159f / 2.0f, GetAspect(eAspectType::Ratio_16x9),	0.1f,	100.0f,		L"FOV 90°, 16:9, near=0.1, far=100" },
		{ 3.14159f / 4.0f, GetAspect(eAspectType::Ratio_16x9),	0.1f,	1000.0f,	L"FOV 45°, 16:9, near=0.1, far=1000" },
		{ 3.14159f / 3.0f, GetAspect(eAspectType::Ratio_4x3),	1.0f,	100.0f,		L"FOV 60°, 4:3, near=1.0, far=100" },
		{ 3.14159f / 6.0f, GetAspect(eAspectType::Ratio_21x9),	0.01f,	10000.0f,	L"FOV 30°, 21:9, near=0.01, far=10000" },
		{ 3.14159f / 2.5f, GetAspect(eAspectType::Ratio_1x1),	0.5f,	500.0f,		L"FOV 72°, 1:1, near=0.5, far=500" }
	};

	// Прогоняем все тест-кейсы
	for (const auto& testCase : testCases)
	{
		auto result = ValidatePerspective_DirectX(testCase.fovY, testCase.aspect, testCase.nearZ, testCase.farZ);
		if (!result)
		{
			std::wstring errorMsg = testCase.description;
			errorMsg += L" - ";
			errorMsg += result.error().getMessage();

			return Unexpected(errorMsg);
		}
	}

	return { "Success" };
}

Result<std::string> test_Matrix4x4::ValidatePerspective_DirectX(
	float fovY,
	float aspect,
	float nearZ,
	float farZ)
{
	Matrix4x4 proj = Matrix4x4::perspective(fovY, aspect, nearZ, farZ);

	// Проверка ключевых элементов DirectX perspective матрицы
	float tanHalfFov = std::tan(fovY * 0.5f);
	float expectedM00 = 1.0f / (aspect * tanHalfFov);
	float expectedM11 = 1.0f / tanHalfFov;
	float expectedM22 = farZ / (farZ - nearZ);
	float expectedM32 = -nearZ * farZ / (farZ - nearZ);

	const float epsilon = 0.0001f;

	// Проверяем элементы [row][col]
	if (std::abs(proj[0][0] - expectedM00) > epsilon)
		return Unexpected(L"DirectX Perspective: M[0][0] incorrect");

	if (std::abs(proj[1][1] - expectedM11) > epsilon)
		return Unexpected(L"DirectX Perspective: M[1][1] incorrect");

	if (std::abs(proj[2][2] - expectedM22) > epsilon)
		return Unexpected(L"DirectX Perspective: M[2][2] incorrect");

	if (std::abs(proj[2][3] - 1.0f) > epsilon)
		return Unexpected(L"DirectX Perspective: M[2][3] should be 1.0");

	if (std::abs(proj[3][2] - expectedM32) > epsilon)
		return Unexpected(L"DirectX Perspective: M[3][2] incorrect");

	if (std::abs(proj[3][3] - 0.0f) > epsilon)
		return Unexpected(L"DirectX Perspective: M[3][3] should be 0.0");

	// Проверка трансформации точек
	// Точка на near plane (центр экрана)
	Vector4 nearPoint(0.0f, 0.0f, nearZ, 1.0f);
	Vector4 projNear = nearPoint * proj;
	// После perspective divide z должен быть 0 (near plane -> 0 в DirectX)
	float zNear = projNear[2] / projNear[3];
	if (std::abs(zNear - 0.0f) > epsilon)
		return Unexpected(L"DirectX Perspective: Near plane Z mapping incorrect");

	// Точка на far plane
	Vector4 farPoint(0.0f, 0.0f, farZ, 1.0f);
	Vector4 projFar = farPoint * proj;
	float zFar = projFar[2] / projFar[3];
	if (std::abs(zFar - 1.0f) > epsilon)
		return Unexpected(L"DirectX Perspective: Far plane Z mapping incorrect");

	return { "Success" };
}

// --- Тест формирования LookAt матрицы ---
Result<std::string> test_Matrix4x4::ValidateLookAt_DirectX()
{
	struct LookAtParams
	{
		Vector4 eye;
		Vector4 target;
		Vector4 up;
		const wchar_t* description;
	};

	LookAtParams testCases[] =
	{
		// Базовые тесты (ваши оригинальные)
		{
			Vector4(0.0f, 0.0f, 5.0f, 1.0f),
			Vector4(0.0f, 0.0f, 0.0f, 1.0f),
			Vector4(0.0f, 1.0f, 0.0f, 1.0f),
			L"Standard view along -Z axis from (0,0,5) to origin, up Y"
		},
		{
			Vector4(5.0f, 0.0f, 0.0f, 1.0f),
			Vector4(0.0f, 0.0f, 0.0f, 1.0f),
			Vector4(0.0f, 1.0f, 0.0f, 1.0f),
			L"View along -X axis from (5,0,0) to origin, up Y"
		},
		{
			Vector4(0.0f, 5.0f, 0.0f, 1.0f),
			Vector4(0.0f, 0.0f, 0.0f, 1.0f),
			Vector4(0.0f, 0.0f, 1.0f, 1.0f),
			L"View along -Y axis from (0,5,0) to origin, up Z (adjusted)"
		},
		{
			Vector4(1.0f, 2.0f, 3.0f, 1.0f),
			Vector4(4.0f, 5.0f, 6.0f, 1.0f),
			Vector4(0.0f, 1.0f, 0.0f, 1.0f),
			L"Arbitrary view from (1,2,3) to (4,5,6), up Y"
		},
		{
			Vector4(0.0f, 0.0f, 10.0f, 1.0f),
			Vector4(0.0f, 0.0f, 0.0f, 1.0f),
			Vector4(1.0f, 0.0f, 0.0f, 1.0f),
			L"View along -Z from (0,0,10) to origin, up X (tilted 90°)"
		},

		// Тесты с отрицательными координатами
		{
			Vector4(-3.0f, -4.0f, -5.0f, 1.0f),
			Vector4(0.0f, 0.0f, 0.0f, 1.0f),
			Vector4(0.0f, 1.0f, 0.0f, 1.0f),
			L"View from negative octant (-3,-4,-5) to origin, up Y"
		},
		{
			Vector4(0.0f, 0.0f, -5.0f, 1.0f),
			Vector4(0.0f, 0.0f, 0.0f, 1.0f),
			Vector4(0.0f, 1.0f, 0.0f, 1.0f),
			L"View along +Z axis from (0,0,-5) to origin (behind target), up Y"
		},
		{
			Vector4(5.0f, 3.0f, 2.0f, 1.0f),
			Vector4(-2.0f, -1.0f, -3.0f, 1.0f),
			Vector4(0.0f, 1.0f, 0.0f, 1.0f),
			L"View from positive to negative coordinates (5,3,2) to (-2,-1,-3), up Y"
		},

		// Тесты с разными up векторами
		{
			Vector4(3.0f, 3.0f, 3.0f, 1.0f),
			Vector4(0.0f, 0.0f, 0.0f, 1.0f),
			Vector4(0.0f, 0.0f, 1.0f, 1.0f),
			L"Diagonal view from (3,3,3) to origin, up Z"
		},
		{
			Vector4(5.0f, 5.0f, 0.0f, 1.0f),
			Vector4(0.0f, 0.0f, 0.0f, 1.0f),
			Vector4(1.0f, 0.0f, 0.0f, 1.0f),
			L"View from (5,5,0) to origin, up X (unusual orientation)"
		},
		{
			Vector4(0.0f, 0.0f, 5.0f, 1.0f),
			Vector4(0.0f, 0.0f, 0.0f, 1.0f),
			Vector4(0.0f, -1.0f, 0.0f, 1.0f),
			L"Standard view along -Z with inverted up (-Y, upside down)"
		},

		// Тесты с большими и маленькими расстояниями
		{
			Vector4(0.0f, 0.0f, 0.1f, 1.0f),
			Vector4(0.0f, 0.0f, 0.0f, 1.0f),
			Vector4(0.0f, 1.0f, 0.0f, 1.0f),
			L"Very close view from (0,0,0.1) to origin, up Y"
		},
		{
			Vector4(0.0f, 0.0f, 1000.0f, 1.0f),
			Vector4(0.0f, 0.0f, 0.0f, 1.0f),
			Vector4(0.0f, 1.0f, 0.0f, 1.0f),
			L"Very far view from (0,0,1000) to origin, up Y"
		},
		{
			Vector4(100.0f, 200.0f, 300.0f, 1.0f),
			Vector4(400.0f, 500.0f, 600.0f, 1.0f),
			Vector4(0.0f, 1.0f, 0.0f, 1.0f),
			L"Large coordinates view from (100,200,300) to (400,500,600), up Y"
		},

		// Тесты с ненулевыми целевыми точками
		{
			Vector4(10.0f, 10.0f, 10.0f, 1.0f),
			Vector4(5.0f, 5.0f, 5.0f, 1.0f),
			Vector4(0.0f, 1.0f, 0.0f, 1.0f),
			L"View from (10,10,10) to (5,5,5), up Y"
		},
		{
			Vector4(0.0f, 5.0f, 5.0f, 1.0f),
			Vector4(0.0f, 0.0f, 0.0f, 1.0f),
			Vector4(1.0f, 0.0f, 0.0f, 1.0f),
			L"View from (0,5,5) at 45° angle to origin, up X"
		},

		// Граничные случаи (edge cases)
		{
			Vector4(0.01f, 5.0f, 0.0f, 1.0f),
			Vector4(0.0f, 0.0f, 0.0f, 1.0f),
			Vector4(0.0f, 0.0f, 1.0f, 1.0f),
			L"Nearly along -Y axis (0.01,5,0) to origin, up Z (small X offset)"
		},
		{
			Vector4(7.071f, 0.0f, 7.071f, 1.0f),  // 45° в XZ плоскости
			Vector4(0.0f, 0.0f, 0.0f, 1.0f),
			Vector4(0.0f, 1.0f, 0.0f, 1.0f),
			L"45° angle in XZ plane from (7.071,0,7.071) to origin, up Y"
		},
		{
			Vector4(1.0f, 1.0f, 1.0f, 1.0f),
			Vector4(2.0f, 2.0f, 2.0f, 1.0f),
			Vector4(-1.0f, 2.0f, 0.0f, 1.0f),  // ненормализованный up
			L"Non-normalized up vector (-1,2,0), should be handled by algorithm"
		},

		// Тесты симметрии
		{
			Vector4(-5.0f, 0.0f, 0.0f, 1.0f),
			Vector4(0.0f, 0.0f, 0.0f, 1.0f),
			Vector4(0.0f, 1.0f, 0.0f, 1.0f),
			L"View along +X axis from (-5,0,0) to origin, up Y (opposite of test 2)"
		},
		{
			Vector4(0.0f, -5.0f, 0.0f, 1.0f),
			Vector4(0.0f, 0.0f, 0.0f, 1.0f),
			Vector4(0.0f, 0.0f, -1.0f, 1.0f),
			L"View along +Y axis from (0,-5,0) to origin, up -Z"
		},

		// ВЫРОЖДЕННЫЕ СЛУЧАИ - должны обрабатываться корректно или падать явно
		// Раскомментируйте если хотите протестировать обработку ошибок:
		/*
		{
			Vector4(0.0f, 5.0f, 0.0f, 1.0f),
			Vector4(0.0f, 0.0f, 0.0f, 1.0f),
			Vector4(0.0f, 1.0f, 0.0f, 1.0f),  // up параллелен forward!
			L"DEGENERATE: up parallel to forward (should fail or handle gracefully)"
		},
		{
			Vector4(0.0f, 0.0f, 5.0f, 1.0f),
			Vector4(0.0f, 0.0f, 5.0f, 1.0f),  // eye == target!
			Vector4(0.0f, 1.0f, 0.0f, 1.0f),
			L"DEGENERATE: eye equals target (should fail)"
		},
		{
			Vector4(0.0f, 0.0f, 5.0f, 1.0f),
			Vector4(0.0f, 0.0f, 0.0f, 1.0f),
			Vector4(0.0f, 0.0f, 0.0f, 1.0f),  // zero up vector!
			L"DEGENERATE: zero up vector (should fail)"
		},
		*/
	};

	// Прогоняем все тест-кейсы
	for (const auto& testCase : testCases)
	{
		auto result = ValidateLookAt_DirectX(testCase.eye, testCase.target, testCase.up);
		if (!result)
		{
			std::wstring errorMsg = testCase.description;
			errorMsg += L" - ";
			errorMsg += result.error().getMessage();
			return Unexpected(errorMsg);
		}
	}

	return { "Success" };
}

Result<std::string> test_Matrix4x4::ValidateLookAt_DirectX(
	const Vector4& eye,
	const Vector4& target,
	const Vector4& up)
{
	Matrix4x4 view = Matrix4x4::lookAt(eye, target, up);
	const float epsilon = 0.0001f;

	// --- 1. Базисные векторы ---
	// ВАЖНО: В вашей матрице базисные векторы идут ПО СТОЛБЦАМ, а не по строкам!
	Vector4 right = Vector4(view[0][0], view[1][0], view[2][0], 0.0f);
	Vector4 upVec = Vector4(view[0][1], view[1][1], view[2][1], 0.0f);
	Vector4 forward = Vector4(view[0][2], view[1][2], view[2][2], 0.0f);

	// Нормализация
	if (std::abs(right.length3() - 1.0f) > epsilon)
		return Unexpected(L"LookAt: Right vector not normalized");
	if (std::abs(upVec.length3() - 1.0f) > epsilon)
		return Unexpected(L"LookAt: Up vector not normalized");
	if (std::abs(forward.length3() - 1.0f) > epsilon)
		return Unexpected(L"LookAt: Forward vector not normalized");

	// Ортогональность
	if (std::abs(right.dot3(upVec)) > epsilon)
		return Unexpected(L"LookAt: Right and Up vectors not orthogonal");
	if (std::abs(right.dot3(forward)) > epsilon)
		return Unexpected(L"LookAt: Right and Forward vectors not orthogonal");
	if (std::abs(upVec.dot3(forward)) > epsilon)
		return Unexpected(L"LookAt: Up and Forward vectors not orthogonal");

	// --- 2. Проверка направления forward (DirectX LH) ---
	Vector4 expectedForward = (target - eye).normalized();
	float forwardDot = forward.dot3(expectedForward);
	if (forwardDot < 0.999f)
		return Unexpected(L"LookAt: Forward vector direction incorrect");

	// --- 3. Проверка translation ---
	float expectedTx = -right.dot3(eye);
	float expectedTy = -upVec.dot3(eye);
	float expectedTz = -forward.dot3(eye);

	if (std::abs(view[3][0] - expectedTx) > epsilon)
		return Unexpected(L"LookAt: Translation X component incorrect");
	if (std::abs(view[3][1] - expectedTy) > epsilon)
		return Unexpected(L"LookAt: Translation Y component incorrect");
	if (std::abs(view[3][2] - expectedTz) > epsilon)
		return Unexpected(L"LookAt: Translation Z component incorrect");
	if (std::abs(view[3][3] - 1.0f) > epsilon)
		return Unexpected(L"LookAt: M[3][3] should be 1.0");

	// --- 4. Проверка трансформации точки eye ---
	Vector4 transformedEye = eye * view;
	float w = transformedEye[3];
	if (std::abs(w) < epsilon)
		return Unexpected(L"LookAt: Transformed eye has invalid w component");

	float x = transformedEye[0] / w;
	float y = transformedEye[1] / w;
	float z = transformedEye[2] / w;

	if (std::abs(x) > epsilon || std::abs(y) > epsilon || std::abs(z) > epsilon)
		return Unexpected(L"LookAt: Eye position should transform to origin");

	// --- 5. Проверка трансформации точки target ---
	Vector4 transformedTarget = target * view;
	float tw = transformedTarget[3];
	if (std::abs(tw) < epsilon)
		return Unexpected(L"LookAt: Transformed target has invalid w component");

	float tz = transformedTarget[2] / tw;
	// В DirectX LH forward = target - eye, значит target впереди камеры => Z > 0
	if (tz <= epsilon)
		return Unexpected(L"LookAt: Target should be in front of camera (positive Z in LH)");

	return { "Success" };
}
#pragma endregion // DirectX

// Тестируем для Vulkan/Metal
Result<std::string> test_Matrix4x4::TestForGAPI_MetalVulkan()
{
	return { "Success" };
}