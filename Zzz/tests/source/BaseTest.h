
#pragma once
#include "Header.h"

#include <string>

#include "ITest.h"

import Result;

using namespace zzz;

namespace zzz::ztests
{
	template <typename T>
	constexpr std::string_view TypeName()
	{
#if defined(_MSC_VER)
		constexpr std::string_view sig = __FUNCSIG__;
		constexpr std::string_view prefix = "TypeName<";
		constexpr std::string_view suffix = ">(void)";
#elif defined(__clang__) || defined(__GNUC__)
		constexpr std::string_view sig = __PRETTY_FUNCTION__;
		constexpr std::string_view prefix = "T = ";
		constexpr std::string_view suffix = "]";
#endif

		const auto start = sig.find(prefix) + prefix.size();
		const auto end = sig.find(suffix);

		constexpr std::string_view name = sig.substr(start, end - start);

		return name;
	}

	template<typename Derived>
	class BaseTest : public ITest
	{
	public:
		BaseTest() :
			testName{ TypeName<Derived>() }
		{
		}

		constexpr std::string_view GetTestName() const { return testName; };

	protected:
		std::string_view testName;
	};
}