#include "pch.h"
export module IIcoBuilder;

import Result;

using namespace zzz;

export namespace zzz::engineCore
{
	template<typename IconT>
	class IIcoBuilder
	{
	public:
		virtual ~IIcoBuilder() = default;

		virtual Result<IconT> LoadIco(const std::wstring& filePath, int size) = 0;
	};
}