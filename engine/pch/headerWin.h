#pragma once

#if defined(_WINDOWS)
#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN // ��������� ����� ������������ ���������� �� ���������� Windows

// ����� ���������� Windows
#include <windows.h>

// ����� ���������� ����� ���������� C
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#endif