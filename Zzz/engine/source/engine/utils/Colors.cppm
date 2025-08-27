#include "pch.h"
export module Colors;

export namespace zzz
{
	export namespace Colors
	{
		struct /*alignas(16)*/ Color
		{
			union {
				float f[4];
				struct { float r, g, b, a; };
				__m128 simd;
			};

			// Конструкторы
			constexpr Color() noexcept : f{ 0.f, 0.f, 0.f, 1.f } {}
			constexpr Color(float r, float g, float b, float a = 1.f) noexcept : r(r), g(g), b(b), a(a) {}
			constexpr Color(const float arr[4]) noexcept : f{ arr[0], arr[1], arr[2], arr[3] } {}
			constexpr Color(__m128 simd_val) noexcept : simd(simd_val) {}

			constexpr float& operator[](size_t index) noexcept { assert(index < 4); return f[index]; }
			constexpr float operator[](size_t index) const noexcept { assert(index < 4); return f[index]; }
			constexpr operator std::array<float, 4>() const noexcept {
				return std::array<float, 4>{ f[0], f[1], f[2], f[3] };
			}

			static Color from_simd(__m128 value) noexcept { return Color(value); }
			__m128 to_simd() const noexcept { return _mm_load_ps(f); }

			constexpr bool operator==(const Color& other) const noexcept { return r == other.r && g == other.g && b == other.b && a == other.a; }
			constexpr bool operator!=(const Color& other) const noexcept { return !(*this == other); }
		};

