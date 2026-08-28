
#include "ViewManager.h"
#include "../platforms/Platform.h"
#include "../package/PackageManager.h"
#include "../package/UserSettingsManager.h"
#include "../platforms/monitor/IMonitorProvider.h"

#include "View.h"

using namespace zzz::core;
using namespace zzz::engine;

ViewManager::ViewManager(
	const Platform& platform,
	std::shared_ptr<GAPI> gapi,
	std::shared_ptr<ScriptFactory> scriptFactory,
	std::shared_ptr<PackageManager> packageManager,
	std::shared_ptr<UserSettingsManager> userSettingsManager,
	std::function<void()> onAllViewsClosed
) :
	m_Platform{ platform },
	m_GAPI{ std::move(gapi) },
	m_ScriptFactory{ std::move(scriptFactory) },
	m_PackageManager{ std::move(packageManager) },
	m_UserSettingsManager{ std::move(userSettingsManager) },
	m_ThreadsUpdate{ "ViewManager", 2 },
	OnAllViewsClosed{ std::move(onAllViewsClosed) }
{
	ensure(m_GAPI != nullptr, "GAPI не должен быть null.");
	ensure(m_ScriptFactory != nullptr, "ScriptFactory не должен быть null.");
	ensure(m_PackageManager != nullptr, "PackageManager не должен быть null.");
	ensure(m_UserSettingsManager != nullptr, "UserSettingsManager не должен быть null.");
	ensure(OnAllViewsClosed != nullptr, "OnAllViewsClosed не должен быть null.");
}

ViewManager::~ViewManager()
{
	m_ChildViews.clear();
	m_IndependentViews.clear();
	m_PrimaryView = nullptr;
}

void ViewManager::CreatePrimaryView()
{
	if (m_PrimaryView)
		THROW_RUNTIME("Первичное (Основное) окно приложения уже создано.");

	// 1. Проверяем наличие ресурсов Основного окна в package.dat
	auto primaryViewData = m_PackageManager->GetPrimaryViewData();
	if (!primaryViewData)
		THROW_RUNTIME("Обязательный ресурс PrimaryViewData не найден в пакете: {}", primaryViewData.error());

	// 2. Получаем или создаем прямой блок ViewPlatformData в UserSettingsManager
	const bool isFirstTime = (m_UserSettingsManager->GetPrimaryViewUserData() == nullptr);
	auto* userPlatformData = m_UserSettingsManager->GetOrCreatePrimaryViewPlatformData(primaryViewData->GetViewGuid(), primaryViewData->GetPlatformData());

	m_PrimaryView = CreateViewInstance(*primaryViewData, userPlatformData, isFirstTime);
}

void ViewManager::CreateChildView(const Guid& viewGuid)
{
#if Z_MOBILE
	THROW_RUNTIME("Мобильные платформы (Android/iOS) поддерживают только одно Основное окно.");
#else // Z_MOBILE
	ensure(m_PrimaryView != nullptr, "Дочернее окно не может быть создано до создания Основного окна.");

	// 1. Обязательная проверка существования ресурса ChildView в пакете ресурсов package.dat
	auto viewDataRes = m_PackageManager->LoadPackageDataByGuid<ChildViewData>(ePackage::ChildView, viewGuid);
	if (!viewDataRes)
		THROW_RUNTIME("Не удалось загрузить ChildViewData из пакета для GUID '{}': {}", viewGuid.ToString(), viewDataRes.error());

	// 2. Получаем или создаем прямой блок ViewPlatformData в UserSettingsManager
	const bool isFirstTime = (m_UserSettingsManager->GetChildViewUserData(viewGuid) == nullptr);
	auto* userPlatformData = m_UserSettingsManager->GetOrCreateChildViewPlatformData(viewGuid, viewDataRes->GetPlatformData());

	m_ChildViews.push_back(CreateViewInstance(*viewDataRes, userPlatformData, isFirstTime, m_PrimaryView.get()));
#endif // Z_MOBILE
}

