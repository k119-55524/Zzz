#include "pch.h"
#include "Scene.h"
#include "../Constants.h"

using namespace Zzz;
using namespace Zzz::Core;
using namespace Zzz::Platforms;

Scene::Scene() :
	version{ c_VerScenesSerialize },
	typeClear{ e_TypeClear::Color }
{
	clearColor.Default();
}

void Scene::Update()
{
}

#pragma region Serialize/Deserialize

zResult Scene::Save(const zStr& filename)
{
	try
	{
		filesystem::path parentPath = filesystem::path(filename).parent_path();
		if (!exists(parentPath))
			create_directories(parentPath);

		stringstream buffer;
		SerializeToBuffer(buffer);

		ofstream out(filename, ios::binary);
		if (!out)
			throw system_error(errno, generic_category(), "Failed to open file for writing");

		out << buffer.str();
	}
	catch (const system_error& sysEx)
	{
		zStr mes = L">>>>> [Scene::Save(" + filename + L")]. " + StringToWstring(sysEx.what());
		DebugOutput(mes.c_str());

		return zResult(e_ErrorCode::eFailure, mes);
	}
	catch (const bad_alloc& ba) // Обработка ошибок выделения памяти
	{
		zStr mes = L">>>>> [Scene::Save(" + filename + L")]. " + zStr(ba.what(), ba.what() + strlen(ba.what()));
		DebugOutput(mes.c_str());

		return zResult(e_ErrorCode::eFailure, mes);
	}
	catch (const exception& ex) // Общая обработка остальных стандартных исключений
	{
		zStr mes = L">>>>> [Scene::Save(" + filename + L")]. " + zStr(ex.what(), ex.what() + strlen(ex.what()));
		DebugOutput(mes.c_str());

		return zResult(e_ErrorCode::eFailure, mes);
	}
	catch ( ... ) // Обработка всех других исключений
	{
		zStr mes = L">>>>> [Scene::Save(" + filename + L")]. Unknown error occurred.";
		DebugOutput(mes.c_str());

		return zResult(e_ErrorCode::eFailure, mes);
	}

	return zResult();
}

zResult Scene::Load(const zStr& filename)
{
	zResult res;

	try
	{
		ifstream in(filename, ios::binary | ios::ate);
		if (!in)
			throw runtime_error("Error opening file.");

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
		zStr mes = L">>>>> [Scene::Load(" + filename + L")]. " + StringToWstring(sysEx.what());
		DebugOutput(mes.c_str());

		return zResult(e_ErrorCode::eFailure, mes);
	}
	catch (const bad_alloc& ba) // Обработка ошибок выделения памяти
	{
		zStr mes = L">>>>> [Scene::Load(" + filename + L")]. " + zStr(ba.what(), ba.what() + strlen(ba.what()));
		DebugOutput(mes.c_str());

		return zResult(e_ErrorCode::eFailure, mes);
	}
	catch (const exception& ex) // Общая обработка остальных стандартных исключений
	{
		zStr mes = L">>>>> [Scene::Load(" + filename + L")]. " + zStr(ex.what(), ex.what() + strlen(ex.what()));
		DebugOutput(mes.c_str());

		return zResult(e_ErrorCode::eFailure, mes);
	}
	catch (...) // Обработка всех других исключений
	{
		zStr mes = L">>>>> [Scene::Load(" + filename + L")]. Unknown error occurred.";
		DebugOutput(mes.c_str());

		return zResult(e_ErrorCode::eFailure, mes);
	}

	return res;
}

void Scene::SerializeToBuffer(stringstream& buffer) const
{
	Serialize(buffer, c_HeapSceneFile);
	version.Serialize(buffer);
	Serialize(buffer, typeClear);
	clearColor.Serialize(buffer);
}

zResult Scene::DeSerializeInBuffer(istringstream& buffer)
{
	zStr pref;
	DeSerialize(buffer, pref);
	if (pref != c_HeapSceneFile)
		return zResult(e_ErrorCode::eFailure, L"Unknown scene file format.");

	zVersion ver;
	ver.DeSerialize(buffer);
	if (ver > version)
		return zResult(e_ErrorCode::eFailure, L"The newer scene file format is not supported.");

	version = ver;

	DeSerialize(buffer, typeClear);
	clearColor.DeSerialize(buffer);

	return zResult();
}

zStr Scene::StringToWstring(const string& str)
{
	size_t length = str.length();
	wstring wstr(length + 1, L'\0');

	size_t convertedChars = 0;
	errno_t err = mbstowcs_s(&convertedChars, &wstr[0], wstr.size(), str.c_str(), length);

	if (err != 0)
		throw runtime_error(">>>>> [StringToWstring( ... )]. Conversion failed.");

	// Уменьшаем размер wstring до фактической длины
	wstr.resize(convertedChars);

	return wstr;
}

#pragma endregion
