#pragma once

#include "Constants.h"

namespace Zzz
{
	enum e_InitState : zI32
	{
		eInitNot,		// Готов к инициализации
		eInitProcess,	// Идёт процесс инициализации
		eInitOK,		// Инициализированн
		eInitError,		// Ошибка инициализации
		eTermination	// Процесс деинициализации
	};

	struct zSize
	{
		zU64 width{ 0 };
		zU64 height{ 0 };
	};

#pragma region Объявление структур для инициализации движка пользователем

#if defined(_WINDOWS) || defined(_SERVICES) || defined(_EDITOR)
	class InitWinData
	{
		friend class DataEngineInitialization;

	public:
		InitWinData() = delete;
		explicit InitWinData(const zStr& _winClassName, const zStr& _winCaption, zU64 _ICO_ID) :
			winClassName{ _winClassName },
			winCaption{ _winCaption },
			ICO_ID{ _ICO_ID }
		{
			if (winClassName.empty())
				throw std::runtime_error(">>>>> [InitWindowsData::InitWindowsData()]. WinClassName.empty() == true.");
		}
		inline const zStr GetWinCaption() const { return winCaption; };
		inline const zStr GetWinClassName() const { return winClassName; };
		inline zU64 GetIcoID() const { return ICO_ID; };

	private:
		zStr winClassName;
		zStr winCaption;
		zU64 ICO_ID;

		void TestParameters()
		{
			if (winClassName[0] == L'\0')
				throw invalid_argument(">>>>> [InitWindowsData.TestParameters()]. Incorrect winClassName.");
		}
	};
#elif defined(_MACOS)
	static_assert(false, ">>>>> [class InitWindowsData]. Нет реализации для MacOS.");
#else
	static_assert(false, ">>>>> [class InitWindowsData]. Неизвестная платформа.");
#endif // _WINDOWS

	class DataEngineInitialization
	{
		friend class zEngine;

	public:
		DataEngineInitialization() = delete;

#if defined(_WINDOWS)  || defined(_SERVICES) || defined(_EDITOR)
		explicit DataEngineInitialization(const InitWinData& _initWinData, const zSize& _winSize, bool _fullScreen = false) :
			initWinData{ make_shared<InitWinData>(_initWinData) },
			winSize{ _winSize },
			fullScreen{ _fullScreen }
		{
		}
#elif defined(_MACOS)
		static_assert(false, ">>>>> [DataEngineInitialization.DataEngineInitialization()]. Нет реализации для MacOS.");
#else
		static_assert(false, ">>>>> [DataEngineInitialization.DataEngineInitialization()");
#endif // _WINDOWS

		inline const shared_ptr<InitWinData> GetWinData() const { return initWinData; };
		inline const zSize& GetWinSize() const { return winSize; };

	private:
		shared_ptr<InitWinData> initWinData;
		zSize winSize;
		bool fullScreen;

		void TestParameters() const
		{
#if defined(_WINDOWS) || defined(_SERVICES) || defined(_EDITOR)
			if (winSize.width < c_MinimumWindowsWidth ||
				winSize.height < c_MinimumWindowsHeight ||
				winSize.width > c_MaximumWindowsWidth ||
				winSize.height > c_MaximumWindowsHeight)
				throw invalid_argument(">>>>> [DataEngineInitialization.TestParameters()]. The requested window size does not meet the restrictions.");
#elif defined(_MACOS)
			static_assert(false, ">>>>> [DataEngineInitialization.TestParameters()]. Нет реализации для MacOS.");
#else
			static_assert(false, ">>>>> [DataEngineInitialization.TestParameters()]. Неизвестная платформа.");
#endif //_WINDOWS

			initWinData->TestParameters();
		}
	};

#pragma endregion Объявление структур для инициализации движка пользователем
}