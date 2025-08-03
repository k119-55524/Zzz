#include "pch.h"
export module IOPathFactory;

namespace zzz::io
{
	namespace fs = std::filesystem;

	static const std::wstring swSettingFolderName = L"appdata";
	//static const std::wstring swSettingFileName = L"swui.zaml";
	static const std::wstring swRCFolderName = L"rc";
}

export namespace zzz::io
{
	export class IOPathFactory
	{
	public:
		static std::wstring GetPath_ExecutableSubfolder();

		template<typename... Args>
		static std::wstring BuildPath(Args&&... parts)
		{
			fs::path result;
			(result /= ... /= fs::path(std::forward<Args>(parts)));
			return result.wstring();
		}

		static std::wstring GetPath_swRC(std::wstring fileName)
		{
			return BuildPath(GetPath_ExecutableSubfolder(), swSettingFolderName, swRCFolderName, fileName);
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
} // namespace zzz::io