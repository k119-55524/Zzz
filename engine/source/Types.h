#pragma once

#include "../pch/header.h"

namespace Zzz
{
#if defined(_WINDOWS)
	typedef uint32_t zU32;
	typedef uint64_t zU64;

	typedef int32_t zI32;
	typedef int64_t zI64;

	typedef float zF32;
	typedef double zF64;

	typedef wstring zStr;
	//typedef LPCWSTR zStrPtr;

	//typedef XMFLOAT2 zf32_2D;
#endif // defined(_WINDOWS)

	enum e_InitState : zI32
	{
		eInitNot,		// Готов к инициализации
		eInitProcess,	// Идёт процесс инициализации
		eInitOK,		// Инициализированн
		eInitError,		// Ошибка инициализации
		eTermination	// Процесс деинициализации
	};

	struct s_zEngineInit
	{
	public:
		struct WinAppSettings
		{
			union Platforms
			{

			};
		} winAppSettings;
	};
}