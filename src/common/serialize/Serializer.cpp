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

std::expected<void, std::string> Serializer::DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, std::string& str) const
{
	// Сначала читаем размер строки
	std::size_t size = 0;
	auto res = DeSerialize(buffer, offset, size);
	if (!res)
		return res;

	// Проверяем, достаточно ли данных в буфере
	if (offset + size > buffer.size())
		return std::unexpected("Buffer too small for string data.");

	// Читаем данные строки
	str.resize(size);
	std::memcpy(str.data(), buffer.data() + offset, size);
	offset += size;

	return {};
}

std::expected<void, std::string> Serializer::Serialize(std::vector<std::byte>& buffer, const std::wstring& str) const
{
	// Сначала записываем размер строки
	const std::size_t size = str.size();
	auto res = Serialize(buffer, size);
	if (!res)
		return res;

	// Затем записываем данные строки (размер в байтах)
	const std::size_t byte_size = size * sizeof(wchar_t);
	const std::size_t old_size = buffer.size();
	buffer.resize(old_size + byte_size);
	std::memcpy(buffer.data() + old_size, str.data(), byte_size);

	return {};
}

std::expected<void, std::string> Serializer::DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, std::wstring& str) const
{
	// Сначала читаем размер строки (количество символов)
	std::size_t size = 0;
	auto res = DeSerialize(buffer, offset, size);
	if (!res)
		return res;

	// Вычисляем размер в байтах
	const std::size_t byte_size = size * sizeof(wchar_t);

	// Проверяем, достаточно ли данных в буфере
	if (offset + byte_size > buffer.size())
		return std::unexpected("Buffer too small for wstring data.");

	// Читаем данные строки
	str.resize(size);
	std::memcpy(str.data(), buffer.data() + offset, byte_size);
	offset += byte_size;

	return {};
}

std::expected<void, std::string> Serializer::Serialize(std::vector<std::byte>& buffer, const ISerializable& obj) const
{
	return obj.Serialize(buffer, *this);
}

// Десериализация объектов ISerializable
std::expected<void, std::string> Serializer::DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, ISerializable& obj) const
{
	return obj.DeSerialize(buffer, offset, *this);
}