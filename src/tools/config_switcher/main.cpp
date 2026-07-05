#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <print>
#include <ranges>
#include <string>
#include <string_view>
#include <unordered_set>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

#include "json.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

// ─────────────────────────────────────────────────────────────────────────────
// Терминал в "сыром" (raw) режиме
// ─────────────────────────────────────────────────────────────────────────────

#ifndef _WIN32
class TerminalRawMode
{
public:
	TerminalRawMode()
	{
		tcgetattr(STDIN_FILENO, &m_old);

		termios raw = m_old;
		raw.c_lflag &= ~static_cast<unsigned>(ICANON | ECHO);

		tcsetattr(STDIN_FILENO, TCSANOW, &raw);
	}

	~TerminalRawMode()
	{
		tcsetattr(STDIN_FILENO, TCSANOW, &m_old);
	}

private:
	termios m_old{};
};
#endif

// ─────────────────────────────────────────────────────────────────────────────
// Вспомогательные функции
// ─────────────────────────────────────────────────────────────────────────────

static void Pause()
{
	std::print("Нажмите любую клавишу...");
	std::fflush(stdout);

#ifdef _WIN32
	_getch();
#else
	TerminalRawMode raw;
	tcflush(STDIN_FILENO, TCIFLUSH);
	getchar();
#endif

	std::println("");
}

[[noreturn]]
static void Fail(std::string_view msg)
{
	std::println(stderr, "Ошибка: {}", msg);
	Pause();
	std::exit(1);
}

static fs::path FindProjectRoot()
{
	fs::path current = fs::current_path();

	while (!current.empty())
	{
		if (fs::exists(current / "build_configs" / "profiles.json"))
			return current;

		current = current.parent_path();
	}

	Fail("Не удалось найти build_configs/profiles.json");
}

// ─────────────────────────────────────────────────────────────────────────────
// Ввод с клавиатуры
// ─────────────────────────────────────────────────────────────────────────────

// Enter     -> подтвердить ввод
// Esc       -> отменить
// Цифры     -> добавить к вводу
// Backspace -> удалить последнюю цифру

static std::string ReadDigits()
{
	std::string buffer;

#ifdef _WIN32

	while (true)
	{
		int ch = _getch();

		if (ch == '\r')
		{
			std::println("");
			return buffer;
		}

		if (ch == 27)
		{
			std::println("");
			return "";
		}

		if ((ch == '\b') && !buffer.empty())
		{
			buffer.pop_back();
			std::print("\b \b");
			std::fflush(stdout);
			continue;
		}

		if (std::isdigit(static_cast<unsigned char>(ch)))
		{
			buffer.push_back(static_cast<char>(ch));

			std::print("{}", static_cast<char>(ch));
			std::fflush(stdout);
		}
	}

#else

	TerminalRawMode raw;

	while (true)
	{
		int ch = getchar();

		if (ch == '\n' || ch == '\r')
		{
			std::println("");
			return buffer;
		}

		if (ch == 27)
		{
			std::println("");
			return "";
		}

		if ((ch == 127 || ch == '\b') && !buffer.empty())
		{
			buffer.pop_back();
			std::print("\b \b");
			std::fflush(stdout);
			continue;
		}

		if (std::isdigit(static_cast<unsigned char>(ch)))
		{
			buffer.push_back(static_cast<char>(ch));

			std::print("{}", static_cast<char>(ch));
			std::fflush(stdout);
		}
	}

#endif
}

// ─────────────────────────────────────────────────────────────────────────────
// Генерация current.cmake
// ─────────────────────────────────────────────────────────────────────────────

