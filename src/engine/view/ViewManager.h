#pragma once

#include "engine/view/View.h"
#include "engine/gapi/IGAPI.h"
#include "engine/platforms/Platform.h"
#include "engine/package/PackageManager.h"
#include "engine/package/UserSettingsManager.h"

namespace zzz::engine
{
	using namespace zzz::core;

	class ViewManager final
	{
		Z_NO_MOVE(ViewManager);

	public:
		ViewManager() = delete;
		ViewManager(const Platform& platform, std::shared_ptr<IGAPI> gapi, std::function<void()> onAllViewsClosed);
		~ViewManager();

		[[nodiscard]] inline std::shared_ptr<IGAPI> GetGAPI() const noexcept { return m_GAPI; }

		/**
		 * @brief Создаёт главный View приложения из стартового ресурса пакета и пользовательских настроек окна.
		 * @details Берёт GUID'ы ViewScript из PackageManager, а параметры native window — из UserSettingsManager.
		 */
		[[nodiscard]] std::expected<std::shared_ptr<View>, std::string> CreateStartView(const PackageManager& packageManager, const UserSettingsManager& userSettingsManager);

		/**
		 * @brief Создаёт View с native window и привязанными ViewScript.
		 * @param settings Параметры создания native window.
		 * @param scripts GUID'ы ViewScript, которые будут созданы и подключены к ViewEventBus этого View.
		 */
		[[nodiscard]] std::expected<std::shared_ptr<View>, std::string> CreateView(const StartViewPlatformData& settings, const std::vector<Guid>& scripts);
#if Z_EDITOR
		[[nodiscard]] std::expected <std::shared_ptr<View>, std::string> CreateView(void* data);
		void RemoveView(View* view);
#endif

		void Update(const Time& time);

	private:
		const Platform& m_Platform;
		std::shared_ptr<IGAPI> m_GAPI;
		std::list<std::shared_ptr<View>> m_Views;

		std::function<void()> OnAllViewsClosed;
		void OnWindowClose(View& view);
	};
}
