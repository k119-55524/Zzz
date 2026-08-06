#pragma once

#include "engine/EngineIncludes.h"
#include "engine/package/UserSettingsManager.h"

namespace zzz::engine
{
	using namespace zzz::core;

	class IGAPI
	{
	public:
		explicit IGAPI(std::shared_ptr<UserSettingsManager> userSettings, eGAPIType type);
		virtual ~IGAPI() = default;

		[[nodiscard]] inline eInitState GetInitState() const noexcept { return m_InitState; }
		[[nodiscard]] constexpr eGAPIType GetGAPIType() const noexcept { return m_GAPIType; }

		[[nodiscard]] virtual std::expected<void, std::string> Initialize();
		virtual void SubmitCommandLists() = 0;
		virtual void BeginRender() = 0;
		virtual void EndRender() = 0;

	protected:
		[[nodiscard]] virtual std::expected<void, std::string> Init() = 0;
		virtual void WaitForGpu() = 0;

		std::shared_ptr<UserSettingsManager> m_UserSettings;
		eGAPIType m_GAPIType;
		eInitState m_InitState;
	};
}
