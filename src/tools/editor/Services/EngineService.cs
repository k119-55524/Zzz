using System;
using System.Collections.Generic;
using System.Windows.Media;

namespace editor.Services
{
    public class EngineService
    {
        private static readonly LogCallback _logCallback = OnNativeLog;

        private readonly List<IntPtr> _activeViewports = new();
        private bool _isEngineInitialized;
        private bool _isProjectOpen;

        public bool IsEngineInitialized => _isEngineInitialized;

        public EngineService()
        {
        }

        private static void OnNativeLog(in NativeLogEntry entry)
        {
            LogLevel level = (LogLevel)entry.Type;
            EditorLogger.Log(
                LogSource.Engine,
                level,
                entry.Text,
                entry.File,
                entry.Function,
                entry.Line
            );
        }

        // Вызывается вьюпортом при создании HWND
        public void AddViewport(IntPtr hwnd, string viewName)
        {
            if (!_activeViewports.Contains(hwnd))
            {
                _activeViewports.Add(hwnd);
                EditorLogger.LogInfo($"Инициализация вьюпорта {viewName} (HWND: {hwnd})", LogSource.Editor);
                
                // Прокидываем в DLL (безопасный вызов с try-catch внутри)
                EngineRuntime.AddView(hwnd);

                // Если проект открыт и движок еще не инициализирован основным HWND - инициализируем
                TryInitializeEngine();
            }
        }

        // Вызывается вьюпортом при уничтожении HWND
        public void RemoveViewport(IntPtr hwnd, string viewName)
        {
            if (_activeViewports.Contains(hwnd))
            {
                _activeViewports.Remove(hwnd);
                EditorLogger.LogInfo($"Прибитие вьюпорта {viewName} (HWND: {hwnd})", LogSource.Editor);
                
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
            if (_isEngineInitialized || _activeViewports.Count == 0 || !_isProjectOpen)
            {
                return;
            }

            // Инициализируем движок самым первым доступным HWND вьюпорта
            IntPtr primaryHwnd = _activeViewports[0];
            
            EditorLogger.LogInfo("Запуск инициализации движка...", LogSource.Editor);
            _isEngineInitialized = EngineRuntime.TryInitialize(_logCallback);

            if (_isEngineInitialized)
            {
                EditorLogger.LogInfo("Движок успешно инициализирован.", LogSource.Editor);
                StartRenderingLoop();
            }
            else
            {
                EditorLogger.LogError("Не удалось инициализировать движок.", LogSource.Editor);
            }
        }

        public void ShutdownEngine()
        {
            if (_isEngineInitialized)
            {
                StopRenderingLoop();
                EngineRuntime.Shutdown();
                _isEngineInitialized = false;
                EditorLogger.LogInfo("Движок остановлен.", LogSource.Editor);
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
