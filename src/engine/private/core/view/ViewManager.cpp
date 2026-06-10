#include "ViewManager.h"
#include "View.h"
#include "../../platforms/Platform.h"
#include "../../platforms/window/Window.h"
#include <foundation.h>

using namespace zzz::engine;

ViewManager::ViewManager(std::shared_ptr<Platform> platform) :
	m_Platform{ platform }
{
	ensure(m_Platform != nullptr, "Platform must not be null.");
}

ViewManager::~ViewManager()
{
	for (auto& view : m_Views)
		view = nullptr;

	m_Views.clear();
}

void ViewManager::CreateView()
{
#if defined(Z_MOBILE)
	if (m_Views.size() >= 1)
		THROW_RUNTIME("Mobile platforms support only one native window per application.");
#endif

	auto view = zzz::safe_make_shared<View>(m_Platform);

	// Добавляем слушателя на закрытие окна
	view->GetWindow()->onCloseRequested += [this, weakView = std::weak_ptr(view)]()
	{
		if (auto v = weakView.lock())
			m_Views.remove(v);

		// Если больше нет активных окон
		if (m_Views.empty() && onAllViewsClosed)
			onAllViewsClosed();
	};

	m_Views.push_back(std::move(view));
}
