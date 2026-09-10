
#include <thread>
#include <algorithm>

#include "core/Core.h"
#include "engine/view/View.h"
#include "engine/view/ViewManager.h"
#include "engine/scene/Scene.h"
#include "engine/scene/SceneManager.h"
#include "engine/platforms/Platform.h"
#include "engine/package/PackageManager.h"
#include "core/io/package/ChildViewData.h"
#include "engine/package/UserSettingsManager.h"
#include "core/io/package/IndependentViewData.h"
#include "engine/platforms/monitor/IMonitorProvider.h"

Z_SET_LOG_CATEGORY(::zzz::core::Window);

using namespace zzz::core;
using namespace zzz::engine;

ViewManager::ViewManager(
	const Platform& platform,
	std::shared_ptr<GAPI> gapi,
	std::shared_ptr<ScriptFactory> scriptFactory,
	std::shared_ptr<PackageManager> packageManager,
	std::shared_ptr<UserSettingsManager> userSettingsManager,
	std::shared_ptr<SceneManager> sceneManager,
	std::function<void()> onAllViewsClosed
) :
	m_Platform{ platform },
	m_GAPI{ std::move(gapi) },
	m_ScriptFactory{ std::move(scriptFactory) },
	m_PackageManager{ std::move(packageManager) },
	m_UserSettingsManager{ std::move(userSettingsManager) },
	m_SceneManager{ std::move(sceneManager) },
	m_ThreadsUpdate{ "ViewManager", std::max(2u, std::thread::hardware_concurrency()) },
	OnAllViewsClosed{ std::move(onAllViewsClosed) }
{
	ensure(m_GAPI != nullptr, "GAPI не должен быть null.");
	ensure(m_ScriptFactory != nullptr, "ScriptFactory не должен быть null.");
	ensure(m_PackageManager != nullptr, "PackageManager не должен быть null.");
	ensure(m_UserSettingsManager != nullptr, "UserSettingsManager не должен быть null.");
	ensure(m_SceneManager != nullptr, "SceneManager не должен быть null.");
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

	auto primaryViewData = m_PackageManager->GetPrimaryViewData();
	ensure(primaryViewData.has_value(), "Не удалось получить данные для основного окна.");

	m_PrimaryView = CreateViewInstance(*primaryViewData, ePackage::PrimaryView);
}

void ViewManager::CreateChildView(const Guid& viewGuid)
{
#if Z_MOBILE
	THROW_RUNTIME("Мобильные платформы (Android/iOS) поддерживают только одно Основное окно.");
#else
	ensure(m_PrimaryView != nullptr, "Дочернее окно не может быть создано до создания Основного окна.");

	auto viewDataRes = m_PackageManager->LoadAsset<ChildViewData>(viewGuid);

	ensure(viewDataRes.has_value(), "Не удалось загрузить ChildViewData из пакета для GUID '{}': {}", viewGuid.ToString(), viewDataRes.error());
	m_ChildViews.push_back(CreateViewInstance(*viewDataRes, ePackage::ChildView, m_PrimaryView.get()));
#endif // Z_MOBILE
}

void ViewManager::CreateIndependentView(const Guid& viewGuid)
{
#if Z_MOBILE
	THROW_RUNTIME("Мобильные платформы (Android/iOS) поддерживают только одно Основное окно.");
#else
	ensure(m_PrimaryView != nullptr, "Независимое окно не может быть создано до создания Основного окна.");

	auto viewDataRes = m_PackageManager->LoadAsset<IndependentViewData>(viewGuid);
	ensure(viewDataRes.has_value(), "Не удалось загрузить IndependentViewData из пакета для GUID '{}': {}", viewGuid.ToString(), viewDataRes.error());

	m_IndependentViews.push_back(CreateViewInstance(*viewDataRes, ePackage::IndependentView));
#endif // Z_MOBILE
}

