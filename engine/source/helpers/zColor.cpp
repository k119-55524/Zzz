#include "pch.h"
#include "zColor.h"

using namespace Zzz;

zColor::zColor() :
	clearColor{ 0.0f, 0.0f, 0.0f, 1.0f }
{
}

zColor::zColor(float c) :
	clearColor{ c, c, c, 1.0f }
{
}

zColor::zColor(float r, float g, float b) :
	clearColor{ r, g, b, 1.0f }
{
}

zColor::zColor(float r, float g, float b, float a) :
	clearColor{ r, g, b, a }
{
}

zColor::zColor(const zColor& other)
{
	memcpy(clearColor, other.clearColor, sizeof(clearColor));
}

zColor::zColor(zColor&& other) noexcept
{
	memcpy(clearColor, other.clearColor, sizeof(clearColor));
}
