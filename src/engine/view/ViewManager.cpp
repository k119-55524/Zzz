
#include "ViewManager.h"
#include "../gapi/IGAPI.h"
#include "../platforms/Platform.h"
#include "../package/PackageManager.h"
#include "../package/UserSettingsManager.h"
#include "../platforms/monitor/IMonitorProvider.h"

#include "View.h"

using namespace zzz::core;
using namespace zzz::engine;

ViewManager::ViewManager(const Platform& platform, std::shared_ptr<IGAPI> gapi, std::shared_ptr<ScriptFactory> scriptFactory, std::shared_ptr<PackageManager> packageManager, std::shared_ptr<UserSettingsManager> userSettingsManager, std::function<void()> onAllViewsClosed) :
	m_Platform{ platform },
	m_GAPI{ std::move(gapi) },
	m_ScriptFactory{ std::move(scriptFactory) },
	m_PackageManager{ std::move(packageManager) },
	m_UserSettingsManager{ std::move(userSettingsManager) },
	m_ThreadsUpdate{ "ViewManager", 2 },
	OnAllViewsClosed{ std::move(onAllViewsClosed) }
{
	ensure(m_GAPI != nullptr, "IGAPI не должен быть null.");
	ensure(m_ScriptFactory != nullptr, "ScriptFactory не должен быть null.");
	ensure(m_PackageManager != nullptr, "PackageManager не должен быть null.");
	ensure(m_UserSettingsManager != nullptr, "UserSettingsManager не должен быть null.");
	ensure(OnAllViewsClosed != nullptr, "OnAllViewsClosed не должен быть null.");
}

ViewManager::~ViewManager()
{
	m_Views.clear();
}

std::expected<std::shared_ptr<View>, std::string> ViewManager::CreatePrimaryView()
{
	if (m_PrimaryView)
		return UNEXPECTED("Первичное (Основное) окно приложения уже создано.");

	const auto& primaryUserData = m_UserSettingsManager->GetPrimaryViewUserData();
	const auto primaryState = primaryUserData.GetPlatformData().GetWindowState();
	ensure(primaryState != eWindowState::Closed && primaryState != eWindowState::Minimized, "Стартовое окно не может быть создано в состоянии Closed или Minimized. Проверьте UserSettings.dat.");

	auto primaryViewData = m_PackageManager->GetPrimaryViewData();
	if (!primaryViewData)
	{
		std::string err = std::format("Обязательный ресурс PrimaryViewData не найден в пакете: {}", primaryViewData.error());
		DOutError("{}", err);
		return UNEXPECTED("{}", err);
	}

	ViewPlatformData platformData = primaryUserData.GetPlatformData();
	const auto& monitorProvider = m_Platform.GetMonitorProvider();
	MonitorInfo targetMonitor = monitorProvider.GetMonitorById(platformData.GetMonitorId());
	Rect2D<zI32> targetRect = m_UserSettingsManager->IsFirstRun()
		? monitorProvider.CenterOnWorkArea(platformData.GetWindowRect(), targetMonitor)
		: monitorProvider.FitToWorkArea(platformData.GetWindowRect(), targetMonitor);

	platformData.SetWindowRect(targetRect);
	platformData.SetMonitorId(targetMonitor.GetPlatformMonitorId());

	std::shared_ptr<View> view;
	try
	{
		auto scripts = CreateViewScripts(primaryViewData->GetUiScriptGuids());
		view = safe_make_shared<View>(primaryUserData.GetViewGuid(), platformData, std::move(scripts), m_Platform, m_GAPI, m_UserSettingsManager, [this](View& v) { OnWindowClose(v); });
		m_Views.push_back(view);
		view->InvokeStart();
	}
	catch (const std::exception& e)
	{
		return UNEXPECTED("{}", e.what());
	}

	m_PrimaryView = view;
	return view;
}

