#pragma once

#if defined(Z_ANDROID)

#include "IWindow.h"

#include "../../../../header.h"

namespace zzz::engine
{
	class WinAndroid final : public IWindow
	{
	public:
		struct AndroidActinityCtx
		{
			WinAndroid*	window;
			IInput*		input;
		};

		WinAndroid() = delete;
		WinAndroid(const std::shared_ptr<IPlatform> platform, const std::shared_ptr<IInput> input);
		~WinAndroid() override;

		[[nodiscard]] virtual std::expected<void, std::string> Initialize(const std::string_view appName) override;

		void ProcessAppCmd(int32_t cmd);

	private:
		AndroidActinityCtx m_Ctx;
	};
}
#endif // defined(Z_ANDROID)
