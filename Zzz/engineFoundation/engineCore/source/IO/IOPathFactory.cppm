
#include "pch.h"

export module IOPathFactory;

namespace zzz::engineCore
{
	namespace fs = std::filesystem;
}

export namespace zzz::engineCore
{
	export class IOPathFactory
	{
	public:
		static std::wstring GetPath_ExecutableSubfolder();

		template<typename... Args>
		static std::wstring BuildPath(Args&&... parts)
		{
			fs::path Result;
			(Result /= ... /= fs::path(std::forward<Args>(parts)));
			return Result.wstring();
		}
	};

	std::wstring IOPathFactory::GetPath_ExecutableSubfolder()
	{
		// TODO: Only for Windows
		wchar_t buffer[MAX_PATH];
		DWORD length = GetModuleFileNameW(nullptr, buffer, MAX_PATH);

		if (length == 0 || length == MAX_PATH)
			return L""; // ошибка получения пути

		fs::path exePath = fs::path(std::wstring(buffer, length));
		return exePath.parent_path().wstring();
	}
}