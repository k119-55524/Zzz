
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
			Matrix4x4::translation(1.0f, 2.0f, 3.0f) * Matrix4x4::scale(2.0f),
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
					1.0f, 2.0f, 3.0f, 1.0f
					),
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

	Vector4 result = matrix * vector;

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
	Vector4 projNear = proj * nearPoint;
	// После perspective divide z должен быть 0 (near plane -> 0 в DirectX)
	float zNear = projNear[2] / projNear[3];
	if (std::abs(zNear - 0.0f) > epsilon)
		return Unexpected(L"DirectX Perspective: Near plane Z mapping incorrect");

	// Точка на far plane
	Vector4 farPoint(0.0f, 0.0f, farZ, 1.0f);
	Vector4 projFar = proj * farPoint;
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
		{ Vector4(0.0f, 0.0f, 5.0f, 1.0f),  Vector4(0.0f, 0.0f, 0.0f, 1.0f), Vector4(0.0f, 1.0f, 0.0f, 0.0f), L"Standard view along -Z axis from (0,0,5) to origin, up Y" },
		{ Vector4(5.0f, 0.0f, 0.0f, 1.0f),  Vector4(0.0f, 0.0f, 0.0f, 1.0f), Vector4(0.0f, 1.0f, 0.0f, 0.0f), L"View along -X axis from (5,0,0) to origin, up Y" },
		{ Vector4(0.0f, 5.0f, 0.0f, 1.0f),  Vector4(0.0f, 0.0f, 0.0f, 1.0f), Vector4(0.0f, 0.0f, 1.0f, 0.0f), L"View along -Y axis from (0,5,0) to origin, up Z (adjusted)" },
		{ Vector4(1.0f, 2.0f, 3.0f, 1.0f),  Vector4(4.0f, 5.0f, 6.0f, 1.0f), Vector4(0.0f, 1.0f, 0.0f, 0.0f), L"Arbitrary view from (1,2,3) to (4,5,6), up Y" },
		{ Vector4(0.0f, 0.0f, 10.0f, 1.0f), Vector4(0.0f, 0.0f, 0.0f, 1.0f), Vector4(1.0f, 0.0f, 0.0f, 0.0f), L"View along -Z from (0,0,10) to origin, up X (tilted)" }
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

	// 1. Проверка ортонормальности базисных векторов
	// Извлекаем базисные векторы из матрицы (первые 3 строки, первые 3 столбца)
	Vector4 right(view[0][0], view[0][1], view[0][2], 0.0f);
	Vector4 upVec(view[1][0], view[1][1], view[1][2], 0.0f);
	Vector4 forward(view[2][0], view[2][1], view[2][2], 0.0f);

	// Проверка нормализации (длина каждого вектора должна быть ~1)
	float rightLen = std::sqrt(right[0] * right[0] + right[1] * right[1] + right[2] * right[2]);
	float upLen = std::sqrt(upVec[0] * upVec[0] + upVec[1] * upVec[1] + upVec[2] * upVec[2]);
	float forwardLen = std::sqrt(forward[0] * forward[0] + forward[1] * forward[1] + forward[2] * forward[2]);

	if (std::abs(rightLen - 1.0f) > epsilon)
		return Unexpected(L"LookAt: Right vector not normalized");

	if (std::abs(upLen - 1.0f) > epsilon)
		return Unexpected(L"LookAt: Up vector not normalized");

	if (std::abs(forwardLen - 1.0f) > epsilon)
		return Unexpected(L"LookAt: Forward vector not normalized");

	// Проверка ортогональности (скалярное произведение должно быть ~0)
	float rightDotUp = right[0] * upVec[0] + right[1] * upVec[1] + right[2] * upVec[2];
	float rightDotForward = right[0] * forward[0] + right[1] * forward[1] + right[2] * forward[2];
	float upDotForward = upVec[0] * forward[0] + upVec[1] * forward[1] + upVec[2] * forward[2];

	if (std::abs(rightDotUp) > epsilon)
		return Unexpected(L"LookAt: Right and Up vectors not orthogonal");

	if (std::abs(rightDotForward) > epsilon)
		return Unexpected(L"LookAt: Right and Forward vectors not orthogonal");

	if (std::abs(upDotForward) > epsilon)
		return Unexpected(L"LookAt: Up and Forward vectors not orthogonal");

	// 2. Проверка направления forward вектора
	// Forward должен указывать от eye к target (для left-handed DirectX)
	Vector4 expectedForward = (target - eye).normalized();

	float forwardDot = forward[0] * expectedForward[0] +
		forward[1] * expectedForward[1] +
		forward[2] * expectedForward[2];

	// Косинус угла между векторами должен быть близок к 1 (векторы сонаправлены)
	if (forwardDot < 0.999f)
		return Unexpected(L"LookAt: Forward vector direction incorrect");

	// 3. Проверка последней строки (перемещение)
	// Последняя строка должна содержать -dot(basis, eye) для каждой оси
	float expectedTx = -(right[0] * eye[0] + right[1] * eye[1] + right[2] * eye[2]);
	float expectedTy = -(upVec[0] * eye[0] + upVec[1] * eye[1] + upVec[2] * eye[2]);
	float expectedTz = -(forward[0] * eye[0] + forward[1] * eye[1] + forward[2] * eye[2]);

	if (std::abs(view[3][0] - expectedTx) > epsilon)
		return Unexpected(L"LookAt: Translation X component incorrect");

	if (std::abs(view[3][1] - expectedTy) > epsilon)
		return Unexpected(L"LookAt: Translation Y component incorrect");

	if (std::abs(view[3][2] - expectedTz) > epsilon)
		return Unexpected(L"LookAt: Translation Z component incorrect");

	// 4. Проверка что последний элемент [3][3] = 1
	if (std::abs(view[3][3] - 1.0f) > epsilon)
		return Unexpected(L"LookAt: M[3][3] should be 1.0");

	// 5. Проверка трансформации точки eye -> должна попасть в начало координат view space
	Vector4 transformedEye = view * eye;

	// Для однородных координат проверяем с учетом w-компоненты
	float w = transformedEye[3];
	if (std::abs(w) < epsilon)
		return Unexpected(L"LookAt: Transformed eye has invalid w component");

	// Нормализуем к декартовым координатам (perspective divide)
	float x = transformedEye[0] / w;
	float y = transformedEye[1] / w;
	float z = transformedEye[2] / w;

	if (std::abs(x) > epsilon ||
		std::abs(y) > epsilon ||
		std::abs(z) > epsilon)
		return Unexpected(L"LookAt: Eye position should transform to origin");

	// 6. Проверка трансформации точки target -> должна быть на положительной оси Z
	Vector4 transformedTarget = view * target;

	// Также нормализуем target
	float tw = transformedTarget[3];
	if (std::abs(tw) < epsilon)
		return Unexpected(L"LookAt: Transformed target has invalid w component");

	float tz = transformedTarget[2] / tw;

	if (tz <= epsilon)
		return Unexpected(L"LookAt: Target should be in front of camera (positive Z in left-handed)");

	return { "Success" };
}
#pragma endregion // DirectX

// Тестируем для Vulkan/Metal
Result<std::string> test_Matrix4x4::TestForGAPI_MetalVulkan()
{
	return { "Success" };
}