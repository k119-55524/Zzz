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

#pragma region Объявление структур для инициализации движка пользователем

	class IInitWinData
	{
	public:
		virtual ~IInitWinData() = default;
	};

#ifdef _WINDOWS
	class InitMSWindowsData : public IInitWinData
	{
	public:
		InitMSWindowsData() = delete;
		explicit InitMSWindowsData(const zStr& winClassName) :
			WinClassName{ winClassName }
		{
			if (WinClassName.empty())
				throw std::runtime_error(">>>>> [MSWindows::InitMSWindowsData()]. WinClassName.empty() == true.");
		}

		inline const wchar_t* GetClassName() const { return WinClassName.c_str(); };

	private:
		zStr WinClassName;
	};
#endif // _WINDOWS

	class s_zEngineInit
	{
	public:
		s_zEngineInit() = delete;
		explicit s_zEngineInit(const IInitWinData& _initWinData) :
			initWinData{ nullptr }
		{
#ifdef _WINDOWS
			const auto msData = dynamic_cast<const InitMSWindowsData*>(&_initWinData);
			if (!msData)
				throw invalid_argument(">>>>> [s_zEngineInit::s_zEngineInit(const IInitWinData& _initWinData)]. Каст dynamic_cast<const InitMSWindowsData*>(&_initWinData) не удался.");

			initWinData = make_shared<InitMSWindowsData>(*msData);
#elif defined(_MACOS)
			static_assert(false, ">>>>> [class s_zEngineInit]. Нет реализации для MacOS.");
#else
			static_assert(false, ">>>>> [class s_zEngineInit]. Неизвестная платформа.");
#endif
		}

		inline const shared_ptr<IInitWinData> GetWinData() const { return initWinData; };

	private:
		shared_ptr<IInitWinData> initWinData;
	};

#pragma endregion Объявление структур для инициализации движка пользователем
}