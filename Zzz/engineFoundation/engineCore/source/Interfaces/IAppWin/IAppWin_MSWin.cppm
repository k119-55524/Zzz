
export module IAppWin_MSWin;

export namespace zzz::core
{
	export class IAppWin_MSWin
	{
#if defined(ZPLATFORM_MSWINDOWS)
		Z_NO_COPY_MOVE(IAppWin_MSWin);

	public:
		IAppWin_MSWin() = default;
		virtual ~IAppWin_MSWin() = default;

		virtual const HWND GetHWND() const noexcept = 0;
		virtual void SetCaptionText(std::wstring caption) = 0;
		virtual void AddCaptionText(std::wstring caption) = 0;
	};
#endif // defined(ZPLATFORM_MSWINDOWS)
}