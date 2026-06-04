#pragma once

#if defined(Z_WINDOWS)

#include "../../../../header.h"
#include "../../Serialize/Serializer.h"

namespace zzz::engine
{
	class ConfigMSWin final : public ISerializable
	{
	public:
		ConfigMSWin();
		~ConfigMSWin() = default;

		inline const std::string& GetIcoResourceName() const noexcept { return m_IcoResourceName; }
		inline const std::string& GetClassName() const noexcept { return m_ClassName; }

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override;
		[[nodiscard]] std::expected<void, std::string> DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override;

		std::string m_IcoResourceName;
		std::string m_ClassName;
	};
}

#endif // Z_WINDOWS