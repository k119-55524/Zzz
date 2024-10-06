#pragma once

#include "../Helpers/zColor.h"
#include "../Helpers/zResult.h"

namespace Zzz::Core
{
	enum e_TypeClear : zU32
	{
		eUnclear,
		Color
	};

	class Scene
	{
	public:
		Scene();

		inline void SetClearColor(const zColor& color) noexcept { clearColor = color; };
		inline const zColor& GetClearColor() const noexcept { return clearColor; };

		void Update();

#ifdef _SERVICES
	public:
#else
	private:
#endif
		zResult Save(const zStr& path);

	private:
		e_TypeClear typeClear;
		zColor clearColor;
	};
}