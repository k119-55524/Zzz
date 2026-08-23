#pragma once

#include "engine/EngineIncludes.h"

#include "../platforms/Platform.h"
#include "../platforms/input/Input.h"
#include "../platforms/window/NativeWindow.h"

#include "engine/gapi/SurfView.h"
#include "engine/view/ViewWindowState.h"

namespace zzz::engine
{
	class UserSettingsManager;

	class View final
	{
		Z_NO_MOVE(View);

	public:
		View() = delete;
		View(zzz::core::Guid guid, const zzz::core::ViewPlatformData& settings, std::vector<std::shared_ptr<zzz::core::ViewScript>> scripts, const Platform& platform, std::shared_ptr<IGAPI> gapi, std::shared_ptr<UserSettingsManager> userSettingsManager, std::function<void(View&)> onWindowClose, const View* parentView = nullptr);
#if Z_EDITOR
		View(const Platform& platform, std::shared_ptr<IGAPI> gapi, void* data);
#endif // Z_EDITOR
		~View();

		inline void InvokeStart() { m_EventBus.InvokeStart(); }
		void Update(const zzz::core::Time& time);

		void PrepareFrame();
		void RenderFrame();

		[[nodiscard]] const zzz::core::Guid& GetGuid() const noexcept { return m_Guid; }
		[[nodiscard]] ViewWindowState GetState() const;

		[[nodiscard]] const NativeWindow& GetNativeWindow() const noexcept { return *m_NativeWindow; }
		[[nodiscard]] NativeWindow& GetNativeWindow() noexcept { return *m_NativeWindow; }

		[[nodiscard]] std::shared_ptr<ISurfView> GetSurfView() const noexcept { return m_SurfView; }

		inline void SetActive(bool active)
		{
			if (m_IsActive == active)
				return;

			m_IsActive = active;

			if (active)
				m_EventBus.InvokeEnable();
			else
				m_EventBus.InvokeDisable();
		}
		inline bool IsActive() const noexcept { return m_IsActive; }

	private:
		void Initialize(const ViewPlatformData& settings, const std::vector<std::shared_ptr<ViewScript>>& scripts, const View* parentView = nullptr);
#if Z_EDITOR
		void Initialize(void* data);
#endif

#pragma region Window Events
		/**
		 * @brief Обрабатывает изменение размера окна (поворот экрана на мобильных).
		 * @param size Новые размеры клиентской области.
		 * @param type Тип ресайза (Resize, Show, Hide).
		 * 
		 * @platforms Поддерживается везде (Windows, macOS, Linux, Android, iOS).
		 * @usage Используется для перерасчета матриц проекции камеры (Aspect Ratio) и перекомпоновки UI-элементов.
		 * При eWinResize::Hide рендер должен ставиться на паузу, при Show - возобновляться.
		 */
		void OnWindowResize(Size2D<>& size, eWinResize type);

		/**
		 * @brief Обрабатывает момент начала перетаскивания рамки окна пользователем.
		 * 
		 * @platforms Гарантировано только на Windows. На мобильных (Android/iOS) и Wayland не вызывается.
		 * @usage Используется для паузы тяжелых вычислений (чтобы UI операционной системы не тормозил при ресайзе).
		 */
		void OnWindowResizeStart();

		/**
		 * @brief Обрабатывает промежуточные изменения при перетаскивании рамки.
		 * 
		 * @platforms Только десктопы (Windows, MacOS).
		 * @usage Можно использовать для "живого" изменения UI, но нужно быть осторожным, так как на Windows 
		 * это блокирует главный поток сообщений.
		 */
		void OnWindowSizing();

		/**
		 * @brief Обрабатывает момент завершения изменения размера окна.
		 * 
		 * @platforms Гарантировано только на Windows.
		 * @usage Сигнал к тому, что можно снимать симуляцию с паузы, вызванной OnWindowResizeStart.
		 */
		void OnWindowResizeEnd();

		/**
		 * @brief Обрабатывает перемещение окна на монитор с другим DPI.
		 * 
		 * @platforms Windows, macOS, Linux (если настроен дробный скейлинг).
		 * @usage Пересчет масштабирования шрифтов (DPI scale) и адаптация UI под новый монитор, чтобы интерфейс не был "мыльным".
		 */
		void OnWindowDpiChanged();

