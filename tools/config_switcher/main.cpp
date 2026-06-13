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
using json   = nlohmann::json;

// ─────────────────────────────────────────────────────────────────────────────
// terminal raw mode
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
// helpers
// ─────────────────────────────────────────────────────────────────────────────

static void pause()
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
static void fail(std::string_view msg)
{
    std::println(stderr, "Ошибка: {}", msg);
    pause();
    std::exit(1);
}

static fs::path find_project_root()
{
    fs::path current = fs::current_path();

    while (!current.empty())
    {
        if (fs::exists(current / "build_configs" / "profiles.json"))
            return current;

        current = current.parent_path();
    }

    fail("Не удалось найти build_configs/profiles.json");
}

// ─────────────────────────────────────────────────────────────────────────────
// input
// ─────────────────────────────────────────────────────────────────────────────

// Enter  -> confirm input
// Esc    -> cancel
// Digits -> append
// Backspace -> remove last digit

static std::string read_digits()
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
// cmake
// ─────────────────────────────────────────────────────────────────────────────

static void write_cmake(
    const fs::path& path,
    const json& data,
    const json& config)
{
    std::ofstream file(path);

    if (!file)
        fail("Не удалось открыть current.cmake");

    file << "# Auto-generated file.\n";
    file << "# Configuration: "
         << config["name"].get<std::string>()
         << "\n\n";

    std::unordered_set<std::string> active_defines;

    for (const auto& item : config["activeDefines"])
    {
        active_defines.insert(item.get<std::string>());
    }

    for (const auto& def : data["defines"])
    {
        if (def.value("isArchived", false))
            continue;

        if (!def.value("isCMake", false))
            continue;

        const std::string name = def.at("name").get<std::string>();
        const bool enabled = active_defines.contains(name);

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
        const bool enabled = active_defines.contains(name);

        file << "    " << name << '=' << (enabled ? "1" : "0") << '\n';
    }

    file << ")\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// main
// ─────────────────────────────────────────────────────────────────────────────

int main()
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    const fs::path root =
        find_project_root();

    const fs::path profiles_path =
        root / "build_configs" / "profiles.json";

    const fs::path cmake_path =
        root / "build_configs" / "current.cmake";

    const fs::path build_dir =
        root / "build";

    json data;

    try
    {
        std::ifstream file(profiles_path);

        if (!file)
            fail("Не удалось открыть profiles.json");

        data = json::parse(file);
    }
    catch (const json::exception& e)
    {
        fail(std::string("Ошибка чтения profiles.json: ") + e.what());
    }

    if (!data.contains("defines") ||
        !data["defines"].is_array())
    {
        fail("profiles.json: отсутствует массив defines");
    }

    if (!data.contains("configurations") ||
        !data["configurations"].is_array())
    {
        fail("profiles.json: отсутствует массив configurations");
    }

    const auto& configs =
        data["configurations"];

    if (configs.empty())
        fail("Нет конфигураций.");

    std::string current_config;

    if (data.contains("currentConfig") &&
        data["currentConfig"].is_string())
    {
        current_config =
            data["currentConfig"].get<std::string>();
    }

    // ─────────────────────────────────────────────────────────────────────
    // menu
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
            (name == current_config);

        std::println(
            "  [{}] {:<20} — {}{}",
            i + 1,
            name,
            desc,
            active ? "  ← активная" : "");
    }

    if (!current_config.empty())
        std::println("\nТекущая: {}", current_config);

    std::print("\nВведите номер: ");

    const std::string input =
        read_digits();

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
        pause();
        return 1;
    }

    if (choice < 1 ||
        choice > static_cast<int>(configs.size()))
    {
        std::println(
            "Нет конфигурации с номером {}.",
            choice);

        pause();
        return 1;
    }

    const auto& selected =
        configs[choice - 1];

    // ─────────────────────────────────────────────────────────────────────
    // apply
    // ─────────────────────────────────────────────────────────────────────

    write_cmake(cmake_path, data, selected);

    data["currentConfig"] =
        selected["name"];

    {
        std::ofstream file(profiles_path);

        if (!file)
            fail("Не удалось записать profiles.json");

        file << data.dump(2, ' ', false) << '\n';
    }

    std::println(
        "Конфигурация \"{}\" применена.",
        selected["name"].get<std::string>());

    // ─────────────────────────────────────────────────────────────────────
    // rebuild
    // ─────────────────────────────────────────────────────────────────────

    if (build_dir.filename() != "build")
    {
        fail("Некорректный путь build directory.");
    }

    if (fs::exists(build_dir))
    {
        std::println(
            "Удаление {}...",
            build_dir.string());

        std::error_code ec;

        fs::remove_all(build_dir, ec);

        if (ec)
        {
            fail(
                "Не удалось удалить build/: " +
                ec.message());
        }
    }

    std::println("Запуск CMake...");

#ifdef _WIN32

    std::wstring cmd =
        L"cmake -S \"" +
        root.wstring() +
        L"\" -B \"" +
        build_dir.wstring() +
        L"\"";

    int result = _wsystem(cmd.c_str());

#else

    std::string cmd =
        "cmake -S \"" +
        root.string() +
        "\" -B \"" +
        build_dir.string() +
        "\"";

    int result = std::system(cmd.c_str());

#endif

    if (result != 0)
    {
        fail(
            "CMake завершился с кодом " +
            std::to_string(result));
    }

    std::println("Готово.");

    pause();

    return 0;
}