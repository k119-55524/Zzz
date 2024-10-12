#pragma once

#include "ISysMB.h"

#ifdef _WINDOWS
namespace Zzz::Platforms
{
	class MB_MSWin : public ISysMB
	{
		public:
			~MB_MSWin() override;

			void ShowError(const zStr& message) override { MessageBox(NULL, message.c_str(), L"Error!!!", MB_ICONERROR | MB_OK); };
	};
}
#endif // _WINDOWS