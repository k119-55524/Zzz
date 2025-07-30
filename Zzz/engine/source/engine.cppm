
#include "pch.h"
export module engine;

import result;
import swSettings;
import ISuperWidget;
import IOPathFactory;
import StringConverters;

#if defined(_WIN64)
import swMSWin;
#else
#error ">>>>> [Compile error]. This branch requires implementation for the current platform"
#endif

using namespace zzz;
using namespace zzz::io;
using namespace zzz::result;
using namespace zzz::platforms;

namespace zzz
{
#if defined(_WIN64)
	typedef swMSWin SuperWidget;
#else
#error ">>>>> [Compile error]. This branch requires implementation for the current platform"
#endif

	enum eEngineState : zU32
	{
		eInitNot,	// Готов к инициализации
		eInitOK,	// Инициализированн
		eRunning,	// Идёт процесс работы
	};

	const std::wstring swSetFolderName = L"appdata";
	const std::wstring swSetFileName = L"mui.zaml";
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
		std::mutex stateMutex;

		swSettings settings;
		SuperWidget superWidget;

		void Reset() noexcept;
		void OnResizeWindow(const zSize2D<>& size, e_TypeWinAppResize resizeType);
	};

	engine::engine() :
		superWidget{ settings, std::bind(&engine::OnResizeWindow, this, std::placeholders::_1, std::placeholders::_2) }
		, initState{ eEngineState::eInitNot }
	{
	}

	engine::~engine()
	{
		Reset();
	}

	void engine::Reset() noexcept
	{
		settings.Reset();
		initState = eEngineState::eInitNot;
	}

	zResult<> engine::Initialize() noexcept
	{
		std::lock_guard<std::mutex> lock(stateMutex);
		if (initState != eEngineState::eInitNot)
			return Unexpected(eResult::failure, L">>>>> [engine::initialize()]. Re-initialization is not allowed.");

		std::wstring err;
		try
		{
			auto res = settings.Get(IOPathFactory::BuildPath(IOPathFactory::GetPath_ExecutableSubfolder(), swSetFolderName, swSetFileName))
				.and_then([&]() { return superWidget.Initialize(); })
				.and_then([&]() { initState = eEngineState::eInitOK; return zResult<>(); })
				.or_else([&](auto error) { Reset(); return zResult<>(error); });

			return res;
		}
		catch (const std::exception& e)
		{
			string_to_wstring(e.what())
				.and_then([&err](const std::wstring& wstr) { err = L">>>>> [engine::initialize()]. Exception: " + wstr + L"\n"; })
				.or_else([&err](const Unexpected& error) { err = L">>>>> #0 [engine::initialize()]. Unknown exception occurred\n"; });
		}
		catch (...)
		{
			err = L">>>>> #1 [wWinMain( ... )]. Unknown exception occurred\n";
		}

		Reset();
		return Unexpected(eResult::exception, err);
	}

	zResult<> engine::Run() noexcept
	{
		std::lock_guard<std::mutex> lock(stateMutex);
		if (initState != eEngineState::eInitOK)
			return Unexpected(eResult::failure, L">>>>> [engine::go()]. Engine is not initialized.");

		if (initState == eEngineState::eRunning)
			return Unexpected(eResult::failure, L">>>>> [engine::go()]. Engine is already running.");

		initState = eEngineState::eRunning;
		std::wstring err;

		try
		{
			return {};
		}
		catch (const std::exception& e)
		{
			string_to_wstring(e.what())
				.and_then([&err](const std::wstring& wstr) { err = L">>>>> [engine::go()]. Exception: " + wstr + L"\n"; })
				.or_else([&err](const Unexpected& error) { err = L">>>>> #0 [engine::go()]. Unknown exception occurred" + std::wstring(L"\n"); });
		}
		catch (...)
		{
			err = L">>>>> #1 [engine::go()]. Unknown exception occurred";
		}

		Reset();
		return Unexpected(eResult::exception, err);
	}

	void engine::OnResizeWindow(const zSize2D<>& size, e_TypeWinAppResize resizeType)
	{

	}
}