std::expected<std::shared_ptr<View>, std::string> ViewManager::CreateChildView(const Guid& viewGuid)
{
#if Z_MOBILE
	THROW_RUNTIME("Мобильные платформы (Android/iOS) поддерживают только одно Основное окно.");
#endif // Z_MOBILE

	ensure(m_PrimaryView != nullptr, "Дочернее окно не может быть создано до создания Основного окна.");

	auto viewDataRes = m_PackageManager->LoadPackageDataByGuid<ViewData>(ePackage::View, viewGuid);
	if (!viewDataRes)
		return UNEXPECTED("Не удалось загрузить ViewData из пакета для GUID '{}': {}", viewGuid.ToString(), viewDataRes.error());

	const auto& childMap = m_UserSettingsManager->GetChildViewsUserData();
	auto it = childMap.find(viewGuid);
	ensure(it != childMap.end(), "Пользовательские настройки для View с GUID '" + viewGuid.ToString() + "' не найдены в UserSettingsManager (ChildViews).");

	const ViewUserData& userData = it->second;

	const auto& monitorProvider = m_Platform.GetMonitorProvider();
	MonitorInfo targetMonitor = monitorProvider.GetMonitorById(userData.GetWindowState().GetNativeState().GetMonitorId());

	ViewPlatformData platformSettings;
	Rect2D<zI32> targetRect = m_UserSettingsManager->IsFirstRun()
		? monitorProvider.CenterOnWorkArea(userData.GetWindowState().GetNativeState().GetWindowRect(), targetMonitor)
		: monitorProvider.FitToWorkArea(userData.GetWindowState().GetNativeState().GetWindowRect(), targetMonitor);
	platformSettings.SetWindowRect(targetRect);
	platformSettings.SetWindowState(userData.GetWindowState().GetNativeState().GetState());
	platformSettings.SetMonitorId(targetMonitor.GetPlatformMonitorId());

	std::shared_ptr<View> view;
	try
	{
		auto viewScripts = CreateViewScripts(viewDataRes->GetUiScriptGuids());
		view = safe_make_shared<View>(viewGuid, platformSettings, std::move(viewScripts), m_Platform, m_GAPI, m_UserSettingsManager, [this](View& v) { OnWindowClose(v); }, m_PrimaryView.get());
		m_ChildViews.push_back(view);
		m_Views.push_back(view);
		view->InvokeStart();
	}
	catch (const std::exception& e)
	{
		return UNEXPECTED("{}", e.what());
	}

	return view;
}

std::expected<std::shared_ptr<View>, std::string> ViewManager::CreateIndependentView(const Guid& viewGuid)
{
#if Z_MOBILE
	THROW_RUNTIME("Мобильные платформы (Android/iOS) поддерживают только одно Основное окно.");
#endif // Z_MOBILE

	auto viewDataRes = m_PackageManager->LoadPackageDataByGuid<ViewData>(ePackage::View, viewGuid);
	if (!viewDataRes)
		return UNEXPECTED("Не удалось загрузить ViewData из пакета для GUID '{}': {}", viewGuid.ToString(), viewDataRes.error());

	const auto& indepMap = m_UserSettingsManager->GetIndependentViewsUserData();
	auto it = indepMap.find(viewGuid);
	ensure(it != indepMap.end(), "Пользовательские настройки для View с GUID '" + viewGuid.ToString() + "' не найдены в UserSettingsManager (IndependentViews).");

	const ViewUserData& userData = it->second;

	const auto& monitorProvider = m_Platform.GetMonitorProvider();
	MonitorInfo targetMonitor = monitorProvider.GetMonitorById(userData.GetWindowState().GetNativeState().GetMonitorId());

	ViewPlatformData platformSettings;
	Rect2D<zI32> targetRect = m_UserSettingsManager->IsFirstRun()
		? monitorProvider.CenterOnWorkArea(userData.GetWindowState().GetNativeState().GetWindowRect(), targetMonitor)
		: monitorProvider.FitToWorkArea(userData.GetWindowState().GetNativeState().GetWindowRect(), targetMonitor);
	platformSettings.SetWindowRect(targetRect);
	platformSettings.SetWindowState(userData.GetWindowState().GetNativeState().GetState());
	platformSettings.SetMonitorId(targetMonitor.GetPlatformMonitorId());

	std::shared_ptr<View> view;
	try
	{
		auto viewScripts = CreateViewScripts(viewDataRes->GetUiScriptGuids());
		view = safe_make_shared<View>(viewGuid, platformSettings, std::move(viewScripts), m_Platform, m_GAPI, m_UserSettingsManager, [this](View& v) { OnWindowClose(v); });
		m_IndependentViews.push_back(view);
		m_Views.push_back(view);
		view->InvokeStart();
	}
	catch (const std::exception& e)
	{
		return UNEXPECTED("{}", e.what());
	}

	return view;
}

std::vector<std::shared_ptr<ViewScript>> ViewManager::CreateViewScripts(const std::vector<Guid>& scriptGuids) const
{
	ensure(m_ScriptFactory != nullptr, "ScriptFactory должен быть инициализирован.");

	std::vector<std::shared_ptr<ViewScript>> viewScripts;
	viewScripts.reserve(scriptGuids.size());
	for (const auto& scriptGuid : scriptGuids)
	{
		auto script = m_ScriptFactory->CreateViewScript(scriptGuid);
		ensure(script != nullptr, "Не удалось создать экземпляр ViewScript с GUID: " + scriptGuid.ToString());
		viewScripts.push_back(std::move(script));
	}

	return viewScripts;
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
	auto view = safe_make_shared<View>(m_Platform, m_GAPI, data);
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
	m_ThreadsUpdate.Submit([this, &time]()
	{
		for (const auto& view : m_Views)
		{
			view->Update(time);
			view->PrepareFrame();
		}
	});

	m_ThreadsUpdate.Submit([this]()
	{
		for (const auto& view : m_Views)
		{
			view->RenderFrame();
		}
	});

	m_ThreadsUpdate.Join();
}
