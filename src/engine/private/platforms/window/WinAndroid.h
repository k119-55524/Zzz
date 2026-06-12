#pragma once


#include "Window_Common.h"

#include "../../../header.h"

namespace zzz::engine
{
	class WinAndroid final : public WindowBase
	{
	public:
		struct AndroidActinityCtx
		{
			WinAndroid*	window;
			Input*		input;
		};

		WinAndroid() = delete;
		WinAndroid(const std::shared_ptr<Platform> platform, const std::shared_ptr<Input> input, WindowCallbacks callbacks);
		~WinAndroid() override;

		[[nodiscard]] std::expected<void, std::string> Initialize(const std::string_view appName);

		void ProcessAppCmd(int32_t cmd);

	private:
		AndroidActinityCtx m_Ctx;
	};
}
