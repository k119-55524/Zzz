#pragma once

namespace Zzz
{
	class zColor
	{
	public:
		zColor();
		zColor(float c);
		zColor(float r, float g, float b);
		zColor(float r, float g, float b, float a);
		zColor(const zColor& other);
		zColor(zColor&& other) noexcept;

		inline zColor& operator=(const zColor& other)
		{
			if (this != &other)
				memcpy(clearColor, other.clearColor, sizeof(clearColor));

			return *this;
		};
		inline zColor& operator=(zColor&& other) noexcept
		{
			if (this != &other)
				memcpy(clearColor, other.clearColor, sizeof(clearColor));

			return *this;
		};

		inline void SetColor(float c) noexcept
		{
			clearColor[0] = c;
			clearColor[1] = c;
			clearColor[2] = c;
			clearColor[3] = 1.0;
		};
		inline void SetColor(float r, float g, float b) noexcept
		{ 
			clearColor[0] = r;
			clearColor[1] = g;
			clearColor[2] = b;
			clearColor[3] = 1.0;
		};
		inline void SetColor(float r, float g, float b, float a) noexcept
		{
			clearColor[0] = r;
			clearColor[1] = g;
			clearColor[2] = b;
			clearColor[3] = a;
		};
		inline void SetColor(const zColor& color) noexcept
		{ 
			memcpy(clearColor, color.GetColor(), sizeof(clearColor));
		};
		inline void Default() noexcept
		{
			clearColor[0] = 0.0f;
			clearColor[1] = 0.2f;
			clearColor[2] = 0.4f;
			clearColor[3] = 1.0f;
		};

		inline const float* GetColor() const noexcept { return clearColor; };

	private:
		float clearColor[4];
	};
}
