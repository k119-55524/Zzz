#pragma once


#include "Window.h"

#include "../../../header.h"

namespace zzz::engine
{
	class WinAndroid final : public Window
	{
	public:
		struct AndroidActinityCtx
		{
			WinAndroid*	window;
			Input*		input;
		};

		WinAndroid() = delete;
		WinAndroid(const std::shared_ptr<Platform> platform, const std::shared_ptr<Input> input);
		~WinAndroid() override;

		[[nodiscard]] virtual std::expected<void, std::string> Initialize(const std::string_view appName) override;

		void ProcessAppCmd(int32_t cmd);

	private:
		AndroidActinityCtx m_Ctx;
	};
}