void ViewManager::CreateIndependentView(const Guid& viewGuid)
{
#if Z_MOBILE
	THROW_RUNTIME("Мобильные платформы (Android/iOS) поддерживают только одно Основное окно.");
#else // Z_MOBILE
	ensure(m_PrimaryView != nullptr, "Независимое окно не может быть создано до создания Основного окна.");

	// 1. Обязательная проверка существования ресурса IndependentView в пакете ресурсов package.dat
	auto viewDataRes = m_PackageManager->LoadPackageDataByGuid<IndependentViewData>(ePackage::IndependentView, viewGuid);
	if (!viewDataRes)
		THROW_RUNTIME("Не удалось загрузить IndependentViewData из пакета для GUID '{}': {}", viewGuid.ToString(), viewDataRes.error());

	// 2. Получаем или создаем прямой блок ViewPlatformData в UserSettingsManager
	const bool isFirstTime = (m_UserSettingsManager->GetIndependentViewUserData(viewGuid) == nullptr);
	auto* userPlatformData = m_UserSettingsManager->GetOrCreateIndependentViewPlatformData(viewGuid, viewDataRes->GetPlatformData());

	m_IndependentViews.push_back(CreateViewInstance(*viewDataRes, userPlatformData, isFirstTime));
#endif // Z_MOBILE
}

std::shared_ptr<View> ViewManager::CreateViewInstance(
	const ViewConfigData& viewData,
	ViewPlatformData* userPlatformData,
	bool isFirstTime,
	const View* parentView)
{
	const auto& monitorProvider = m_Platform.GetMonitorProvider();
	MonitorInfo targetMonitor = monitorProvider.GetMonitorById(userPlatformData->GetMonitorId());

	// Если окно новое (isFirstTime) — вычисляем позицию по центру экрана (CenterOnWorkArea).
	// Если окно уже сохранялось в user.dat — проверяем вписанность пользовательских координат (FitToWorkArea).
	Rect2D<zI32> targetRect = isFirstTime
		? monitorProvider.CenterOnWorkArea(userPlatformData->GetWindowRect(), targetMonitor)
		: monitorProvider.FitToWorkArea(userPlatformData->GetWindowRect(), targetMonitor);

	userPlatformData->SetWindowRect(targetRect);
	userPlatformData->SetMonitorId(targetMonitor.GetPlatformMonitorId());

	auto view = safe_make_shared<View>(
		viewData,
		userPlatformData,
		m_ScriptFactory,
		m_Platform,
		m_GAPI,
		[this](View& v) { OnWindowClose(v); },
		parentView
	);

	view->InvokeStart();
	return view;
}

void ViewManager::OnWindowClose(View& view)
{
	m_UserSettingsManager->StoreViewState(view);

	if (m_PrimaryView.get() == &view)
	{
		DOut("[ViewManager] Закрывается Первичное (Основное) окно приложения.");

		m_ChildViews.clear();
		m_IndependentViews.clear();
		m_PrimaryView = nullptr;

		auto res = m_UserSettingsManager->SaveConfig();
		if (!res)
			DOutError("[ViewManager::OnWindowClose] Ошибка при сохранении user.dat: {}", res.error());

		if (OnAllViewsClosed)
			OnAllViewsClosed();

		return;
	}

	std::erase_if(m_ChildViews, [&view](const auto& v) { return v.get() == &view; });
	std::erase_if(m_IndependentViews, [&view](const auto& v) { return v.get() == &view; });
}

#if Z_EDITOR
std::shared_ptr<View> ViewManager::CreateView(void* data)
{
	auto view = safe_make_shared<View>(m_Platform, m_GAPI, data);
	m_IndependentViews.push_back(view);

	return view;
}

void ViewManager::RemoveView(View* view)
{
	if (!view)
		return;

	std::erase_if(m_IndependentViews, [view](const auto& p) { return p.get() == view; });
}
#endif // Z_EDITOR

void ViewManager::Update(const Time& time)
{
	if (!m_PrimaryView)
		return;

	m_PrimaryView->PreRender();
	for (const auto& view : m_ChildViews)
		view->PreRender();
	for (const auto& view : m_IndependentViews)
		view->PreRender();

	m_ThreadsUpdate.SubmitTasks(
		[this, &time]()
		{
			m_PrimaryView->Update(time);
			m_PrimaryView->PrepareFrame();

			for (const auto& view : m_ChildViews)
			{
				view->Update(time);
				view->PrepareFrame();
			}

			for (const auto& view : m_IndependentViews)
			{
				view->Update(time);
				view->PrepareFrame();
			}
		},
		[this]()
		{
			m_PrimaryView->RenderFrame();

			for (const auto& view : m_ChildViews)
				view->RenderFrame();

			for (const auto& view : m_IndependentViews)
				view->RenderFrame();
		}
	);

	m_ThreadsUpdate.Join();

	m_PrimaryView->PostRender();
	for (const auto& view : m_ChildViews)
		view->PostRender();
	for (const auto& view : m_IndependentViews)
		view->PostRender();
}
