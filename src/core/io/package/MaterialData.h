#pragma once

#include <string>
#include <vector>
#include <span>
#include <cstddef>
#include <expected>

#include "core/utils/Export.h"
#include "core/serialize/Serializer.h"

namespace zzz::core
{
	/**
	 * @class MaterialData
	 * @brief Сериализуемый бинарный контейнер данных материала в data.dat.
	 */
	class Z_CORE_API MaterialData final : public ISerializable
	{
	public:
		MaterialData() = default;
		explicit MaterialData(std::string name);

		[[nodiscard]] const std::string& GetName() const noexcept { return m_Name; }

	protected:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const override;
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer) override;

	private:
		std::string m_Name;
	};
}
