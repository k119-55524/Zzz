
#include "View.h"
#include "ViewManager.h"
#include "../platforms/Platform.h"
#include "../package/PackageManager.h"
#include "../package/UserSettingsManager.h"

#include "../gapi/IGAPI.h"

using namespace zzz::core;
using namespace zzz::engine;

ViewManager::ViewManager(const Platform& platform, std::shared_ptr<IGAPI> gapi, std::function<void()> onAllViewsClosed) :
	m_Platform{ platform },
	m_GAPI{ std::move(gapi) },
	OnAllViewsClosed{ std::move(onAllViewsClosed) }
{
	ensure(m_GAPI != nullptr, "IGAPI не должен быть null.");
	ensure(OnAllViewsClosed != nullptr, "OnAllViewsClosed не должен быть null.");
}

ViewManager::~ViewManager()
{
	m_Views.clear();
}

std::expected<std::shared_ptr<View>, std::string> ViewManager::CreateStartView(const PackageManager& packageManager, const UserSettingsManager& userSettingsManager)
{
	auto startViewData = packageManager.GetStartViewData();
	if (!startViewData)
	{
		std::string err = std::format("Обязательный ресурс StartViewData не найден в пакете: {}", startViewData.error());
		DOutError("{}", err);
		return std::unexpected(err);
	}

	return CreateView(userSettingsManager.GetStartViewUserData().GetPlatformData(), startViewData->GetUiScriptGuids());
}

std::expected<std::shared_ptr<View>, std::string> ViewManager::CreateView(const StartViewPlatformData& settings, const std::vector<Guid>& scripts)
{
#if Z_MOBILE
	if (m_Views.size() >= 1)
		THROW_RUNTIME("Мобильные платформы поддерживают только одно нативное окно на приложение.");
#endif

	std::shared_ptr<View> view;
	try
	{
		view = safe_make_shared<View>(settings, scripts, m_Platform, [this](View& v) { OnWindowClose(v); });
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
	auto it = std::ranges::find_if(
		m_Views,
		[&view](const auto& p)
		{
			return p.get() == &view;
		});

	if (it != m_Views.end())
		m_Views.erase(it);
	else
		THROW_RUNTIME("View не найден в m_Views.");

	if (m_Views.empty())
		OnAllViewsClosed();
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
