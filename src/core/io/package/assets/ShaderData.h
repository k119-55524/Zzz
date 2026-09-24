#pragma once

#include <string>
#include <vector>
#include <span>
#include <cstddef>
#include <expected>

#include "core/utils/Export.h"
#include "core/serialize/Serializer.h"
#include "core/enums/eDataDatType.h"
#include "core/enums/eEngineResourceType.h"

namespace zzz::core
{
	/**
	 * @class ShaderData
	 * @brief Сериализуемый бинарный контейнер данных шейдера в data.dat.
	 */
	class Z_CORE_API ShaderData final : public ISerializable
	{
	public:
		static constexpr eDataDatType c_DataDatType = eDataDatType::Shader;
		static constexpr eEngineResourceType c_ResourceType = eEngineResourceType::Shader;

		ShaderData() = default;
		explicit ShaderData(std::string name);

		[[nodiscard]] const std::string& GetName() const noexcept { return m_Name; }

	protected:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const override;
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer) override;

	private:
		std::string m_Name;
	};
}
