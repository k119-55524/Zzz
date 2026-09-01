#pragma once

#include <span>
#include <vector>
#include <string>
#include <cstring>
#include <expected>
#include <concepts>
#include <type_traits>

#include "core/utils/Export.h"
#include "core/utils/Macroses.h"
#include <math/Math.h>

namespace zzz::core
{
	/// @brief Концепт для типов примитивов и enum, подлежащих сериализации.
	/// @details Рассчитан на 64-битные системы (x64 / ARM64, Little-Endian: Windows, Linux, macOS, Android, iOS).
	/// Поддерживает: целочисленные типы фиксированной ширины, float, double, enum/enum class.
	/// Исключает (для обеспечения кроссплатформенности):
	/// - bool (сериализуется отдельно через 1 байт uint8_t);
	/// - wchar_t / std::wstring (различаются 2B MSVC / 4B GCC);
	/// - long double (различается 8B MSVC / 16B GCC);
	/// - указатели.
	template<typename T>
	concept SerializablePrimitive =
		(std::integral<T> || std::floating_point<T> || std::is_enum_v<T>) &&
		!std::same_as<T, bool> &&
		!std::same_as<T, wchar_t> &&
		!std::same_as<T, long double>;

	class Serializer;

	/// @brief Интерфейс для сериализуемых объектов движка.
	class Z_CORE_API ISerializable
	{
	public:
		virtual ~ISerializable() = default;

	protected:
		/// @brief Записывает состояние объекта в бинарный буфер.
		/// @param buffer Целевой буфер байт.
		/// @param serializer Ссылка на сериализатор.
		/// @return Ожидает void при успехе или сообщение об ошибке.
		[[nodiscard]] virtual std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const = 0;

		/// @brief Восстанавливает состояние объекта из бинарного буфера.
		/// @param buffer Исходный буфер байт.
		/// @param offset Смещение чтения в буфере (обновляется при чтении).
		/// @param serializer Ссылка на сериализатор.
		/// @return Ожидает void при успехе или сообщение об ошибке.
		[[nodiscard]] virtual std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer) = 0;

