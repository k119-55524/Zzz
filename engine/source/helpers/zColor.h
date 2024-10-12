#pragma once

#include "../SceneEntities/zSerialize.h"

namespace Zzz
{
	class zColor : protected zSerialize
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
				memcpy(color, other.color, sizeof(color));

			return *this;
		};
		inline zColor& operator=(zColor&& other) noexcept
		{
			if (this != &other)
				memcpy(color, other.color, sizeof(color));

			return *this;
		};

		inline void SetColor(float c) noexcept
		{
			color[0] = c;
			color[1] = c;
			color[2] = c;
			color[3] = 1.0;
		};
		inline void SetColor(float r, float g, float b) noexcept
		{ 
			color[0] = r;
			color[1] = g;
			color[2] = b;
			color[3] = 1.0;
		};
		inline void SetColor(float r, float g, float b, float a) noexcept
		{
			color[0] = r;
			color[1] = g;
			color[2] = b;
			color[3] = a;
		};
		inline void SetColor(const zColor& _color) noexcept
		{ 
			memcpy(color, _color.GetColor(), sizeof(color));
		};
		inline void Default() noexcept
		{
			color[0] = 0.0f;
			color[1] = 0.2f;
			color[2] = 0.4f;
			color[3] = 1.0f;
		};

		inline const float* GetColor() const noexcept { return color; };

		inline void Serialize(stringstream& buffer) const
		{
			zSerialize::Serialize(buffer, color[0]);
			zSerialize::Serialize(buffer, color[1]);
			zSerialize::Serialize(buffer, color[2]);
			zSerialize::Serialize(buffer, color[3]);
		}

		void DeSerialize(istringstream& buffer)
		{
			zSerialize::DeSerialize(buffer, color[0]);
			zSerialize::DeSerialize(buffer, color[1]);
			zSerialize::DeSerialize(buffer, color[2]);
			zSerialize::DeSerialize(buffer, color[3]);
		}

	private:
		float color[4];
	};
}
