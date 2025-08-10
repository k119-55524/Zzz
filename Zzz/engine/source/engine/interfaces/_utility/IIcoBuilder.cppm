#include "pch.h"
export module IIcoBuilder;

import result;

using namespace zzz;

export namespace zzz::icoBuilder
{
	template<typename IconT>
	class IIcoBuilder
	{
	public:
		virtual ~IIcoBuilder() = default;

		virtual result<IconT> LoadIco(const std::wstring& filePath, int size) = 0;
	};
}