		friend class Serializer;
	};

	/// @brief Класс для бинарной сериализации и десериализации данных.
	class Z_CORE_API Serializer
	{
	public:
		/// @brief Сериализует примитивные типы и enum в буфер байт.
		/// @tparam T Тип сериализуемого значения.
		/// @param buffer Целевой вектор байт.
		/// @param value Значение для записи.
		/// @return Результат сериализации.
		template<typename T> requires SerializablePrimitive<T>
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const T& value) const
		{
			if constexpr (std::is_enum_v<T>)
			{
				using UnderlyingType = std::underlying_type_t<T>;
				const UnderlyingType val = static_cast<UnderlyingType>(value);
				return Serialize(buffer, val);
			}
			else
			{
				const std::size_t old_size = buffer.size();
				buffer.resize(old_size + sizeof(T));
				std::memcpy(buffer.data() + old_size, &value, sizeof(T));
				return {};
			}
		}

		/// @brief Десериализует примитивные типы и enum из буфера байт.
		/// @tparam T Тип считываемого значения.
		/// @param buffer Исходный буфер байт.
		/// @param offset Текущее смещение в буфере.
		/// @param value Переменная для записи результата.
		/// @return Результат десериализации.
		template<typename T> requires SerializablePrimitive<T>
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, T& value) const
		{
			if constexpr (std::is_enum_v<T>)
			{
				using UnderlyingType = std::underlying_type_t<T>;
				UnderlyingType val{};
				auto res = Deserialize(buffer, offset, val);
				if (!res)
					return res;
				value = static_cast<T>(val);
				return {};
			}
			else
			{
				if (offset > buffer.size() || buffer.size() - offset < sizeof(T))
					return std::unexpected("Buffer too small.");

				std::memcpy(&value, buffer.data() + offset, sizeof(T));
				offset += sizeof(T);
				return {};
			}
		}

		/// @brief Сериализует логический тип bool как 1 байт (uint8_t).
		/// @param buffer Целевой вектор байт.
		/// @param value Значение bool.
		/// @return Результат сериализации.
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, bool value) const
		{
			const uint8_t byteVal = value ? 1 : 0;
			return Serialize(buffer, byteVal);
		}

		/// @brief Десериализует логический тип bool из 1 байта (uint8_t).
		/// @param buffer Исходный буфер байт.
		/// @param offset Текущее смещение в буфере.
		/// @param value Переменная для записи bool.
		/// @return Результат десериализации.
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, bool& value) const
		{
			uint8_t byteVal = 0;
			auto res = Deserialize(buffer, offset, byteVal);
			if (!res)
				return res;
			value = (byteVal != 0);
			return {};
		}

		/// @brief Сериализует сырой диапазон байт (span).
		/// @param buffer Целевой вектор байт.
		/// @param bytes Непрерывный диапазон байт.
		/// @return Результат сериализации.
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, std::span<const std::byte> bytes) const
		{
			const auto oldSize = buffer.size();
			buffer.resize(oldSize + bytes.size());
			std::memcpy(buffer.data() + oldSize, bytes.data(), bytes.size());

			return {};
		}

		/// @brief Десериализует сырой диапазон байт в span целевой памяти.
		/// @param buffer Исходный буфер байт.
		/// @param offset Текущее смещение в буфере.
		/// @param targetSpan Целевой спан байт.
		/// @return Результат десериализации.
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, std::span<std::byte> targetSpan) const
		{
			if (offset > buffer.size() || buffer.size() - offset < targetSpan.size())
				return std::unexpected("Buffer too small.");

			std::memcpy(targetSpan.data(), buffer.data() + offset, targetSpan.size());
			offset += targetSpan.size();

			return {};
		}

		/// @brief Сериализует фиксированный массив std::array байт.
		/// @tparam N Размер массива.
		/// @param buffer Целевой вектор байт.
		/// @param value Исходный массив байт.
		/// @return Результат сериализации.
		template<std::size_t N>
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const std::array<std::byte, N>& value) const
		{
			return Serialize(buffer, std::span<const std::byte>(value));
		}

		/// @brief Десериализует фиксированный массив std::array байт.
		/// @tparam N Размер массива.
		/// @param buffer Исходный буфер байт.
		/// @param offset Текущее смещение в буфере.
		/// @param value Целевой массив байт.
		/// @return Результат десериализации.
		template<std::size_t N>
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, std::array<std::byte, N>& value) const
		{
			return Deserialize(buffer, offset, std::span<std::byte>(value));
		}

		/// @brief Сериализует строку UTF-8 (std::string) с предварительной записью её длины.
		/// @param buffer Целевой вектор байт.
		/// @param str Исходная строка UTF-8.
		/// @return Результат сериализации.
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const std::string& str) const;

		/// @brief Десериализует строку UTF-8 (std::string).
		/// @param buffer Исходный буфер байт.
		/// @param offset Текущее смещение в буфере.
		/// @param str Переменная для записи строки.
		/// @return Результат десериализации.
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, std::string& str) const;

		/// @brief Сериализует пользовательский объект, реализующий ISerializable.
		/// @param buffer Целевой вектор байт.
		/// @param obj Объект для сериализации.
		/// @return Результат сериализации.
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const ISerializable& obj) const;

		template<typename T> requires std::derived_from<std::remove_cvref_t<T>, ISerializable> && (!std::same_as<std::remove_cvref_t<T>, ISerializable>)
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const T& obj) const
		{
			return Serialize(buffer, static_cast<const ISerializable&>(obj));
		}

		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, ISerializable& obj) const;

		template<typename T> requires std::derived_from<std::remove_cvref_t<T>, ISerializable> && (!std::same_as<std::remove_cvref_t<T>, ISerializable>)
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, T& obj) const
		{
			return Deserialize(buffer, offset, static_cast<ISerializable&>(obj));
		}

		template<typename T>
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const zzz::math::Size2D<T>& size) const
		{
			return Serialize(buffer, size.width)
				.and_then([&]() { return Serialize(buffer, size.height); });
		}

		template<typename T>
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, zzz::math::Size2D<T>& size) const
		{
			return Deserialize(buffer, offset, size.width)
				.and_then([&]() { return Deserialize(buffer, offset, size.height); });
		}

		template<typename T>
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const zzz::math::Point2D<T>& pt) const
		{
			return Serialize(buffer, pt.x)
				.and_then([&]() { return Serialize(buffer, pt.y); });
		}

		template<typename T>
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, zzz::math::Point2D<T>& pt) const
		{
			return Deserialize(buffer, offset, pt.x)
				.and_then([&]() { return Deserialize(buffer, offset, pt.y); });
		}

		template<typename T>
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const zzz::math::Vec2<T>& v) const
		{
			return Serialize(buffer, v.x)
				.and_then([&]() { return Serialize(buffer, v.y); });
		}

		template<typename T>
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, zzz::math::Vec2<T>& v) const
		{
			return Deserialize(buffer, offset, v.x)
				.and_then([&]() { return Deserialize(buffer, offset, v.y); });
		}

		template<typename T>
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const zzz::math::Vec3<T>& v) const
		{
			return Serialize(buffer, v.x)
				.and_then([&]() { return Serialize(buffer, v.y); })
				.and_then([&]() { return Serialize(buffer, v.z); });
		}

		template<typename T>
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, zzz::math::Vec3<T>& v) const
		{
			return Deserialize(buffer, offset, v.x)
				.and_then([&]() { return Deserialize(buffer, offset, v.y); })
				.and_then([&]() { return Deserialize(buffer, offset, v.z); });
		}

		template<typename T>
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const zzz::math::Vec4<T>& v) const
		{
			return Serialize(buffer, v.x)
				.and_then([&]() { return Serialize(buffer, v.y); })
				.and_then([&]() { return Serialize(buffer, v.z); })
				.and_then([&]() { return Serialize(buffer, v.w); });
		}

		template<typename T>
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, zzz::math::Vec4<T>& v) const
		{
			return Deserialize(buffer, offset, v.x)
				.and_then([&]() { return Deserialize(buffer, offset, v.y); })
				.and_then([&]() { return Deserialize(buffer, offset, v.z); })
				.and_then([&]() { return Deserialize(buffer, offset, v.w); });
		}

		template<typename T>
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const zzz::math::Rect2D<T>& rect) const
		{
			return Serialize(buffer, rect.position)
				.and_then([&]() { return Serialize(buffer, rect.size); });
		}

		template<typename T>
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, zzz::math::Rect2D<T>& rect) const
		{
			return Deserialize(buffer, offset, rect.position)
				.and_then([&]() { return Deserialize(buffer, offset, rect.size); });
		}

		template<typename T>
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const zzz::math::Color3<T>& color) const
		{
			return Serialize(buffer, color.R)
				.and_then([&]() { return Serialize(buffer, color.G); })
				.and_then([&]() { return Serialize(buffer, color.B); });
		}

		template<typename T>
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, zzz::math::Color3<T>& color) const
		{
			return Deserialize(buffer, offset, color.R)
				.and_then([&]() { return Deserialize(buffer, offset, color.G); })
				.and_then([&]() { return Deserialize(buffer, offset, color.B); });
		}

		template<typename T>
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const zzz::math::Color4<T>& color) const
		{
			return Serialize(buffer, color.R)
				.and_then([&]() { return Serialize(buffer, color.G); })
				.and_then([&]() { return Serialize(buffer, color.B); })
				.and_then([&]() { return Serialize(buffer, color.A); });
		}

		template<typename T>
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, zzz::math::Color4<T>& color) const
		{
			return Deserialize(buffer, offset, color.R)
				.and_then([&]() { return Deserialize(buffer, offset, color.G); })
				.and_then([&]() { return Deserialize(buffer, offset, color.B); })
				.and_then([&]() { return Deserialize(buffer, offset, color.A); });
		}

		template<typename T>
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const zzz::math::Mat3<T>& m) const
		{
			for (std::size_t i = 0; i < 9; ++i)
			{
				auto res = Serialize(buffer, m.elements[i]);
				if (!res)
					return res;
			}
			return {};
		}

		template<typename T>
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, zzz::math::Mat3<T>& m) const
		{
			for (std::size_t i = 0; i < 9; ++i)
			{
				auto res = Deserialize(buffer, offset, m.elements[i]);
				if (!res)
					return res;
			}
			return {};
		}

		template<typename T>
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const zzz::math::Mat4<T>& m) const
		{
			for (std::size_t i = 0; i < 16; ++i)
			{
				auto res = Serialize(buffer, m.elements[i]);
				if (!res)
					return res;
			}
			return {};
		}

		template<typename T>
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, zzz::math::Mat4<T>& m) const
		{
			for (std::size_t i = 0; i < 16; ++i)
			{
				auto res = Deserialize(buffer, offset, m.elements[i]);
				if (!res)
					return res;
			}
			return {};
		}
		template<typename T>
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const zzz::math::Quat<T>& q) const
		{
			return Serialize(buffer, q.x)
				.and_then([&]() { return Serialize(buffer, q.y); })
				.and_then([&]() { return Serialize(buffer, q.z); })
				.and_then([&]() { return Serialize(buffer, q.w); });
		}

		template<typename T>
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, zzz::math::Quat<T>& q) const
		{
			return Deserialize(buffer, offset, q.x)
				.and_then([&]() { return Deserialize(buffer, offset, q.y); })
				.and_then([&]() { return Deserialize(buffer, offset, q.z); })
				.and_then([&]() { return Deserialize(buffer, offset, q.w); });
		}
	};
}
