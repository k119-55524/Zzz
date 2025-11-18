
export module Size2D;

import Result;
import Serializer;

export namespace zzz::core
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
	class Size2D final : public ISerializable
	{
	public:
		Size2D() : width{ 0 }, height{ 0 } {}
		explicit Size2D(T size) : width{ size }, height{ size } {}
		Size2D(T _width, T _height) : width{ _width }, height{ _height } {}
		Size2D(const Size2D& size) : width{ size.width }, height{ size.height } {}
		Size2D(Size2D&&) = default;

		Size2D& operator=(const Size2D&) = default;
		Size2D& operator=(Size2D&&) noexcept = default;

		inline void SetSize(T _width, T _height) noexcept { width = _width; height = _height; }

		inline bool operator==(const Size2D& other) const noexcept { return width == other.width && height == other.height; }
		inline bool operator!=(const Size2D& other) const noexcept { return !(*this == other); }

		T width;  // Ширина объекта.
		T height; // Высота объекта.

	private:
		Result<> Serialize(std::vector<std::byte>& buffer, const zzz::core::Serializer& s) const override
		{
			return s.Serialize(buffer, width)
				.and_then([&]() {return s.Serialize(buffer, height); });
		}

		Result<> DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, const zzz::core::Serializer& s) override
		{
			return s.DeSerialize(buffer, offset, width)
				.and_then([&]() {return s.DeSerialize(buffer, offset, height); });
		}
	};
}