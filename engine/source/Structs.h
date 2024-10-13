#pragma once

#include "Constants.h"
#include "SceneEntities/zSerialize.h"

namespace Zzz
{
	enum e_InitState : zI32
	{
		eInitNot,		// Готов к инициализации
		eInitProcess,	// Идёт процесс инициализации
		eInitOK,		// Инициализированн
		eInitError,		// Ошибка инициализации
		eTermination	// Процесс деинициализации
	};

	class zSize : protected zSerialize
	{
	public:
		zSize() : width{ 0 }, height{ 0 } {};
		zSize(zU64 size) : width{ size }, height{ size } {};
		zSize(zU64 _width, zU64 _height) : width{ _width }, height{ _height } {};

		zSize(const zSize& size) : width{ size.width }, height{ size.height } {};
		zSize(zSize&&) = default;

		inline void SetSize(zU64 _width, zU64 _height) noexcept
		{
			width = _width;
			height = _height;
		}

		inline void Serialize(stringstream& buffer) const
		{
			zSerialize::Serialize(buffer, width);
			zSerialize::Serialize(buffer, height);
		}

		inline void DeSerialize(istringstream& buffer)
		{
			zSerialize::DeSerialize(buffer, width);
			zSerialize::DeSerialize(buffer, height);
		}

		zU64 width{ 0 };
		zU64 height{ 0 };
	};
}