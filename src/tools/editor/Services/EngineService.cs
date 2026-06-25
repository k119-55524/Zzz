using System;
using System.Collections.Generic;
using System.Windows.Media;

namespace editor.Services
{
    public class EngineService
    {
        private readonly List<IntPtr> _activeViewports = new();
        private bool _isEngineInitialized;
        private bool _isProjectOpen;

        public bool IsEngineInitialized => _isEngineInitialized;

        public EngineService()
        {
        }

        // Вызывается вьюпортом при создании HWND
        public void AddViewport(IntPtr hwnd)
        {
            if (!_activeViewports.Contains(hwnd))
            {
                _activeViewports.Add(hwnd);
                
                // Прокидываем в DLL (безопасный вызов с try-catch внутри)
                EngineRuntime.AddView(hwnd);

                // Если проект открыт и движок еще не инициализирован основным HWND - инициализируем
                TryInitializeEngine();
            }
        }

        // Вызывается вьюпортом при уничтожении HWND
        public void RemoveViewport(IntPtr hwnd)
        {
            if (_activeViewports.Contains(hwnd))
            {
                _activeViewports.Remove(hwnd);
                
                // Прокидываем в DLL
                EngineRuntime.RemoveView(hwnd);

                // Если активных окон не осталось, а движок был запущен - останавливаем рендеринг
                if (_activeViewports.Count == 0 && _isEngineInitialized)
                {
                    StopRenderingLoop();
                }
            }
        }

        // Вызывается при успешном открытии/создании проекта
        public void OnProjectOpened(string projectPath)
        {
            _isProjectOpen = true;
            EngineRuntime.ClearEngine();
        }

        // Вызывается при закрытии/выгрузке проекта
        public void OnProjectClosed()
        {
            _isProjectOpen = false;
            EngineRuntime.ClearEngine();
        }

        private void TryInitializeEngine()
        {
            if (_isEngineInitialized || _activeViewports.Count == 0)
            {
                return;
            }

            // Инициализируем движок самым первым доступным HWND вьюпорта
            IntPtr primaryHwnd = _activeViewports[0];
            _isEngineInitialized = EngineRuntime.TryInitialize(primaryHwnd);

            if (_isEngineInitialized)
            {
                StartRenderingLoop();
            }
        }

        public void ShutdownEngine()
        {
            if (_isEngineInitialized)
            {
                StopRenderingLoop();
                EngineRuntime.Shutdown();
                _isEngineInitialized = false;
            }
        }

        private void StartRenderingLoop()
        {
            CompositionTarget.Rendering -= CompositionTarget_Rendering;
            CompositionTarget.Rendering += CompositionTarget_Rendering;
        }

        private void StopRenderingLoop()
        {
            CompositionTarget.Rendering -= CompositionTarget_Rendering;
        }

        private void CompositionTarget_Rendering(object? sender, EventArgs e)
        {
            if (_isEngineInitialized)
            {
                EngineRuntime.Tick();
            }
        }
    }
}
