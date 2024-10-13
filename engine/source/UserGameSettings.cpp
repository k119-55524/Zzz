#include "pch.h"
#include "UserGameSettings.h"
#include "Platforms/Platform.h"

using namespace Zzz;

UserGameSettings::UserGameSettings(shared_ptr<IIO> _platformIO) :
	platformIO{ _platformIO },
	isFullScreen{ false },
	msWinIcoID{ c_MSIcoID }
{
}

zResult UserGameSettings::Save()
{
	zStr fullPath = platformIO->GetAppResourcesPath() + c_GameSettingsFileName;

	try
	{
		filesystem::path parentPath = filesystem::path(fullPath).parent_path();
		if (!exists(parentPath))
			create_directories(parentPath);

		stringstream buffer;
		SerializeToBuffer(buffer);

		ofstream out(fullPath, ios::binary);
		if (!out)
			throw system_error(errno, generic_category(), "Failed to open file for writing");

		out << buffer.str();
	}
	catch (const system_error& sysEx)
	{
		zStr mes = L">>>>> [GameSettings::Save()]. " + Platform::StringToWstring(sysEx.what());
		DebugOutput(mes.c_str());

		return zResult(e_ErrorCode::eFailure, mes);
	}
	catch (const bad_alloc& ba) // Обработка ошибок выделения памяти
	{
		zStr mes = L">>>>> [GameSettings::Save()]. " + zStr(ba.what(), ba.what() + strlen(ba.what()));
		DebugOutput(mes.c_str());

		return zResult(e_ErrorCode::eFailure, mes);
	}
	catch (const exception& ex) // Общая обработка остальных стандартных исключений
	{
		zStr mes = L">>>>> [GameSettings::Save()]. " + zStr(ex.what(), ex.what() + strlen(ex.what()));
		DebugOutput(mes.c_str());

		return zResult(e_ErrorCode::eFailure, mes);
	}
	catch (...) // Обработка всех других исключений
	{
		zStr mes = L">>>>> [GameSettings::Save()]. Unknown error occurred.";
		DebugOutput(mes.c_str());

		return zResult(e_ErrorCode::eFailure, mes);
	}

	return zResult();
}

zResult UserGameSettings::Load()
{
	zResult res;
	zStr fullPath = platformIO->GetAppResourcesPath() + c_GameSettingsFileName;

	try
	{
		ifstream in(fullPath, ios::binary | ios::ate);
		if (!in)
		{
			SetDefault();
			return zResult();
		}

		streamsize fileSize = in.tellg();
		in.seekg(0, ios::beg); // Возвращаемся в начало файла

		// Читаем весь файл в буфер
		vector<char> buffer(fileSize);
		if (!in.read(buffer.data(), fileSize))
			throw runtime_error("Error reading file.");

		// Создаем поток для чтения из буфера
		istringstream bufStream(string(buffer.data(), buffer.size()));
		res = DeSerializeInBuffer(bufStream);

		if (!res.isSuccess())
		{
			SetDefault();
			return zResult(e_ErrorCode::eSuccess, L">>>>> [UserGameSettings::Load()]. Error desearelize. Set default.");
		}

		if (!TestParameters().isSuccess())
		{
			SetDefault();
			return zResult(e_ErrorCode::eSuccess, L">>>>> [UserGameSettings::Load()]. Error test pframeters. Set default.");
		}
	}
	catch (const system_error& sysEx)
	{
		zStr mes = L">>>>> [GameSettings::Load()]. " + Platform::StringToWstring(sysEx.what());
		DebugOutput(mes.c_str());

		res = zResult(e_ErrorCode::eFailure, mes);
	}
	catch (const bad_alloc& ba) // Обработка ошибок выделения памяти
	{
		zStr mes = L">>>>> [GameSettings::Load()]. " + zStr(ba.what(), ba.what() + strlen(ba.what()));
		DebugOutput(mes.c_str());

		res = zResult(e_ErrorCode::eFailure, mes);
	}
	catch (const exception& ex) // Общая обработка остальных стандартных исключений
	{
		zStr mes = L">>>>> [GameSettings::Load()]. " + zStr(ex.what(), ex.what() + strlen(ex.what()));
		DebugOutput(mes.c_str());

		res = zResult(e_ErrorCode::eFailure, mes);
	}
	catch (...) // Обработка всех других исключений
	{
		zStr mes = L">>>>> [GameSettings::Load()]. Unknown error occurred.";
		DebugOutput(mes.c_str());

		res = zResult(e_ErrorCode::eFailure, mes);
	}

	return res;
}

void UserGameSettings::SetDefault() noexcept
{
	version = c_VerGameSettingsSerialize;
	winSize.SetSize(800, 600);
	isFullScreen = false;
	startScene = c_DefaultStartSceneName;
	msWinClassName = c_MSWindowsClassName;
	msWinIcoID = c_MSIcoID;
	winCaption = c_CaptionWindows;
}

void UserGameSettings::SerializeToBuffer(stringstream& buffer) const
{
	Serialize(buffer, c_HeapGameSettingsFile);
	version.Serialize(buffer);
	winSize.Serialize(buffer);
	Serialize(buffer, isFullScreen);
	Serialize(buffer, startScene);

	Serialize(buffer, msWinClassName);
	Serialize(buffer, msWinIcoID);
	Serialize(buffer, winCaption);
}

zResult UserGameSettings::DeSerializeInBuffer(istringstream& buffer)
{
	zStr pref;
	DeSerialize(buffer, pref);
	if (pref != c_HeapGameSettingsFile)
		return zResult(e_ErrorCode::eFailure, L"Unknown game settings file format.");

	zVersion ver;
	ver.DeSerialize(buffer);
	if (ver > c_VerGameSettingsSerialize)
		return zResult(e_ErrorCode::eFailure, L"The newer game settings file format is not supported.");

	version = ver;

	winSize.DeSerialize(buffer);
	DeSerialize(buffer, isFullScreen);
	DeSerialize(buffer, startScene);

	DeSerialize(buffer, msWinClassName);
	DeSerialize(buffer, msWinIcoID);
	DeSerialize(buffer, winCaption);

	return zResult();
}

zResult UserGameSettings::TestParameters() const
{
#if defined(_WINDOWS) || defined(_SERVICES) || defined(_EDITOR)
	if (winSize.width < c_MinimumWindowsWidth ||
		winSize.height < c_MinimumWindowsHeight ||
		winSize.width > c_MaximumWindowsWidth ||
		winSize.height > c_MaximumWindowsHeight)
		return zResult(e_ErrorCode::eFailure, L">>>>> [DataEngineInitialization.TestParameters()]. The requested window size does not meet the restrictions.");
#elif defined(_MACOS)
	static_assert(false, ">>>>> [DataEngineInitialization.TestParameters()]. Нет реализации для MacOS.");
#else
	static_assert(false, ">>>>> [DataEngineInitialization.TestParameters()]. Неизвестная платформа.");
#endif // defined(_WINDOWS) || defined(_SERVICES) || defined(_EDITOR)
	return zResult();
}
