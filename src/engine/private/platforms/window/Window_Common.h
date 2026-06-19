#pragma once

#include "../Platform.h"
#include "../input/Input.h"
#include "../../core/templates/Size2D.h"

#include <common/enums/eWinResize.h>

namespace zzz::engine
{
	using namespace zzz::common;
	/**
	 * @brief Набор всех возможных колбэков жизненного цикла окна.
	 * Передается из View в платформенную реализацию окна при создании.
	 */
	struct WindowCallbacks
	{
#pragma region Window Events
		/**
		 * @brief Вызывается при штатном запросе на закрытие окна.
		 * Например, при нажатии на крестик окна или шорткат (Alt+F4).
		 * ОС ожидает, что после этого приложение корректно завершится.
		 */
		std::function<void()> OnClose;

		/**
		 * @brief Вызывается при изменении размера окна (или при повороте экрана).
		 * @param size Новый размер клиентской области окна в пикселях.
		 * @param type Тип изменения (Resize, Show, Hide).
		 */
		std::function<void(Size2D<>& size, eWinResize type)> OnResize;

		/**
		 * @brief Вызывается, когда пользователь начинает перетаскивать рамку окна (захватил мышью).
		 * Может использоваться для приостановки симуляции или рендера, чтобы избежать фризов (особенно на Windows).
		 */
		std::function<void()> OnResizeStart;

		/**
		 * @brief Вызывается непрерывно в процессе перетаскивания рамки окна пользователем.
		 */
		std::function<void()> OnSizing;

		/**
		 * @brief Вызывается, когда пользователь отпускает рамку окна после изменения размера.
		 * Симуляция и полноценный рендер могут быть возобновлены.
		 */
		std::function<void()> OnResizeEnd;

		/**
		 * @brief Вызывается при перемещении окна на монитор с другим DPI.
		 * TODO: В будущем стоит добавить передачу ScaleFactor (float) и нового размера, чтобы движок знал, как перестроить UI.
		 */
		std::function<void()> OnDpiChanged;

		/**
		 * @brief Изменение логического (клавиатурного) фокуса окна. Специфично для десктопа.
		 * @param focus true, если окно получило фокус; false, если потеряло.
		 */
		std::function<void(bool focus)> OnFocus;

		/**
		 * @brief Окно выходит на передний план или уходит на задний (может быть частично видно, но не активно).
		 * @param active true, если окно стало активным; false, если ушло в фон.
		 */
		std::function<void(bool active)> OnActivate;
#pragma endregion

#pragma region App Lifecycle & GPU Surface (All Platforms)
		/**
		 * @brief Вызывается, когда ОС выделяет окну графическую поверхность.
		 * КРИТИЧНО ДЛЯ ANDROID/iOS, но также необходимо на macOS, Linux и Win для передачи платформенного хэндла
		 * и инициализации Vulkan/Metal Swapchain.
		 * @param nativeHandle Платформозависимый указатель (HWND, wl_surface*, CAMetalLayer* и т.д.).
		 */
		std::function<void(void* nativeHandle)> OnSurfaceCreated;

		/**
		 * @brief КРИТИЧНО ДЛЯ ANDROID/iOS: Вызывается при сворачивании приложения.
		 * Графическая поверхность уничтожается ОС. Здесь ОБЯЗАТЕЛЬНО нужно убить Swapchain, иначе будет краш.
		 */
		std::function<void()> OnSurfaceDestroyed;

		/**
		 * @brief Вызывается при уходе приложения в фон. 
		 * Следует остановить симуляцию и рендер для экономии батареи мобильного устройства или ресурсов ПК.
		 */
		std::function<void()> OnSuspend;

		/**
		 * @brief Вызывается при возвращении приложения из фона на передний план.
		 * Симуляция и рендер должны быть восстановлены.
		 */
		std::function<void()> OnResume;

		/**
		 * @brief Вызывается, когда системе (особенно на смартфонах) не хватает оперативной памяти.
		 * Движок должен сбросить текстуры, кэши и прочие некритичные ресурсы, иначе ОС убьет процесс.
		 */
		std::function<void()> OnLowMemory;

		/**
		 * @brief Вызывается при изменении безопасной зоны экрана (вырез под камеру, челка, полоска "Домой").
		 * @param top Отступ сверху в пикселях.
		 * @param bottom Отступ снизу в пикселях.
		 * @param left Отступ слева в пикселях.
		 * @param right Отступ справа в пикселях.
		 */
		std::function<void(int top, int bottom, int left, int right)> OnSafeAreaChanged;
#pragma endregion
	};

	class WindowBase
	{
	public:
		WindowBase() = delete;
		WindowBase(
			const Platform& platform,
			const std::shared_ptr<Input> input,
			WindowCallbacks callbacks);
		virtual ~WindowBase() = default;

		// Public для того, чтобы глобальные Си-функции (Wayland/Android) 
		// и Objective-C делегаты (macOS/iOS) могли вызывать события окна 
		// напрямую без написания boilerplate геттеров.
		WindowCallbacks m_Callbacks;

	protected:
		const Platform& m_Platform;
		const std::shared_ptr<Input> m_Input;
		Size2D<> m_WinSize;
		bool IsActivate;
	};
}