		/**
		 * @brief Обрабатывает изменение фокуса окна (настольные ОС).
		 * @param focus Получен (true) или потерян (false).
		 * 
		 * @platforms Windows, macOS, Linux (Wayland/X11). Не актуально для мобилок.
		 * @usage Захват/отпускание курсора мыши (режим шутера). Если окно теряет фокус, курсор нужно освободить.
		 */
		void OnWindowFocus(bool focus);

		/**
		 * @brief Обрабатывает активацию/деактивацию окна.
		 * @param active Активно (true) или ушло в фон (false).
		 * 
		 * @platforms Windows, macOS, Linux.
		 * @usage Приглушение звука (Mute) или снижение FPS (кадров в секунду), если игра работает в фоне, но не свернута.
		 */
		void OnWindowActivate(bool active);

		/**
		 * @brief Обрабатывает изменение разрешения или конфигурации монитора.
		 */
		void OnWindowDisplayChanged();

#pragma endregion

#pragma region App Lifecycle & GPU Surface Events
		/**
		 * @brief Вызывается при выделении графической поверхности ОС.
		 * @param handle Нативный хэндл (HWND, wl_surface*, CAMetalLayer*).
		 * 
		 * @platforms Гарантировано везде (Windows, macOS, Linux, Android, iOS).
		 * @usage Самый важный колбэк для старта графики. Здесь инициализируется Vulkan Surface / Metal Swapchain 
		 * и начинается отрисовка.
		 */
		void OnWindowSurfaceCreated(void* handle);

		/**
		 * @brief Вызывается при уничтожении графической поверхности.
		 * 
		 * @platforms Гарантировано везде. Особенно критично на Android (при скрытии/повороте экрана ОС физически удаляет surface).
		 * @usage Обязательное уничтожение Swapchain и приостановка рендер-лупа. Иначе будет краш из-за попытки рендера в "мертвую" поверхность.
		 */
		void OnWindowSurfaceDestroyed();

		/**
		 * @brief Приложение ушло в фон (свернуто или перекрыто).
		 * 
		 * @platforms Гарантировано везде. На мобильных (Android/iOS) вызывается при нажатии кнопки "Home".
		 * @usage Постановка геймплея на паузу, остановка всех тяжелых потоков, сохранение состояния игры на диск.
		 */
		void OnWindowSuspend();

		/**
		 * @brief Приложение возвращено на передний план.
		 * 
		 * @platforms Гарантировано везде.
		 * @usage Снятие игры с паузы, возобновление аудио, перезапуск остановленных потоков симуляции.
		 */
		void OnWindowResume();

		/**
		 * @brief Системе не хватает памяти, необходимо освободить ресурсы.
		 * 
		 * @platforms Гарантировано на Android, iOS, macOS, Windows (через WM_COMPACTING).
		 * @usage Очистка кэша текстур, пулов памяти, удаление невидимых объектов. Если проигнорировать — ОС убьет процесс (OOM Killer).
		 */
		void OnWindowLowMemory();

		/**
		 * @brief Изменилась безопасная зона экрана (вырезы/челки).
		 * @param top Отступ сверху.
		 * @param bottom Отступ снизу.
		 * @param left Отступ слева.
		 * @param right Отступ справа.
		 * 
		 * @platforms iOS (челки, Dynamic Island), Android (вырезы).
		 * @usage Ограничение кликабельного UI, чтобы кнопки меню не оказались под вырезом камеры или системными свайп-жестами.
		 */
		void OnWindowSafeAreaChanged(int top, int bottom, int left, int right);
#pragma endregion

		const Platform& m_Platform;
		Guid m_Guid;
		std::shared_ptr<IGAPI> m_GAPI;
		std::shared_ptr<ISurfView> m_SurfView;
		std::shared_ptr<Input>  m_Input;
		std::shared_ptr<NativeWindow> m_NativeWindow;
		std::shared_ptr<UserSettingsManager> m_UserSettingsManager;
		zzz::templates::ThreadPool m_ThreadsUpdate;

		bool m_IsActive;
		ViewEventBus m_EventBus;
		std::shared_ptr<Scene> m_ActiveScene;
		std::vector<std::shared_ptr<ViewScript>> m_Scripts;

		std::function<void(View&)> OnWindowClose;
		void HandleWindowClose();
	};
}
