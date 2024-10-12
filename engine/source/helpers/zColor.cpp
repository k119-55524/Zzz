#include "pch.h"
#include "zColor.h"

using namespace Zzz;

zColor::zColor() :
	color{ 0.0f, 0.0f, 0.0f, 1.0f }
{
}

zColor::zColor(float c) :
	color{ c, c, c, 1.0f }
{
}

zColor::zColor(float r, float g, float b) :
	color{ r, g, b, 1.0f }
{
}

zColor::zColor(float r, float g, float b, float a) :
	color{ r, g, b, a }
{
}

zColor::zColor(const zColor& other)
{
	memcpy(color, other.color, sizeof(color));
}

zColor::zColor(zColor&& other) noexcept
{
	memcpy(color, other.color, sizeof(color));
}
