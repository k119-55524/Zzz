#include "Path.h"

namespace zzz::io
{
	std::filesystem::path Path::GetExecutableDirectory()
	{
#if defined(_WIN32)
		wchar_t buffer[MAX_PATH];
		GetModuleFileNameW(nullptr, buffer, MAX_PATH);

		return std::filesystem::path(buffer).parent_path();
#elif defined(__APPLE__)
		uint32_t size = 0;
		_NSGetExecutablePath(nullptr, &size);

		std::string path(size, '\0');
		_NSGetExecutablePath(path.data(), &size);

		return std::filesystem::canonical(path).parent_path();
#elif defined(__linux__)
		char buffer[PATH_MAX];
		ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);

		if (len == -1)
		{
			throw std::runtime_error("Failed to get executable path.");
		}

		buffer[len] = '\0';

		return std::filesystem::canonical(buffer).parent_path();
#else
#error Unsupported platform
#endif
	}

	char Path::GetSeparator()
	{
		return std::filesystem::path::preferred_separator;
	}

	bool Path::IsPathStringValid(std::string_view path)
	{
		if (path.empty())
		{
			return false;
		}

		for (char c : path)
		{
			if (c == '\0')
			{
				return false;
			}
		}

#if defined(_WIN32)
		constexpr std::string_view invalidChars = "<>:\"|?*";

		for (size_t i = 0; i < path.size(); ++i)
		{
			const char c = path[i];

			// Разрешаем ':' только после буквы диска
			if (c == ':')
			{
				if (!(i == 1 && std::isalpha(static_cast<unsigned char>(path[0]))))
				{
					return false;
				}

				continue;
			}

			if (invalidChars.find(c) != std::string_view::npos)
			{
				return false;
			}
		}
#endif
		return true;
	}

	bool Path::IsValidPath(std::string_view path)
	{
		if (path.empty())
		{
			return false;
		}

#if defined(_WIN32)
		constexpr std::string_view invalidChars = "<>:\"|?*";
		for (char c : path)
		{
			if (invalidChars.contains(c))
			{
				return false;
			}
		}
#else
		for (char c : path)
		{
			if (c == '\0')
			{
				return false;
			}
		}
#endif

		try
		{
			std::filesystem::path p(path);

			return !p.empty();
		}
		catch (...)
		{
			return false;
		}
	}
}
