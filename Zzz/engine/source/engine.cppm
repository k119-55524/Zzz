
#include "pch.h"
export module engine;

import result;
import ISuperWidget;
import StringConverters;

#if defined(_WIN64)
import SuperWidget_MSWindows;
#else
#error ">>>>> [Compile error]. This branch requires implementation for the current platform"
#endif

using namespace std;
using namespace zzz;
using namespace zzz::result;
using namespace zzz::platforms;

namespace zzz
{
#if defined(_WIN64)
	typedef SuperWidget_MSWindows SuperWidget;
#else
#error ">>>>> [Compile error]. This branch requires implementation for the current platform"
#endif

	enum eEngineState : zU32
	{
		eInitNot,	// Готов к инициализации
		eInitOK,	// Инициализированн
		eRunning,	// Идёт процесс работы
	};
}

export namespace zzz
{
	export class engine
	{
	public:
		engine();
		~engine();

		zResult<> Initialize() noexcept;
		zResult<> Run() noexcept;

	private:
		eEngineState initState;
		mutex initMutex;
		mutex runMutex;

		SuperWidget superWidget;

		void Reset() noexcept;
		void ResetDuringInitialize() noexcept;
		void ResetDuringRun() noexcept;
		void OnResizeWindow(const zSize2D<>& size, e_TypeWinAppResize resizeType);
	};

	engine::engine() :
		superWidget{ bind(&engine::OnResizeWindow, this, placeholders::_1, placeholders::_2) }
		, initState{ eEngineState::eInitNot }
	{
	}

	engine::~engine()
	{
		Reset();
	}

	void engine::Reset() noexcept
	{
		initState = eEngineState::eInitNot;
	}

	zResult<> engine::Initialize() noexcept
	{
		lock_guard<mutex> lock(initMutex);
		if (initState != eEngineState::eInitNot)
			return Unexpected(eResult::failure, L">>>>> [engine::initialize()]. Re-initialization is not allowed.");

		try
		{
			auto res = superWidget.Initialize(nullptr)
				.and_then([&](auto result) { initState = eEngineState::eInitOK;;})
				.or_else([&](auto error) { ResetDuringInitialize(); return error; });
			
			return res;
		}
		catch (const std::exception& e)
		{
			std::wstring err;
			zResult<std::wstring> res = string_to_wstring(e.what())
				.and_then([&err](const std::wstring& wstr) { err = L">>>>> [engine::initialize()]. Exception: " + wstr + L"\n";  std::wcerr << err; })
				.or_else([&err](const Unexpected& error) { err = L">>>>> #0 [engine::initialize()]. Unknown exception occurred\n";  std::wcerr << err; });

			ResetDuringInitialize();
			return Unexpected(eResult::exeption, err);
		}
		catch (...)
		{
			std::wstring err = L">>>>> #1 [wWinMain( ... )]. Unknown exception occurred\n";
			std::wcerr << err;

			ResetDuringInitialize();
			return Unexpected(eResult::exeption, err);
		}
	}

	void engine::ResetDuringInitialize() noexcept
	{
		Reset();
	}

	zResult<> engine::Run() noexcept
	{
		lock_guard<mutex> lock(runMutex);
		if (initState != eEngineState::eInitOK)
			return Unexpected(eResult::failure, L">>>>> [engine::go()]. Engine is not initialized.");

		if (initState == eEngineState::eRunning)
			return Unexpected(eResult::failure, L">>>>> [engine::go()]. Engine is already running.");

		initState = eEngineState::eRunning;

		try
		{
			return eResult::success;
		}
		catch (const std::exception& e)
		{
			std::wstring err;
			zResult<std::wstring> res = string_to_wstring(e.what())
				.and_then([&err](const std::wstring& wstr) { err = L">>>>> [engine::go()]. Exception: " + wstr + L"\n";  std::wcerr << err; })
				.or_else([&err](const Unexpected& error) { err = L">>>>> #0 [engine::go()]. Unknown exception occurred" + std::wstring(L"\n"); std::wcerr << err; });

			ResetDuringRun();
			return Unexpected(eResult::exeption, err);
		}
		catch (...)
		{
			const std::wstring err = L">>>>> #1 [engine::go()]. Unknown exception occurred";
			std::wcerr << err << std::endl;

			ResetDuringRun();
			return Unexpected(eResult::exeption, err);
		}
	}

	void engine::ResetDuringRun() noexcept
	{
		Reset();
	}

	void engine::OnResizeWindow(const zSize2D<>& size, e_TypeWinAppResize resizeType)
	{

	}
}