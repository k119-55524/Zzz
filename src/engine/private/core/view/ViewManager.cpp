
#include "View.h"
#include "ViewManager.h"
#include "../../platforms/Platform.h"

using namespace zzz::engine;

ViewManager::ViewManager(const Platform& platform, std::function<void()> onAllViewsClosed) :
	m_Platform{ platform },
	OnAllViewsClosed{ std::move(onAllViewsClosed) }
{
	ensure(OnAllViewsClosed != nullptr, "OnAllViewsClosed не должен быть null.");
}

ViewManager::~ViewManager()
{
	for (auto& view : m_Views)
		view = nullptr;

	m_Views.clear();
}

View* ViewManager::CreateView(const std::string_view viewName, const std::vector<std::shared_ptr<zzz::script::ViewScript>>& scripts)
{
#if Z_MOBILE
	if (m_Views.size() >= 1)
		THROW_RUNTIME("Мобильные платформы поддерживают только одно нативное окно на приложение.");
#endif

	auto view = safe_make_shared<View>(viewName, m_Platform, scripts, [this](View& v) { OnWindowClose(v); });
	View* viewPtr = view.get();
	m_Views.push_back(std::move(view));

	return viewPtr;
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
View* ViewManager::CreateView(void* data, const std::vector<std::shared_ptr<zzz::script::ViewScript>>& scripts)
{
	auto view = safe_make_shared<View>(m_Platform, data, scripts);
	View* viewPtr = view.get();
	m_Views.push_back(std::move(view));

	return viewPtr;
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

void ViewManager::Update(const zzz::engine::Time& time)
{
	for (const auto& view : m_Views)
	{
		view->Update(time);
	}
}