
export module ViewSetup;

export import Colors;

using namespace zzz::colors;

namespace zzz
{
	export class ViewSetup
	{
	public:
		ViewSetup() = delete;
		ViewSetup(bool isClearrDepth) :
			m_SurfClearType{ eSurfClearType::None },
			m_ClearColor{ colors::DarkMidnightBlue },
			b_IsClearDepth{ isClearrDepth }
		{
		}

		inline void ActivateClearColor(const Color& color) noexcept { m_ClearColor = color; m_SurfClearType = eSurfClearType::Color; }

		inline eSurfClearType GetSurfClearType() const noexcept { return m_SurfClearType; }
		inline const Color& GetClearColor() const noexcept { return m_ClearColor; }
		inline bool IsClearDepth() const noexcept { return b_IsClearDepth; }

	protected:
		eSurfClearType m_SurfClearType;
		Color m_ClearColor;
		bool b_IsClearDepth;
	};
}