#include "pch.h"
#include "IIO.h"

using namespace Zzz::Platforms;

IIO::IIO() :
	separator{ std::filesystem::path::preferred_separator }
{
}

IIO::~IIO()
{
}