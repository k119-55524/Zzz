
#include "test_Matrix4x4.h"

import Matrix4x4;

using namespace zzz::math;
using namespace zzz::ztests;

test_Matrix4x4::test_Matrix4x4()
{
}

bool test_Matrix4x4::Initialize()
{
	return true;
}

bool test_Matrix4x4::Run()
{
	testResult = "Error determinant.";
	return true;
}

bool test_Matrix4x4::Kill()
{
	return true;
}