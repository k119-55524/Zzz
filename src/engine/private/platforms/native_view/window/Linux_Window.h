#pragma once

#if defined(Z_LINUX)

#include "IWindow.h"
#include "../../../../header.h"
#include "../../../core/config/EngineConfig.h"

#include "IWindow.h"

namespace zzz::engine
{
	class Linux_Window final : public IWindow
	{
	public:
		Linux_Window() = delete;
		Linux_Window(const EngineConfig& config);
		~Linux_Window() = default;

		[[nodiscard]] virtual std::expected<void, std::string> Initialize(const std::string_view appName) override;

	private:
	};
}
#endif // defined(Z_LINUX)