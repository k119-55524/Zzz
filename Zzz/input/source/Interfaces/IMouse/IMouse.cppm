
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
	};

	IMouse::~IMouse() {};

	void IMouse::OnMouseEnter(bool enter)
	{
		//DebugOutput(std::format(L">>>>> [IMouse::OnMouseEnter()]. Mouse enter state changed: {}", enter ? L"true" : L"false"));
	}
}