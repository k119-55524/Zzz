#include "Serializer.h"

using namespace zzz::common;

std::expected<void, std::string> Serializer::Serialize(std::vector<std::byte>& buffer, const std::string& str) const
{
	// Сначала записываем размер строки
	const std::size_t size = str.size();
	auto res = Serialize(buffer, size);
	if (!res)
		return res;

	// Затем записываем данные строки
	const std::size_t old_size = buffer.size();
	buffer.resize(old_size + size);
	std::memcpy(buffer.data() + old_size, str.data(), size);

	return {};
}

std::expected<void, std::string> Serializer::Deserialize(std::span<const std::byte> buffer, std::size_t& offset, std::string& str) const
{
	// Сначала читаем размер строки
	std::size_t size = 0;
	auto res = Deserialize(buffer, offset, size);
	if (!res)
		return res;

	// Проверяем, достаточно ли данных в буфере
	if (offset > buffer.size() || buffer.size() - offset < size)
		return std::unexpected("Buffer too small for string data.");

	// Читаем данные строки
	str.resize(size);
	std::memcpy(str.data(), buffer.data() + offset, size);
	offset += size;

	return {};
}

std::expected<void, std::string> Serializer::Serialize(std::vector<std::byte>& buffer, const ISerializable& obj) const
{
	return obj.Serialize(buffer, *this);
}

std::expected<void, std::string> Serializer::Deserialize(std::span<const std::byte> buffer, std::size_t& offset, ISerializable& obj) const
{
	return obj.Deserialize(buffer, offset, *this);
}