#pragma once

#include "MessageBoxBase.h"

#if defined(_WINDOWS)
namespace Zzz::Platforms
{
	class MessageBoxMSWindows : public MessageBoxBase
	{
		public:
			void ShowError(const zStr& message) override { MessageBox(NULL, message.c_str(), L"Error!!!", MB_ICONERROR | MB_OK); };
	};
}
#endif // defined(_WINDOWS)