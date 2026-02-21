
export module ScreenResolution;

import Result;
import Serializer;

export namespace zzz
{
	// Структура для разрешения экрана
	class ScreenResolution : public ISerializable
	{
	public:
		constexpr ScreenResolution(zU32 w, zU32 h) : Width(w), Height(h) {}

		constexpr float GetAspectRatio() const noexcept { return static_cast<float>(Width) / static_cast<float>(Height); }
		constexpr zU32 GetPixelCount() const noexcept { return Width * Height; }
		constexpr ScreenResolution ToLandscape() const noexcept { return Width > Height ? *this : ScreenResolution{ Height, Width }; }
		constexpr ScreenResolution ToPortrait() const noexcept { return Width < Height ? *this : ScreenResolution{ Height, Width }; }

		constexpr bool operator==(const ScreenResolution& other) const noexcept { return Width == other.Width && Height == other.Height; }
		constexpr bool operator!=(const ScreenResolution& other) const noexcept { return !(*this == other); }

		zU32 Width;
		zU32 Height;

	private:
		Result<> Serialize(std::vector<std::byte>& buffer, const zzz::Serializer& s) const override
		{
			return s.Serialize(buffer, Width)
				.and_then([&]() {return s.Serialize(buffer, Height); });
		}

		Result<> DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, const zzz::Serializer& s) override
		{
			return s.DeSerialize(buffer, offset, Width)
				.and_then([&]() {return s.DeSerialize(buffer, offset, Height); });
		}
	};

	// Предопределённые стандартные разрешения
	namespace StandardScreenResolutions
	{
		// ============ DESKTOP ============

		// Классические 4:3
		inline constexpr ScreenResolution VGA{ 640, 480 };          // VGA
		inline constexpr ScreenResolution SVGA{ 800, 600 };         // Super VGA
		inline constexpr ScreenResolution XGA{ 1024, 768 };         // Extended Graphics Array
		inline constexpr ScreenResolution SXGA{ 1280, 1024 };       // Super XGA (5:4)

		// Широкоэкранные 16:10
		inline constexpr ScreenResolution WXGA{ 1280, 800 };        // Widescreen XGA
		inline constexpr ScreenResolution WXGA_Plus{ 1440, 900 };   // WXGA+
		inline constexpr ScreenResolution WSXGA_Plus{ 1680, 1050 }; // WSXGA+
		inline constexpr ScreenResolution WUXGA{ 1920, 1200 };      // Widescreen UXGA
		inline constexpr ScreenResolution WQXGA{ 2560, 1600 };      // Widescreen QXGA

		// Широкоэкранные 16:9 (наиболее популярные)
		inline constexpr ScreenResolution WXGA_16_9{ 1366, 768 };   // Популярное для ноутбуков
		inline constexpr ScreenResolution HD{ 1280, 720 };          // 720p - HD Ready
		inline constexpr ScreenResolution HD_Plus{ 1600, 900 };     // 900p - HD+
		inline constexpr ScreenResolution FullHD{ 1920, 1080 };     // 1080p - Full HD
		inline constexpr ScreenResolution QHD{ 2560, 1440 };        // 1440p - Quad HD / 2K
		inline constexpr ScreenResolution QHD_Plus{ 3200, 1800 };   // QHD+
		inline constexpr ScreenResolution UHD_4K{ 3840, 2160 };     // 4K (2160p) - Ultra HD
		inline constexpr ScreenResolution UHD_8K{ 7680, 4320 };     // 8K (4320p) - на будущее

		// Ультраширокие 21:9
		inline constexpr ScreenResolution UWFHD{ 2560, 1080 };      // UltraWide Full HD
		inline constexpr ScreenResolution UWQHD{ 3440, 1440 };      // UltraWide Quad HD
		inline constexpr ScreenResolution UW5K{ 5120, 2160 };       // UltraWide 5K

		// Сверхширокие 32:9
		inline constexpr ScreenResolution DQHD{ 5120, 1440 };       // Dual QHD (Super UltraWide)

		// Кинематографические
		inline constexpr ScreenResolution DCI_2K{ 2048, 1080 };     // Digital Cinema 2K
		inline constexpr ScreenResolution DCI_4K{ 4096, 2160 };     // Digital Cinema 4K

		// ============ MOBILE (Portrait) ============

		// Старые устройства
		inline constexpr ScreenResolution Mobile_HVGA{ 320, 480 };      // iPhone 3GS, старые Android
		inline constexpr ScreenResolution Mobile_WVGA{ 480, 800 };      // Старые Android
		inline constexpr ScreenResolution Mobile_qHD{ 540, 960 };       // Quarter HD

		// Базовые HD (16:9)
		inline constexpr ScreenResolution Mobile_HD{ 720, 1280 };       // 720p - бюджетные смартфоны
		inline constexpr ScreenResolution Mobile_HD_Plus{ 900, 1600 };  // HD+
		inline constexpr ScreenResolution Mobile_FullHD{ 1080, 1920 };  // 1080p - средний сегмент
		inline constexpr ScreenResolution Mobile_FullHD_Plus{ 1080, 2340 }; // FullHD+ (19.5:9)

		// Quad HD и выше
		inline constexpr ScreenResolution Mobile_QHD{ 1440, 2560 };     // 1440p - флагманы
		inline constexpr ScreenResolution Mobile_QHD_Plus{ 1440, 3040 }; // QHD+ (Samsung S10+)
		inline constexpr ScreenResolution Mobile_QHD_Plus2{ 1440, 3200 }; // QHD+ (Samsung S20+)

		// iPhone (специфичные разрешения)
		inline constexpr ScreenResolution iPhone_4{ 640, 960 };         // iPhone 4/4S (Retina)
		inline constexpr ScreenResolution iPhone_5{ 640, 1136 };        // iPhone 5/5S/SE (1st gen)
		inline constexpr ScreenResolution iPhone_6{ 750, 1334 };        // iPhone 6/7/8/SE (2nd/3rd gen)
		inline constexpr ScreenResolution iPhone_6_Plus{ 1080, 1920 };  // iPhone 6/7/8 Plus
		inline constexpr ScreenResolution iPhone_X{ 1125, 2436 };       // iPhone X/XS/11 Pro
		inline constexpr ScreenResolution iPhone_XR{ 828, 1792 };       // iPhone XR/11
		inline constexpr ScreenResolution iPhone_XS_Max{ 1242, 2688 };  // iPhone XS Max/11 Pro Max
		inline constexpr ScreenResolution iPhone_12{ 1170, 2532 };      // iPhone 12/12 Pro/13/13 Pro/14
		inline constexpr ScreenResolution iPhone_12_Pro_Max{ 1284, 2778 }; // iPhone 12/13/14 Pro Max
		inline constexpr ScreenResolution iPhone_14_Plus{ 1284, 2778 }; // iPhone 14/15 Plus
		inline constexpr ScreenResolution iPhone_15_Pro_Max{ 1290, 2796 }; // iPhone 15 Pro Max

		// Android флагманы (популярные модели)
		inline constexpr ScreenResolution Galaxy_S8{ 1440, 2960 };      // Samsung Galaxy S8/S9 (18.5:9)
		inline constexpr ScreenResolution Galaxy_S10{ 1440, 3040 };     // Samsung Galaxy S10/S20 (19:9)
		inline constexpr ScreenResolution Galaxy_S20_Ultra{ 1440, 3200 }; // Samsung S20 Ultra/S21 Ultra
		inline constexpr ScreenResolution Pixel_5{ 1080, 2340 };        // Google Pixel 5/6/7
		inline constexpr ScreenResolution Pixel_7_Pro{ 1440, 3120 };    // Google Pixel 7 Pro
		inline constexpr ScreenResolution OnePlus_9{ 1080, 2400 };      // OnePlus 9 (20:9)
		inline constexpr ScreenResolution Xiaomi_Mi_11{ 1440, 3200 };   // Xiaomi Mi 11/12

		// Складные устройства
		inline constexpr ScreenResolution Galaxy_Fold_Cover{ 816, 2260 };    // Samsung Fold внешний экран
		inline constexpr ScreenResolution Galaxy_Fold_Inner{ 1768, 2208 };   // Samsung Fold внутренний
		inline constexpr ScreenResolution Galaxy_Z_Flip{ 1080, 2640 };       // Samsung Z Flip (22:9)

		// Планшеты (Portrait)
		inline constexpr ScreenResolution iPad_Mini{ 1536, 2048 };      // iPad mini (4:3)
		inline constexpr ScreenResolution iPad{ 1620, 2160 };           // iPad 10th gen
		inline constexpr ScreenResolution iPad_Air{ 1640, 2360 };       // iPad Air (2022)
		inline constexpr ScreenResolution iPad_Pro_11{ 1668, 2388 };    // iPad Pro 11"
		inline constexpr ScreenResolution iPad_Pro_13{ 2048, 2732 };    // iPad Pro 12.9"
		inline constexpr ScreenResolution Android_Tablet_10{ 1200, 1920 }; // Стандартный 10" Android планшет
	}
}