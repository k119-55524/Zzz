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
        private readonly Dictionary<IntPtr, IntPtr> _hwndToViewMap = new();

        public bool IsEngineInitialized => _isEngineInitialized;

        public EngineService()
        {
            TryInitializeEngine();
        }

        private static void OnNativeLog(in NativeLogEntry entry)
        {
            LogLevel level = (LogLevel)entry.Type;
            LogSource source = LogSource.Engine;

            if (!string.IsNullOrEmpty(entry.File) && entry.File.Replace('\\', '/').Contains("/" + Project.ProjectStructure.AssetsRoot + "/", StringComparison.OrdinalIgnoreCase))
            {
                source = LogSource.Scripts;
            }

            EditorLogger.Log(
                source,
                level,
                entry.Text,
                entry.File,
                entry.Function,
                entry.Line
            );
        }


        private static string GetLocString(string key)
        {
            if (System.Windows.Application.Current == null) return string.Empty;
            if (System.Windows.Application.Current.Dispatcher.CheckAccess())
            {
                return System.Windows.Application.Current.TryFindResource(key) as string ?? string.Empty;
            }
            return System.Windows.Application.Current.Dispatcher.Invoke(() => System.Windows.Application.Current.TryFindResource(key) as string) ?? string.Empty;
        }

        public void AddViewport(IntPtr hwnd, string viewName)
        {
            if (!_activeViewports.Contains(hwnd))
            {
                _activeViewports.Add(hwnd);
                EditorLogger.LogInfo(string.Format(GetLocString("Log_Viewport_Init"), viewName, hwnd), LogSource.Editor);
                
                // Прокидываем в DLL
                IntPtr viewPtr = EngineRuntime.AddView(hwnd);
                if (viewPtr == IntPtr.Zero)
                {
                    EditorLogger.Log(LogSource.Editor, LogLevel.Critical, string.Format(GetLocString("Log_Viewport_Init_Failed"), viewName, hwnd));
                }
                else
                {
                    _hwndToViewMap[hwnd] = viewPtr;
                }

                if (_isEngineInitialized)
                {
                    StartRenderingLoop();
                }
            }
        }

        // Вызывается вьюпортом при уничтожении HWND
        public void RemoveViewport(IntPtr hwnd, string viewName)
        {
            if (_activeViewports.Contains(hwnd))
            {
                _activeViewports.Remove(hwnd);
                EditorLogger.LogInfo(string.Format(GetLocString("Log_Viewport_Destroy"), viewName, hwnd), LogSource.Editor);
                
                // Прокидываем в DLL
                if (_hwndToViewMap.TryGetValue(hwnd, out IntPtr viewPtr))
                {
                    EngineRuntime.RemoveView(viewPtr);
                    _hwndToViewMap.Remove(hwnd);
                }
                else
                {
                    EngineRuntime.RemoveView(IntPtr.Zero);
                }

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
            EngineRuntime.ClearEngine();
        }

        // Вызывается при закрытии/выгрузке проекта
        public void OnProjectClosed()
        {
            EngineRuntime.ClearEngine();
        }

        private void TryInitializeEngine()
        {
            if (_isEngineInitialized)
            {
                return;
            }
            
            EditorLogger.LogInfo(GetLocString("Log_Engine_Init_Start"), LogSource.Editor);
            _isEngineInitialized = EngineRuntime.TryInitialize(_logCallback);

            if (_isEngineInitialized)
            {
                EditorLogger.LogInfo(GetLocString("Log_Engine_Init_Success"), LogSource.Editor);
                if (_activeViewports.Count > 0)
                {
                    StartRenderingLoop();
                }
            }
            else
            {
                EditorLogger.LogError(GetLocString("Log_Engine_Init_Fail"), LogSource.Editor);
            }
        }

        public void ShutdownEngine()
        {
            if (_isEngineInitialized)
            {
                StopRenderingLoop();
                EngineRuntime.Shutdown();
                _isEngineInitialized = false;
                EditorLogger.LogInfo(GetLocString("Log_Engine_Stop"), LogSource.Editor);
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