std::shared_ptr<View> ViewManager::CreateViewInstance(
	const ViewConfigData& viewData,
	ePackage viewType,
	const View* parentView)
{
	const Guid& guid = viewData.GetViewGuid();
	bool needsAutoCentering = false;
	ViewPlatformData* userPlatformData = nullptr;

	switch (viewType)
	{
	case ePackage::PrimaryView:
	{
		const auto* primaryUserData = m_UserSettingsManager->GetPrimaryViewUserData();
		needsAutoCentering = (primaryUserData == nullptr || primaryUserData->GetViewGuid() != guid);
		userPlatformData = m_UserSettingsManager->GetOrCreatePrimaryViewPlatformData(guid, viewData.GetPlatformData());
		break;
	}
	case ePackage::ChildView:
	{
		needsAutoCentering = (m_UserSettingsManager->GetChildViewUserData(guid) == nullptr);
		userPlatformData = m_UserSettingsManager->GetOrCreateChildViewPlatformData(guid, viewData.GetPlatformData());
		break;
	}
	case ePackage::IndependentView:
	{
		needsAutoCentering = (m_UserSettingsManager->GetIndependentViewUserData(guid) == nullptr);
		userPlatformData = m_UserSettingsManager->GetOrCreateIndependentViewPlatformData(guid, viewData.GetPlatformData());
		break;
	}
	default:
		THROW_RUNTIME("Неподдерживаемый тип окна для создания экземпляра View: {}", ToString(viewType));
	}

	ensure(userPlatformData != nullptr, "ViewPlatformData не может быть null.");

	const auto& monitorProvider = m_Platform.GetMonitorProvider();
	MonitorInfo targetMonitor = monitorProvider.GetMonitorById(userPlatformData->GetMonitorId());

	// Если окно новое для пользователя (needsAutoCentering) — вычисляем позицию по центру экрана (CenterOnWorkArea).
	// Если окно уже сохранялось в user.dat — проверяем вписанность пользовательских координат (FitToWorkArea).
	Rect2D<zI32> targetRect = needsAutoCentering
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
	SetupSceneAsync(view, viewData.GetSceneGuid());

	return view;
}

void ViewManager::SetupSceneAsync(std::weak_ptr<View> viewWeak, Guid sceneGuid)
{
	m_SceneManager->LoadSceneAsync(sceneGuid, [viewWeak](auto sceneRes)
	{
		auto view = viewWeak.lock();
		if (!view)
			return;

		if (!sceneRes)
		{
			DOutError("[ViewManager] Не удалось загрузить стартовую сцену View: {}", sceneRes.error());
			return;
		}

		view->SetScene(*sceneRes);
	});
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

	// 1. Поток рендера (отправка кадра N-1 на GPU для всех окон в одном потоке)
	m_ThreadsUpdate.Submit([this]()
		{
			m_PrimaryView->RenderFrame();
			for (const auto& view : m_ChildViews)
				view->RenderFrame();
			for (const auto& view : m_IndependentViews)
				view->RenderFrame();
		});

	// 2. Параллельная подготовка кадра N (для каждого окна в отдельном потоке)
	m_ThreadsUpdate.Submit([this, &time]()
		{
			m_PrimaryView->PreRender();
			m_PrimaryView->Update(time);
			m_PrimaryView->PrepareFrame();
		});

	for (const auto& view : m_ChildViews)
	{
		m_ThreadsUpdate.Submit([view, &time]()
			{
				view->PreRender();
				view->Update(time);
				view->PrepareFrame();
			});
	}

	for (const auto& view : m_IndependentViews)
	{
		m_ThreadsUpdate.Submit([view, &time]()
			{
				view->PreRender();
				view->Update(time);
				view->PrepareFrame();
			});
	}

	// 3. Ждём завершения рендера кадра N-1 и подготовки кадра N
	m_ThreadsUpdate.Join();

	// 5. Пост-рендер (переключение слотов и показ)
	m_PrimaryView->PostRender();
	for (const auto& view : m_ChildViews)
		view->PostRender();
	for (const auto& view : m_IndependentViews)
		view->PostRender();
}

