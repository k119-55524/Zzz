#pragma once

#include <format>
#include <string>

#include <foundation.h>
#include "../serialize/Serializer.h"

namespace zzz::engine
{
	/**
	* @class size2D
	* @brief РЁР°Р±Р»РѕРЅРЅС‹Р№ РєР»Р°СЃСЃ РґР»СЏ С…СЂР°РЅРµРЅРёСЏ Рё СѓРїСЂР°РІР»РµРЅРёСЏ РґРІСѓРјРµСЂРЅС‹РјРё СЂР°Р·РјРµСЂР°РјРё.
	*
	* Р­С‚РѕС‚ РєР»Р°СЃСЃ РїСЂРµРґРЅР°Р·РЅР°С‡РµРЅ РґР»СЏ СЂР°Р±РѕС‚С‹ СЃ С€РёСЂРёРЅРѕР№ Рё РІС‹СЃРѕС‚РѕР№, РїРѕРґРґРµСЂР¶РёРІР°СЏ СЂР°Р·Р»РёС‡РЅС‹Рµ С‡РёСЃР»РѕРІС‹Рµ С‚РёРїС‹
	* (РЅР°РїСЂРёРјРµСЂ, int, unsigned long, float). РќР°СЃР»РµРґСѓРµС‚СЃСЏ РѕС‚ zSerialize РґР»СЏ РїРѕРґРґРµСЂР¶РєРё
	* СЃРµСЂРёР°Р»РёР·Р°С†РёРё Рё РґРµСЃРµСЂРёР°Р»РёР·Р°С†РёРё.
	*
	* @tparam T РўРёРї РґР°РЅРЅС‹С… РґР»СЏ С…СЂР°РЅРµРЅРёСЏ С€РёСЂРёРЅС‹ Рё РІС‹СЃРѕС‚С‹ (РґРѕР»Р¶РµРЅ Р±С‹С‚СЊ Р°СЂРёС„РјРµС‚РёС‡РµСЃРєРёРј).
	*			Р—РЅР°С‡РµРЅРёРµ РїРѕ СѓРјРѕР»С‡Р°РЅРёСЋ: zU64.
	*/
	template<typename T = zU32, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
	class Size2D final : public ISerializable
	{
	public:
		constexpr Size2D() : width{ 0 }, height{ 0 } {}
		explicit constexpr Size2D(T size) : width{ size }, height{ size } {}
		constexpr Size2D(T _width, T _height) : width{ _width }, height{ _height } {}
		constexpr Size2D(const Size2D& size) : width{ size.width }, height{ size.height } {}
		constexpr Size2D(Size2D&&) = default;

		inline void Set(T _width, T _height) noexcept { width = _width; height = _height; }
		template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
		inline void SetFrom(U w, U h)
		{
			width = static_cast<T>(w);
			height = static_cast<T>(h);
		}
		template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
		inline void SetFrom(const Size2D<U>& other)
		{
			width = static_cast<T>(other.width);
			height = static_cast<T>(other.height);
		}

		Size2D& operator=(const Size2D&) = default;
		Size2D& operator=(Size2D&&) noexcept = default;

		inline bool operator==(const Size2D& other) const noexcept { return width == other.width && height == other.height; }
		inline bool operator!=(const Size2D& other) const noexcept { return !(*this == other); }

		[[nodiscard]] inline std::string ToString() const noexcept { return std::format("Width: {}, Height: {}", width, height); }

		T width;  // РЁРёСЂРёРЅР° РѕР±СЉРµРєС‚Р°.
		T height; // Р’С‹СЃРѕС‚Р° РѕР±СЉРµРєС‚Р°.

//#if Z_VULKAN
//		Size2D(const VkExtent2D& extent)
//			: width(static_cast<T>(extent.width))
//			, height(static_cast<T>(extent.height))
//		{}
//
//		Size2D& operator=(const VkExtent2D& extent) noexcept
//		{
//			width = static_cast<T>(extent.width);
//			height = static_cast<T>(extent.height);
//			return *this;
//		}
//
//		operator VkExtent2D() const noexcept
//		{
//			return VkExtent2D{
//				static_cast<uint32_t>(width),
//				static_cast<uint32_t>(height)
//			};
//		}
//#endif

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override
		{
			return s.Serialize(buffer, width)
				.and_then([&]() {return s.Serialize(buffer, height); });
		}

		[[nodiscard]] std::expected<void, std::string> DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override
		{
			return s.DeSerialize(buffer, offset, width)
				.and_then([&]() {return s.DeSerialize(buffer, offset, height); });
		}
	};
}