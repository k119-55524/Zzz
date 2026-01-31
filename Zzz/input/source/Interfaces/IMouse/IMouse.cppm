
export module IMouse;

namespace zzz::input
{
	/// <summary>
	/// Интерфейс для работы с мышью
	/// </summary>
	export class IMouse
	{
	public:
		explicit IMouse() = default;
		virtual ~IMouse() = 0;

		void OnMouseEnter(bool enter);
		void OnMouseDelta(zI32 dX, zI32 dY);
		void OnMouseButtonsChanged(MouseButtonMask pressed, MouseButtonMask released);
		void OnMouseWheelVertical(zI32 delta);
		void OnMouseWheelHorizontal(zI32 delta);

		// Максимально поддерживаемое количество кнопок
		static constexpr zU8 MaxMouseButtonCount = 5;

	protected:
		static constexpr const wchar_t* buttonNames[MaxMouseButtonCount] = { L"Left", L"Right", L"Middle", L"Button4", L"Button5" };
	};

	IMouse::~IMouse() {};

	void IMouse::OnMouseEnter(bool enter)
	{
		//DebugOutput(std::format(L">>>>> [IMouse::OnMouseEnter()]. Enter: {}", enter ? L"true" : L"false"));
	}

	void IMouse::OnMouseDelta(zI32 dX, zI32 dY)
	{
		//DebugOutput(std::format(L">>>>> [IMouse::OnMouseDelta()]. Move(delta): {}, {}", dX, dY));
	}

	void IMouse::OnMouseButtonsChanged(MouseButtonMask pressed, MouseButtonMask released)
	{
		for (int i = 0; i < MaxMouseButtonCount; i++)
		{
			MouseButtonMask mask = static_cast<MouseButtonMask>(1 << i);

			if (static_cast<uint8_t>(pressed & mask) != 0)
			{
				//DebugOutput(std::format(L">>>>> [IMouse::OnMouseButtonsChanged()]. Press: {}", buttonNames[i]));
			}

			if (static_cast<uint8_t>(released & mask) != 0)
			{
				//DebugOutput(std::format(L">>>>> [IMouse::OnMouseButtonsChanged()]. Released: {}", buttonNames[i]));
			}
		}
	}

	void IMouse::OnMouseWheelVertical(zI32 delta)
	{
		DebugOutput(std::format(L">>>>> [IMouse::OnMouseWheelVertical()]. Ddelta: {}", delta));
	}

	void IMouse::OnMouseWheelHorizontal(zI32 delta)
	{
		DebugOutput(std::format(L">>>>> [IMouse::OnMouseWheelHorizontal()]. Delta: {}", delta));
	}
}