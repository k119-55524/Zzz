#pragma once

#include "..\..\source\BaseTest.h"

import Math;
import Result;
import Matrix4x4;

using namespace zzz;
using namespace zzz::math;
using namespace zzz::ztests;

namespace zzz::ztests
{
	class test_Matrix4x4 : public BaseTest<test_Matrix4x4>
	{
	public:
		test_Matrix4x4();

		bool Initialize() override;
		Result<std::string> Run() override;
		bool Kill() override;

	private:
		// Тесты для DirectX
		Result<std::string> TestForGAPI_DirectX();
		Result<std::string> ValidateMatrixVectorMultiply();
		Result<std::string> ValidateMatrixVectorMultiply(const Matrix4x4& matrix, const Vector4& vector, const Vector4& expectedResult, const wchar_t* testName);
		Result<std::string> ValidatePerspective_DirectX();
		Result<std::string> ValidatePerspective_DirectX(float fovY, float aspect, float nearZ, float farZ);
		Result<std::string> ValidateLookAt_DirectX();
		Result<std::string> ValidateLookAt_DirectX(const Vector3& eye, const Vector3& target, const Vector3& up);

		// Тесты для Metal/Vulkan
		Result<std::string> TestForGAPI_MetalVulkan();
	};
}