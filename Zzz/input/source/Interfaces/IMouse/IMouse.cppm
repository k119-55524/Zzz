
export module IMouse;

namespace zzz::input
{
	export class IMouse
	{
	public:
		explicit IMouse() = default;
		virtual ~IMouse() = 0;

		void OnMouseEnter();
		void OnMouseLeave();

		//void OnMouseMove(int x, int y);
		//void OnMouseDown(int button, int x, int y);
		//void OnMouseUp(int button, int x, int y);
		//void OnMouseWheel(int delta, int x, int y);
	};

	IMouse::~IMouse() {};

	void IMouse::OnMouseEnter()
	{
	}

	void IMouse::OnMouseLeave()
	{
	}
}