		// Предопределенные цвета
		constexpr Color AliceBlue{ 0.941176534f, 0.972549081f, 1.f };
		constexpr Color AntiqueWhite{ 0.980392218f, 0.921568692f, 0.843137324f };
		constexpr Color Aqua{ 0.f, 1.f, 1.f };
		constexpr Color Aquamarine{ 0.498039246f, 1.f, 0.831372619f };
		constexpr Color Azure{ 0.941176534f, 1.f, 1.f };
		constexpr Color Beige{ 0.960784376f, 0.960784376f, 0.862745166f };
		constexpr Color Bisque{ 1.f, 0.894117713f, 0.768627524f };
		constexpr Color Black{ 0.f, 0.f, 0.f };
		constexpr Color BlanchedAlmond{ 1.f, 0.921568692f, 0.803921640f };
		constexpr Color Blue{ 0.f, 0.f, 1.f };
		constexpr Color BlueViolet{ 0.541176498f, 0.168627456f, 0.886274576f };
		constexpr Color Brown{ 0.647058845f, 0.164705887f, 0.164705887f };
		constexpr Color BurlyWood{ 0.870588303f, 0.721568644f, 0.529411793f };
		constexpr Color CadetBlue{ 0.372549027f, 0.619607866f, 0.627451003f };
		constexpr Color Chartreuse{ 0.498039246f, 1.f, 0.f };
		constexpr Color Chocolate{ 0.823529482f, 0.411764741f, 0.117647067f };
		constexpr Color Coral{ 1.f, 0.498039246f, 0.313725501f };
		constexpr Color CornflowerBlue{ 0.392156899f, 0.584313750f, 0.929411829f };
		constexpr Color Cornsilk{ 1.f, 0.972549081f, 0.862745166f };
		constexpr Color Crimson{ 0.862745166f, 0.078431375f, 0.235294133f };
		constexpr Color Cyan{ 0.f, 1.f, 1.f };
		constexpr Color DarkBlue{ 0.f, 0.f, 0.545098066f };
		constexpr Color DarkCyan{ 0.f, 0.545098066f, 0.545098066f };
		constexpr Color DarkGoldenrod{ 0.721568644f, 0.525490224f, 0.043137256f };
		constexpr Color DarkGray{ 0.662745118f, 0.662745118f, 0.662745118f };
		constexpr Color DarkGreen{ 0.f, 0.392156899f, 0.f };
		constexpr Color DarkKhaki{ 0.741176486f, 0.717647076f, 0.419607878f };
		constexpr Color DarkMagenta{ 0.545098066f, 0.f, 0.545098066f };
		constexpr Color DarkOliveGreen{ 0.333333343f, 0.419607878f, 0.184313729f };
		constexpr Color DarkOrange{ 1.f, 0.549019635f, 0.f };
		constexpr Color DarkOrchid{ 0.600000024f, 0.196078435f, 0.800000072f };
		constexpr Color DarkRed{ 0.545098066f, 0.f, 0.f };
		constexpr Color DarkSalmon{ 0.913725555f, 0.588235319f, 0.478431404f };
		constexpr Color DarkSeaGreen{ 0.560784340f, 0.737254918f, 0.545098066f };
		constexpr Color DarkSlateBlue{ 0.282352954f, 0.239215687f, 0.545098066f };
		constexpr Color DarkSlateGray{ 0.184313729f, 0.309803933f, 0.309803933f };
		constexpr Color DarkTurquoise{ 0.f, 0.807843208f, 0.819607913f };
		constexpr Color DarkViolet{ 0.580392182f, 0.f, 0.827451050f };
		constexpr Color DeepPink{ 1.f, 0.078431375f, 0.576470613f };
		constexpr Color DeepSkyBlue{ 0.f, 0.749019623f, 1.f };
		constexpr Color DimGray{ 0.411764741f, 0.411764741f, 0.411764741f };
		constexpr Color DodgerBlue{ 0.117647067f, 0.564705908f, 1.f };
		constexpr Color Firebrick{ 0.698039234f, 0.133333340f, 0.133333340f };
		constexpr Color FloralWhite{ 1.f, 0.980392218f, 0.941176534f };
		constexpr Color ForestGreen{ 0.133333340f, 0.545098066f, 0.133333340f };
		constexpr Color Fuchsia{ 1.f, 0.f, 1.f };
		constexpr Color Gainsboro{ 0.862745166f, 0.862745166f, 0.862745166f };
		constexpr Color GhostWhite{ 0.972549081f, 0.972549081f, 1.f };
		constexpr Color Gold{ 1.f, 0.843137324f, 0.f };
		constexpr Color Goldenrod{ 0.854902029f, 0.647058845f, 0.125490203f };
		constexpr Color Gray{ 0.501960814f, 0.501960814f, 0.501960814f };
		constexpr Color Green{ 0.f, 0.501960814f, 0.f };
		constexpr Color GreenYellow{ 0.678431392f, 1.f, 0.184313729f };
		constexpr Color Honeydew{ 0.941176534f, 1.f, 0.941176534f };
		constexpr Color HotPink{ 1.f, 0.411764741f, 0.705882370f };
		constexpr Color IndianRed{ 0.803921640f, 0.360784322f, 0.360784322f };
		constexpr Color Indigo{ 0.294117659f, 0.f, 0.509803951f };
		constexpr Color Ivory{ 1.f, 1.f, 0.941176534f };
		constexpr Color Khaki{ 0.941176534f, 0.901960850f, 0.549019635f };
		constexpr Color Lavender{ 0.901960850f, 0.901960850f, 0.980392218f };
		constexpr Color LavenderBlush{ 1.f, 0.941176534f, 0.960784376f };
		constexpr Color LawnGreen{ 0.486274540f, 0.988235354f, 0.f };
		constexpr Color LemonChiffon{ 1.f, 0.980392218f, 0.803921640f };
		constexpr Color LightBlue{ 0.678431392f, 0.847058892f, 0.901960850f };
		constexpr Color LightCoral{ 0.941176534f, 0.501960814f, 0.501960814f };
		constexpr Color LightCyan{ 0.878431439f, 1.f, 1.f };
		constexpr Color LightGoldenrodYellow{ 0.980392218f, 0.980392218f, 0.823529482f };
		constexpr Color LightGray{ 0.827451050f, 0.827451050f, 0.827451050f };
		constexpr Color LightGreen{ 0.564705908f, 0.933333397f, 0.564705908f };
		constexpr Color LightPink{ 1.f, 0.713725507f, 0.756862819f };
		constexpr Color LightSalmon{ 1.f, 0.627451003f, 0.478431404f };
		constexpr Color LightSeaGreen{ 0.125490203f, 0.698039234f, 0.666666687f };
		constexpr Color LightSkyBlue{ 0.529411793f, 0.807843208f, 0.980392218f };
		constexpr Color LightSlateGray{ 0.466666698f, 0.533333361f, 0.600000024f };
		constexpr Color LightSteelBlue{ 0.690196097f, 0.768627524f, 0.870588303f };
		constexpr Color LightYellow{ 1.f, 1.f, 0.878431439f };
		constexpr Color Lime{ 0.f, 1.f, 0.f };
		constexpr Color LimeGreen{ 0.196078435f, 0.803921640f, 0.196078435f };
		constexpr Color Linen{ 0.980392218f, 0.941176534f, 0.901960850f };
		constexpr Color Magenta{ 1.f, 0.f, 1.f };
		constexpr Color Maroon{ 0.501960814f, 0.f, 0.f };
		constexpr Color MediumAquamarine{ 0.400000036f, 0.803921640f, 0.666666687f };
		constexpr Color MediumBlue{ 0.f, 0.f, 0.803921640f };
		constexpr Color MediumOrchid{ 0.729411781f, 0.333333343f, 0.827451050f };
		constexpr Color MediumPurple{ 0.576470613f, 0.439215720f, 0.858823597f };
		constexpr Color MediumSeaGreen{ 0.235294133f, 0.701960802f, 0.443137288f };
		constexpr Color MediumSlateBlue{ 0.482352942f, 0.407843173f, 0.933333397f };
		constexpr Color MediumSpringGreen{ 0.f, 0.980392218f, 0.603921592f };
		constexpr Color MediumTurquoise{ 0.282352954f, 0.819607913f, 0.800000072f };
		constexpr Color MediumVioletRed{ 0.780392230f, 0.082352944f, 0.521568656f };
		constexpr Color MidnightBlue{ 0.098039225f, 0.098039225f, 0.439215720f };
		constexpr Color MintCream{ 0.960784376f, 1.f, 0.980392218f };
		constexpr Color MistyRose{ 1.f, 0.894117713f, 0.882353008f };
		constexpr Color Moccasin{ 1.f, 0.894117713f, 0.709803939f };
		constexpr Color NavajoWhite{ 1.f, 0.870588303f, 0.678431392f };
		constexpr Color Navy{ 0.f, 0.f, 0.501960814f };
		constexpr Color OldLace{ 0.992156923f, 0.960784376f, 0.901960850f };
		constexpr Color Olive{ 0.501960814f, 0.501960814f, 0.f };
		constexpr Color OliveDrab{ 0.419607878f, 0.556862772f, 0.137254909f };
		constexpr Color Orange{ 1.f, 0.647058845f, 0.f };
		constexpr Color OrangeRed{ 1.f, 0.270588249f, 0.f };
		constexpr Color Orchid{ 0.854902029f, 0.439215720f, 0.839215755f };
		constexpr Color PaleGoldenrod{ 0.933333397f, 0.909803987f, 0.666666687f };
		constexpr Color PaleGreen{ 0.596078455f, 0.984313786f, 0.596078455f };
		constexpr Color PaleTurquoise{ 0.686274529f, 0.933333397f, 0.933333397f };
		constexpr Color PaleVioletRed{ 0.858823597f, 0.439215720f, 0.576470613f };
		constexpr Color PapayaWhip{ 1.f, 0.937254965f, 0.835294187f };
		constexpr Color PeachPuff{ 1.f, 0.854902029f, 0.725490212f };
		constexpr Color Peru{ 0.803921640f, 0.521568656f, 0.246078432f };
		constexpr Color Pink{ 1.f, 0.752941251f, 0.796078503f };
		constexpr Color Plum{ 0.866666734f, 0.627451003f, 0.866666734f };
		constexpr Color PowderBlue{ 0.690196097f, 0.878431439f, 0.901960850f };
		constexpr Color Purple{ 0.501960814f, 0.f, 0.501960814f };
		constexpr Color Red{ 1.f, 0.f, 0.f };
		constexpr Color RosyBrown{ 0.737254918f, 0.560784340f, 0.560784340f };
		constexpr Color RoyalBlue{ 0.254901975f, 0.411764741f, 0.882353008f };
		constexpr Color SaddleBrown{ 0.545098066f, 0.270588249f, 0.074509807f };
		constexpr Color Salmon{ 0.980392218f, 0.501960814f, 0.447058856f };
		constexpr Color SandyBrown{ 0.956862807f, 0.643137276f, 0.376470625f };
		constexpr Color SeaGreen{ 0.180392161f, 0.545098066f, 0.341176480f };
		constexpr Color SeaShell{ 1.f, 0.960784376f, 0.933333397f };
		constexpr Color Sienna{ 0.627451003f, 0.321568638f, 0.176470593f };
		constexpr Color Silver{ 0.752941251f, 0.752941251f, 0.752941251f };
		constexpr Color SkyBlue{ 0.529411793f, 0.807843208f, 0.921568692f };
		constexpr Color SlateBlue{ 0.415686309f, 0.352941185f, 0.803921640f };
		constexpr Color SlateGray{ 0.439215720f, 0.501960814f, 0.564705908f };
		constexpr Color Snow{ 1.f, 0.980392218f, 0.980392218f };
		constexpr Color SpringGreen{ 0.f, 1.f, 0.498039246f };
		constexpr Color SteelBlue{ 0.274509817f, 0.509803951f, 0.705882370f };
		constexpr Color Tan{ 0.823529482f, 0.705882370f, 0.549019635f };
		constexpr Color Teal{ 0.f, 0.501960814f, 0.501960814f };
		constexpr Color Thistle{ 0.847058892f, 0.749019623f, 0.847058892f };
		constexpr Color Tomato{ 1.f, 0.388235331f, 0.278431386f };
		constexpr Color Transparent{ 0.f, 0.f, 0.f, 0.f };
		constexpr Color Turquoise{ 0.250980407f, 0.878431439f, 0.815686345f };
		constexpr Color Violet{ 0.933333397f, 0.509803951f, 0.933333397f };
		constexpr Color Wheat{ 0.960784376f, 0.870588303f, 0.701960802f };
		constexpr Color White{ 1.f, 1.f, 1.f };
		constexpr Color WhiteSmoke{ 0.960784376f, 0.960784376f, 0.960784376f };
		constexpr Color Yellow{ 1.f, 1.f, 0.f };
		constexpr Color YellowGreen{ 0.603921592f, 0.803921640f, 0.196078435f };
	}
}
