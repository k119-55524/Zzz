#pragma once

#include "../Platform.h"
#include "../input/Input.h"
#include "NativeWindowState.h"

namespace zzz::engine
{
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
		 * @brief Вызывается, когда пользователь начинает изменять размер окна (захватил рамку мышью).
		 */
		std::function<void()> OnResizeStart;

		/**
		 * @brief Вызывается непрерывно в процессе изменения размера окна пользователем.
		 */
		std::function<void()> OnSizing;

		/**
		 * @brief Вызывается, когда пользователь отпускает рамку окна после изменения размера.
		 */
		std::function<void()> OnResizeEnd;

		/**
		 * @brief Вызывается, когда пользователь начинает перемещать окно (захватил заголовок мышью).
		 */
		std::function<void()> OnMoveStart;

		/**
		 * @brief Вызывается непрерывно в процессе перемещения окна пользователем.
		 */
		std::function<void()> OnMoving;

		/**
		 * @brief Вызывается, когда пользователь отпускает окно после перемещения.
		 */
		std::function<void()> OnMoveEnd;

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

		/**
		 * @brief Вызывается при изменении разрешения или конфигурации мониторов в системе.
		 */
		std::function<void()> OnDisplayChanged;
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

	class View;

	class WindowBase
	{
	public:
		WindowBase() = delete;
		WindowBase(
			const Platform& platform,
			const std::shared_ptr<Input> input,
			WindowCallbacks callbacks);
		virtual ~WindowBase() = default;

		[[nodiscard]] virtual std::expected<void, std::string> Initialize(const zzz::core::ViewPlatformData& platformData, const View* parentView = nullptr) = 0;
		[[nodiscard]] virtual bool IsMinimized() const noexcept = 0;
		[[nodiscard]] virtual bool IsMaximized() const = 0;


		/**
		 * @brief Возвращает прямоугольник клиентской области окна без учета рамок и заголовка (Client Rect / Surface Rect).
		 */
		[[nodiscard]] virtual zzz::math::Rect2D<zzz::core::zI32> GetClientRect() const = 0;

		/**
		 * @brief Возвращает точный физический размер клиентской области в пикселях для GAPI (Vulkan / DirectX 12 / Metal).
		 * Вычисляется как GetClientRect().GetSize() * Monitor.ScaleFactor.
		 */
		[[nodiscard]] zzz::math::Size2D<zzz::core::zU32> GetPhysicalClientSize() const noexcept;

		/**
		 * @brief Обрабатывает событие смены разрешения или конфигурации монитора.
		 */
		virtual void OnMonitorResolutionChanged() = 0;

		[[nodiscard]] const NativeWindowState& GetState() const noexcept { return m_NativeState; }
		[[nodiscard]] NativeWindowState& GetState() noexcept { return m_NativeState; }

		// Public для того, чтобы глобальные Си-функции (Wayland/Android) 
		// и Objective-C делегаты (macOS/iOS) могли вызывать события окна 
		// напрямую без написания boilerplate геттеров.
		WindowCallbacks m_Callbacks;

	protected:
		const Platform& m_Platform;
		const std::shared_ptr<Input> m_Input;
		NativeWindowState m_NativeState;
		zzz::math::Size2D<> m_WinSize;
		bool m_IsActivate;
	};
}
