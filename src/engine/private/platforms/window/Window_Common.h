#pragma once

#include "../Platform.h"
#include "../input/Input.h"
#include "../../core/templates/Size2D.h"


namespace zzz::engine
{
	enum class eWinResize : zU32
	{
		Show,
		Hide,
		Resize
	};

	class WindowBase
	{
	public:
		WindowBase() = delete;
		WindowBase(
			const std::shared_ptr<Platform> platform,
			const std::shared_ptr<Input> input,
			std::function<void()> onWindowClose);
		virtual ~WindowBase() = default;

#pragma region Window Events
		// Вызывается при штатном запросе на закрытие окна (крестик, Alt+F4).
		std::function<void()> OnClose;

		// Вызывается при изменении размера окна (или при повороте экрана на мобильных устройствах).
		std::function<void(Size2D<>&, eWinResize)> OnResize;

		// Вызывается при перемещении окна на монитор с другим DPI.
		// TODO: В будущем стоит добавить передачу ScaleFactor (float) и нового размера, чтобы движок знал, как перестроить UI.
		std::function<void()> OnDpiChanged;

		// Изменение логического (клавиатурного) фокуса окна. Специфично для десктопа.
		std::function<void(bool)> OnFocus;

		// Окно выходит на передний план или уходит на задний (может быть частично видно, но не активно).
		std::function<void(bool)> OnActivate;
#pragma endregion

#pragma region App Lifecycle & GPU Surface (All Platforms)
		// Вызывается, когда ОС выделяет окну графическую поверхность.
		// КРИТИЧНО ДЛЯ ANDROID/iOS, но также необходимо на macOS, Linux и Win для передачи платформенного хэндла
		// и инициализации Vulkan/Metal Swapchain.
		std::function<void(void* nativeHandle)> OnSurfaceCreated;

		// КРИТИЧНО ДЛЯ ANDROID/iOS: Вызывается при сворачивании приложения.
		// Графическая поверхность уничтожается ОС. Здесь ОБЯЗАТЕЛЬНО нужно убить Swapchain, иначе будет краш.
		std::function<void()> OnSurfaceDestroyed;

		// Вызывается при уходе приложения в фон. Следует остановить симуляцию и рендер для экономии батареи.
		std::function<void()> OnSuspend;

		// Вызывается при возвращении приложения из фона на передний план.
		std::function<void()> OnResume;

		// Вызывается, когда системе (особенно на смартфонах) не хватает оперативной памяти.
		// Движок должен сбросить текстуры, кэши и прочие некритичные ресурсы, иначе ОС убьет процесс.
		std::function<void()> OnLowMemory;

		// Вызывается при изменении безопасной зоны экрана (вырез под камеру, челка, полоска "Домой").
		// Передает отступы: top, bottom, left, right (в пикселях), которые нельзя перекрывать UI.
		std::function<void(int top, int bottom, int left, int right)> OnSafeAreaChanged;
#pragma endregion

	protected:
		const std::shared_ptr<Platform> m_Platform;
		const std::shared_ptr<Input> m_Input;
		Size2D<> m_WinSize;
		bool IsActivate;
	};
}
