#pragma once


#include "InitializationFactory.h"
#include "source/Platforms/Platform.h"
#include "source/Platforms/WinApplication/WinAppBase.h"

using namespace Zzz::Platforms;

namespace Zzz
{
	class zEngine
	{
	public:
		zEngine();

		zResult Initialize(const s_zEngineInit* const initData);

	private:
#pragma region Инициализация
		enum e_EngineState : zI64
		{
			eInitNot,		// Готов к инициализации
			eInitProcess,	// Идёт процесс инициализации
			eInitOK,		// Инициализированн
			eTermination	// Процесс деинициализации
		};

		InitializationFactory initFactory;
		mutex initMutex;
		e_EngineState engineState;
#pragma endregion // Инициализация

		unique_ptr<Platform> platform;
		unique_ptr<GAPIBase> gapi;
	};
}