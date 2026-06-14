
#include "View.h"
#include "ViewManager.h"
#include "../../platforms/Platform.h"

using namespace zzz::engine;

ViewManager::ViewManager(std::shared_ptr<Platform> platform, std::function<void()> onAllViewsClosed) :
	m_Platform{ platform },
	OnAllViewsClosed{ onAllViewsClosed }
{
	ensure(m_Platform != nullptr, "Platform must not be null.");
	ensure(OnAllViewsClosed != nullptr, "OnAllViewsClosed must not be null.");
}

ViewManager::~ViewManager()
{
	for (auto& view : m_Views)
		view = nullptr;

	m_Views.clear();
}

void ViewManager::CreateView()
{
#if Z_MOBILE
	if (m_Views.size() >= 1)
		THROW_RUNTIME("Mobile platforms support only one native window per application.");
#endif

	auto view = safe_make_shared<View>(m_Platform, std::bind(&ViewManager::HandleWindowClose, this, std::placeholders::_1));
	m_Views.push_back(std::move(view));
}

void ViewManager::HandleWindowClose(View& view)
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
		THROW_RUNTIME("View not found in m_Views.");

	if (m_Views.empty())
		OnAllViewsClosed();
}
