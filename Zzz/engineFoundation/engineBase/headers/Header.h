#pragma once

#include <bit>
#include <span>
#include <mutex>
#include <array>
#include <atomic>
#include <vector>
#include <string>
#include <cctype>
#include <memory>
#include <format>
#include <locale>
#include <future>
#include <fstream>
#include <sstream>
#include <variant>
#include <utility>
#include <codecvt>
#include <cstdint>
#include <cassert>
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <filesystem>
#include <functional>
#include <string_view>
#include <type_traits>
#include <shared_mutex>
#include <unordered_map>
#include <source_location>

#include "PlatformsDefines.h"
#include "HeadersMSWin.h"
#include "HeaderVulkan.h"
#include "HeaderMetal.h"
#include "headerDX.h"
#include "MacroDefinition.h"
#include "HeaderThrowWrappers.h"
#include "HeaderTypes.h"
#include "HeaderEnums.h"

namespace zzz
{
	template<typename T>
	inline void ensure(T&& condition,
		const std::string& msg = "Ensure failed",
		const std::source_location& loc = std::source_location::current())
	{
		if (!condition)
			throw std::runtime_error(
				msg +
				". Method=" + std::string(loc.function_name()) +
				", line=" + std::to_string(loc.line()) +
				", file=" + std::string(loc.file_name()));
	}

	template<typename T>
	inline void SafeRelease(T*& ptr)
	{
		if (ptr)
		{
			ptr->Release();
			ptr = nullptr;
		}
	}

#pragma region SafeMake
	template<typename Creator, typename... Args>
	inline decltype(auto) safe_make_impl(
		Creator&& creator,
		const std::source_location& loc,
		Args&&... args) noexcept(false)
	{
		try
		{
			return creator(std::forward<Args>(args)...);
		}
		catch (const std::bad_alloc& e)
		{
			std::ostringstream oss;
			oss << "Memory allocation error. Exception: " << e.what()
				<< ". Method=" << loc.function_name()
				<< ", line=" << loc.line()
				<< ", file=" << loc.file_name();
			throw std::runtime_error(oss.str());
		}
	}

	// Безопасный make_shared
	template<typename T, typename... Args>
	inline std::shared_ptr<T> safe_make_shared(Args&&... args) noexcept(false)
	{
		return safe_make_impl(
			[](auto&&... a) { return std::make_shared<T>(std::forward<decltype(a)>(a)...); },
			std::source_location::current(),
			std::forward<Args>(args)...
		);
	}

	// Безопасный make_unique
	template<typename T, typename... Args>
	inline std::unique_ptr<T> safe_make_unique(Args&&... args) noexcept(false)
	{
		return safe_make_impl(
			[](auto&&... a) { return std::make_unique<T>(std::forward<decltype(a)>(a)...); },
			std::source_location::current(),
			std::forward<Args>(args)...
		);
	}
#pragma endregion SafeMake
}