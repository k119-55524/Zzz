#include "pch.h"
#include "GameSettings.h"
#include "Platforms/Platform.h"

using namespace Zzz;

GameSettings::GameSettings(shared_ptr<IIO> _platformIO) :
	platformIO{ _platformIO }
{
	SetDefault();
}

zResult Zzz::GameSettings::Save()
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

zResult GameSettings::Load()
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

	if (!res.isSuccess())
		SetDefault();

	return res;
}

void GameSettings::SetDefault() const noexcept
{

}

void GameSettings::SerializeToBuffer(stringstream& buffer) const
{
	//Serialize(buffer, c_HeapSceneFile);
	//version.Serialize(buffer);
	//Serialize(buffer, typeClear);
	//clearColor.Serialize(buffer);
}

zResult GameSettings::DeSerializeInBuffer(istringstream& buffer)
{
	//zStr pref;
	//DeSerialize(buffer, pref);
	//if (pref != c_HeapSceneFile)
	//	return zResult(e_ErrorCode::eFailure, L"Unknown scene file format.");

	//zVersion ver;
	//ver.DeSerialize(buffer);
	//if (ver > version)
	//	return zResult(e_ErrorCode::eFailure, L"The newer scene file format is not supported.");

	//version = ver;

	//DeSerialize(buffer, typeClear);
	//clearColor.DeSerialize(buffer);

	return zResult();
}