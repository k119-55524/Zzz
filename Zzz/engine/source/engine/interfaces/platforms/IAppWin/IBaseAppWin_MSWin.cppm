
#include "pch.h"
export module IBaseAppWin_MSWin;

export namespace zzz::platforms::mswin
{
	export class IBaseAppWin_MSWin
	{
#if defined(ZPLATFORM_MSWINDOWS)
	public:
		IBaseAppWin_MSWin() = default;
		IBaseAppWin_MSWin(const IBaseAppWin_MSWin&) = delete;
		IBaseAppWin_MSWin(IBaseAppWin_MSWin&&) = delete;
		IBaseAppWin_MSWin& operator=(const IBaseAppWin_MSWin&) = delete;
		IBaseAppWin_MSWin& operator=(IBaseAppWin_MSWin&&) = delete;

		virtual ~IBaseAppWin_MSWin() = default;

		virtual const HWND GetHWND() const noexcept = 0;
		virtual void SetCaptionText(std::wstring caption) = 0;
		virtual void AddCaptionText(std::wstring caption) = 0;
	};
#endif // defined(ZPLATFORM_MSWINDOWS)
}