
#include "pch.h"

export module size2D;

import result;
import serializer;

export namespace zzz::engineCore
{
	/**
	* @class size2D
	* @brief Шаблонный класс для хранения и управления двумерными размерами.
	*
	* Этот класс предназначен для работы с шириной и высотой, поддерживая различные числовые типы
	* (например, int, unsigned long, float). Наследуется от zSerialize для поддержки
	* сериализации и десериализации.
	*
	* @tparam T Тип данных для хранения ширины и высоты (должен быть арифметическим).
	*			Значение по умолчанию: zU64.
	*/
	template<typename T = zU64, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
	class size2D : protected serializer
	{
	public:
		size2D() : width{ 0 }, height{ 0 } {}
		explicit size2D(T size) : width{ size }, height{ size } {}
		size2D(T _width, T _height) : width{ _width }, height{ _height } {}
		size2D(const size2D& size) : width{ size.width }, height{ size.height } {}
		size2D(size2D&&) = default;

		inline void SetSize(T _width, T _height) noexcept { width = _width; height = _height; }

		inline bool operator==(const size2D& other) const noexcept { return width == other.width && height == other.height; }
		inline bool operator!=(const size2D& other) const noexcept { return !(*this == other); }

		inline result<> SaveSerialize(std::stringstream& buffer) const
		{
			auto res =
				serializer::Serialize(buffer, width)
				.and_then([]() { return serializer::Serialize(buffer, height); });

			return res;
		}

		inline result<> SaveDeSerialize(std::istringstream& buffer)
		{
			auto res =
				serializer::DeSerialize(buffer, width)
				.and_then([]() { return serializer::DeSerialize(buffer, height); });

			return res;
		}

		T width{ 0 };  // Ширина объекта.
		T height{ 0 }; // Высота объекта.
	};
}