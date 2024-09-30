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
#pragma region �������������
		InitializationFactory initFactory;
		mutex initMutex;
		e_InitState initState;
#pragma endregion // �������������

		shared_ptr<Platform> platform;
		unique_ptr<GAPIBase> gapi;
	};
}