static void WriteCmake(
	const fs::path& path,
	const json& data,
	const json& config)
{
	std::ofstream file(path);

	if (!file)
		Fail("Не удалось открыть current.cmake");

	file << "# Auto-generated file.\n";
	file << "# Configuration: "
		<< config["name"].get<std::string>()
		<< "\n\n";

	std::unordered_set<std::string> activeDefines;

	for (const auto& item : config["activeDefines"])
	{
		activeDefines.insert(item.get<std::string>());
	}

	for (const auto& def : data["defines"])
	{
		if (def.value("isArchived", false))
			continue;

		if (!def.value("isCMake", false))
			continue;

		const std::string name = def.at("name").get<std::string>();
		const bool enabled = activeDefines.contains(name);

		file << "set(" << name << " " << (enabled ? "ON" : "OFF") << ")\n";
	}

	file << "\nadd_compile_definitions(\n";

	for (const auto& def : data["defines"])
	{
		if (def.value("isArchived", false))
			continue;

		if (def.value("isCMake", false))
			continue;

		const std::string name = def.at("name").get<std::string>();
		const bool enabled = activeDefines.contains(name);

		file << "    " << name << '=' << (enabled ? "1" : "0") << '\n';
	}

	file << ")\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// Точка входа
// ─────────────────────────────────────────────────────────────────────────────

int main()
{
#ifdef _WIN32
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
#endif

	const fs::path root =
		FindProjectRoot();

	const fs::path profilesPath =
		root / "build_configs" / "profiles.json";

	const fs::path cmakePath =
		root / "build_configs" / "current.cmake";

	const fs::path buildDir =
		root / "build";

	json data;

	try
	{
		std::ifstream file(profilesPath);

		if (!file)
			Fail("Не удалось открыть profiles.json");

		data = json::parse(file);
	}
	catch (const json::exception& e)
	{
		Fail(std::string("Ошибка чтения profiles.json: ") + e.what());
	}

	if (!data.contains("defines") ||
		!data["defines"].is_array())
	{
		Fail("profiles.json: отсутствует массив defines");
	}

	if (!data.contains("configurations") ||
		!data["configurations"].is_array())
	{
		Fail("profiles.json: отсутствует массив configurations");
	}

	const auto& configs =
		data["configurations"];

	if (configs.empty())
		Fail("Нет конфигураций.");

	std::string currentConfig;

	if (data.contains("currentConfig") &&
		data["currentConfig"].is_string())
	{
		currentConfig =
			data["currentConfig"].get<std::string>();
	}

	// ─────────────────────────────────────────────────────────────────────
	// Меню
	// ─────────────────────────────────────────────────────────────────────

	std::println("Конфигурации сборки:\n");

	for (std::size_t i = 0; i < configs.size(); ++i)
	{
		const auto& cfg = configs[i];

		const std::string name =
			cfg.value("name", "<unknown>");

		const std::string desc =
			cfg.value("description", "");

		const bool active =
			(name == currentConfig);

		std::println(
			"  [{}] {:<20} — {}{}",
			i + 1,
			name,
			desc,
			active ? "  ← активная" : "");

		std::unordered_set<std::string> cfgActiveDefines;
		if (cfg.contains("activeDefines"))
		{
			for (const auto& item : cfg["activeDefines"])
			{
				cfgActiveDefines.insert(item.get<std::string>());
			}
		}

		std::vector<std::string> projDefs;
		std::vector<std::string> cmakeDefs;

		if (data.contains("defines"))
		{
			for (const auto& def : data["defines"])
			{
				if (def.value("isArchived", false)) continue;
				std::string dname = def.value("name", "");
				if (dname.empty() || !cfgActiveDefines.contains(dname)) continue;

				if (def.value("isCMake", false))
				{
					cmakeDefs.push_back(dname);
				}
				else
				{
					projDefs.push_back(dname);
				}
			}
		}

		if (!cmakeDefs.empty())
		{
			std::println("       [CMake Defines]");
			for (const auto& def : cmakeDefs)
			{
				std::println("         - {}", def);
			}
		}

		if (!projDefs.empty())
		{
			std::println("       [Project Defines]");
			for (const auto& def : projDefs)
			{
				std::println("         - {}", def);
			}
		}
		std::println("");
	}

	if (!currentConfig.empty())
		std::println("Текущая: {}", currentConfig);

	std::print("Введите номер: ");

	const std::string input =
		ReadDigits();

	if (input.empty())
	{
		std::println("Отменено.");
		return 0;
	}

	int choice{};

	try
	{
		choice = std::stoi(input);
	}
	catch (...)
	{
		std::println("Некорректный ввод.");
		Pause();
		return 1;
	}

	if (choice < 1 ||
		choice > static_cast<int>(configs.size()))
	{
		std::println(
			"Нет конфигурации с номером {}.",
			choice);

		Pause();
		return 1;
	}

	const auto& selected =
		configs[choice - 1];

	// ─────────────────────────────────────────────────────────────────────
	// Применение
	// ─────────────────────────────────────────────────────────────────────

	WriteCmake(cmakePath, data, selected);

	data["currentConfig"] =
		selected["name"];

	{
		std::ofstream file(profilesPath);

		if (!file)
			Fail("Не удалось записать profiles.json");

		file << data.dump(2, ' ', false) << '\n';
	}

	std::println(
		"Конфигурация \"{}\" применена.",
		selected["name"].get<std::string>());

	std::println("Готово.");

	Pause();

	return 0;
}