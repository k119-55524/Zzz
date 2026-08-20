
#include "View.h"
#include "ViewManager.h"
#include "../gapi/IGAPI.h"
#include "../platforms/Platform.h"
#include "../package/PackageManager.h"
#include "../package/UserSettingsManager.h"

using namespace zzz::core;
using namespace zzz::engine;

ViewManager::ViewManager(const Platform& platform, std::shared_ptr<IGAPI> gapi, std::shared_ptr<ScriptFactory> scriptFactory, std::shared_ptr<UserSettingsManager> userSettingsManager, std::function<void()> onAllViewsClosed) :
	m_Platform{ platform },
	m_GAPI{ std::move(gapi) },
	m_ScriptFactory{ std::move(scriptFactory) },
	m_UserSettingsManager{ std::move(userSettingsManager) },
	OnAllViewsClosed{ std::move(onAllViewsClosed) }
{
	ensure(m_GAPI != nullptr, "IGAPI не должен быть null.");
	ensure(m_ScriptFactory != nullptr, "ScriptFactory не должен быть null.");
	ensure(m_UserSettingsManager != nullptr, "UserSettingsManager не должен быть null.");
	ensure(OnAllViewsClosed != nullptr, "OnAllViewsClosed не должен быть null.");
}

ViewManager::~ViewManager()
{
	m_Views.clear();
}

std::expected<std::shared_ptr<View>, std::string> ViewManager::CreateStartView(const PackageManager& packageManager, const UserSettingsManager& userSettingsManager)
{
	if (m_PrimaryView)
		return std::unexpected("Первичное (Основное) окно приложения уже создано.");

	auto startViewData = packageManager.GetStartViewData();
	if (!startViewData)
	{
		std::string err = std::format("Обязательный ресурс StartViewData не найден в пакете: {}", startViewData.error());
		DOutError("{}", err);
		return std::unexpected(err);
	}

	const auto& startUserData = userSettingsManager.GetStartViewUserData();
	const auto startState = startUserData.GetPlatformData().GetWindowState();
	ensure(startState != eWindowState::Closed && startState != eWindowState::Minimized,
		"КРИТИЧЕСКАЯ ОШИБКА РАЗРАБОТЧИКА: Попытка создать стартовое окно в статусе Closed или Minimized!");

	auto res = CreateView(startUserData.GetViewGuid(), startUserData.GetPlatformData(), startViewData->GetUiScriptGuids());
	if (res)
	{
		m_PrimaryView = *res;
	}
	return res;
}

std::expected<std::shared_ptr<View>, std::string> ViewManager::CreateChildView(Guid viewGuid, const ViewPlatformData& settings, const std::vector<Guid>& scripts)
{
#if Z_MOBILE
	THROW_RUNTIME("Мобильные платформы (Android/iOS) поддерживают только одно Основное окно.");
#else
	ensure(m_PrimaryView != nullptr, "Дочернее окно не может быть создано до создания Основного окна.");

	std::shared_ptr<View> view;
	try
	{
		view = safe_make_shared<View>(std::move(viewGuid), settings, scripts, m_Platform, *m_ScriptFactory, m_UserSettingsManager, [this](View& v) { OnWindowClose(v); }, m_PrimaryView.get());
		m_ChildViews.push_back(view);
		m_Views.push_back(view);
		view->InvokeStart();
	}
	catch (const std::exception& e)
	{
		return std::unexpected(e.what());
	}

	return view;
#endif
}

std::expected<std::shared_ptr<View>, std::string> ViewManager::CreateIndependentView(Guid viewGuid, const ViewPlatformData& settings, const std::vector<Guid>& scripts)
{
#if Z_MOBILE
	THROW_RUNTIME("Мобильные платформы (Android/iOS) поддерживают только одно Основное окно.");
#else
	std::shared_ptr<View> view;
	try
	{
		view = safe_make_shared<View>(std::move(viewGuid), settings, scripts, m_Platform, *m_ScriptFactory, m_UserSettingsManager, [this](View& v) { OnWindowClose(v); });
		m_IndependentViews.push_back(view);
		m_Views.push_back(view);
		view->InvokeStart();
	}
	catch (const std::exception& e)
	{
		return std::unexpected(e.what());
	}

	return view;
#endif
}

std::expected<std::shared_ptr<View>, std::string> ViewManager::CreateView(Guid viewGuid, const ViewPlatformData& settings, const std::vector<Guid>& scripts)
{
#if Z_MOBILE
	if (m_Views.size() >= 1)
		THROW_RUNTIME("Мобильные платформы поддерживают только одно нативное окно на приложение.");
#endif

	std::shared_ptr<View> view;
	try
	{
		view = safe_make_shared<View>(std::move(viewGuid), settings, scripts, m_Platform, *m_ScriptFactory, m_UserSettingsManager, [this](View& v) { OnWindowClose(v); });
		m_Views.push_back(view);
		view->InvokeStart();
	}
	catch (const std::exception& e)
	{
		return std::unexpected(e.what());
	}

	return view;
}



void ViewManager::OnWindowClose(View& view)
{
	if (m_PrimaryView.get() == &view)
	{
		DOut("[ViewManager] Закрывается Первичное (Основное) окно приложения.");

		m_ChildViews.clear();
		m_IndependentViews.clear();
		m_Views.clear();
		m_PrimaryView = nullptr;

		if (OnAllViewsClosed)
			OnAllViewsClosed();

		return;
	}

	std::erase_if(m_ChildViews, [&view](const auto& v) { return v.get() == &view; });
	std::erase_if(m_IndependentViews, [&view](const auto& v) { return v.get() == &view; });

	auto it = std::ranges::find_if(
		m_Views,
		[&view](const auto& p)
		{
			return p.get() == &view;
		});

	if (it != m_Views.end())
		m_Views.erase(it);
}

#if Z_EDITOR
[[nodiscard]] std::expected <std::shared_ptr<View>, std::string> ViewManager::CreateView(void* data)
{
	auto view = safe_make_shared<View>(m_Platform, data);
	m_Views.push_back(std::move(view));

	return view;
}

void ViewManager::RemoveView(View* view)
{
	if (!view)
		return;

	auto it = std::ranges::find_if(
		m_Views,
		[view](const auto& p)
		{
			return p.get() == view;
		});

	if (it != m_Views.end())
		m_Views.erase(it);
}
#endif // Z_EDITOR

void ViewManager::Update(const Time& time)
{
	for (const auto& view : m_Views)
	{
		view->Update(time);
	}
}
