
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

	std::vector<std::shared_ptr<ViewScript>> scripts;
	for (const auto& scriptGuid : startViewData->GetUiScriptGuids())
	{
		auto script = ScriptRegistry::CreateViewScript(scriptGuid);
		if (!script)
		{
			std::string err = std::format("Не удалось создать ViewScript по GUID {} для стартового вида.", scriptGuid.ToString());
			DOutError("{}", err);
			return std::unexpected(err);
		}

		scripts.push_back(script);
	}

	const auto& userPlatformData = userSettingsManager.GetStartViewUserData().GetPlatformData();
#if Z_DESKTOP
	ViewData viewData(userPlatformData.GetSize(), startViewData->GetSceneGuid(), startViewData->GetUiScriptGuids());
#else
	ViewData viewData(Size2D<zU32>{ 0, 0 }, startViewData->GetSceneGuid(), startViewData->GetUiScriptGuids());
#endif

	return CreateView(viewData, scripts);
}

std::expected <std::shared_ptr<View>, std::string> ViewManager::CreateView(const ViewData& viewData, const std::vector<std::shared_ptr<ViewScript>>& scripts)
{
#if Z_MOBILE
	if (m_Views.size() >= 1)
		THROW_RUNTIME("Мобильные платформы поддерживают только одно нативное окно на приложение.");
#endif

	auto view = safe_make_shared<View>(viewData, m_Platform, scripts, [this](View& v) { OnWindowClose(v); });
	m_Views.push_back(view);
	view->InvokeStart();

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
