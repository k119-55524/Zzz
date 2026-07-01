using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace editor.Services
{
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct NativeLogEntry
    {
        public ulong Timestamp;
        public int Type;
        [MarshalAs(UnmanagedType.LPStr)]
        public string Text;
        [MarshalAs(UnmanagedType.LPStr)]
        public string File;
        [MarshalAs(UnmanagedType.LPStr)]
        public string Function;
        public uint Line;
    }

    [UnmanagedFunctionPointer(CallingConvention.StdCall)]
    public delegate void LogCallback(in NativeLogEntry entry);

    public static class EngineRuntime
    {
        private static readonly HashSet<string> _reportedNativeFailures = new();
        private static readonly object _nativeFailureLock = new();

        [DllImport("editor_dll.dll", EntryPoint = "Initialize", CallingConvention = CallingConvention.Cdecl)]
        private static extern bool NativeInitialize(LogCallback callback);

        [DllImport("editor_dll.dll", EntryPoint = "Deinitialize", CallingConvention = CallingConvention.Cdecl)]
        private static extern void NativeDeinitialize();

        [DllImport("editor_dll.dll", EntryPoint = "Tick", CallingConvention = CallingConvention.Cdecl)]
        private static extern void NativeTick();

        [DllImport("editor_dll.dll", EntryPoint = "ClearEngine", CallingConvention = CallingConvention.Cdecl)]
        private static extern void NativeClearEngine();

        [DllImport("editor_dll.dll", EntryPoint = "AddView", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr NativeAddView(IntPtr hwnd);

        [DllImport("editor_dll.dll", EntryPoint = "RemoveView", CallingConvention = CallingConvention.Cdecl)]
        private static extern void NativeRemoveView(IntPtr view);

        [DllImport("editor_dll.dll", EntryPoint = "SetProjectPath", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern void NativeSetProjectPath(string projectPath);

        [DllImport("editor_dll.dll", EntryPoint = "ReloadScripts", CallingConvention = CallingConvention.Cdecl)]
        private static extern void NativeReloadScripts();

        public static bool TryInitialize(LogCallback callback)
        {
            try
            {
                return NativeInitialize(callback);
            }
            catch (Exception ex) when (IsNativeInteropException(ex))
            {
                ReportNativeInteropFailure(nameof(NativeInitialize), ex);
                return false;
            }
        }

        public static void SetProjectPath(string projectPath)
        {
            try
            {
                NativeSetProjectPath(projectPath);
            }
            catch (Exception ex) when (IsNativeInteropException(ex))
            {
                ReportNativeInteropFailure(nameof(NativeSetProjectPath), ex);
            }
        }

        public static void ReloadScripts()
        {
            try
            {
                NativeReloadScripts();
            }
            catch (Exception ex) when (IsNativeInteropException(ex))
            {
                ReportNativeInteropFailure(nameof(NativeReloadScripts), ex);
            }
        }

        public static void Shutdown()
        {
            try
            {
                NativeDeinitialize();
            }
            catch (Exception ex) when (IsNativeInteropException(ex))
            {
                ReportNativeInteropFailure(nameof(NativeDeinitialize), ex);
            }
        }

        public static void Tick()
        {
            try
            {
                NativeTick();
            }
            catch (Exception ex) when (IsNativeInteropException(ex))
            {
                ReportNativeInteropFailure(nameof(NativeTick), ex);
            }
        }

        public static void ClearEngine()
        {
            try
            {
                NativeClearEngine();
            }
            catch (Exception ex) when (IsNativeInteropException(ex))
            {
                ReportNativeInteropFailure(nameof(NativeClearEngine), ex);
            }
        }

        public static IntPtr AddView(IntPtr hwnd)
        {
            try
            {
                return NativeAddView(hwnd);
            }
            catch (Exception ex) when (IsNativeInteropException(ex))
            {
                ReportNativeInteropFailure(nameof(NativeAddView), ex);
            }
            return IntPtr.Zero;
        }

        public static void RemoveView(IntPtr view)
        {
            try
            {
                NativeRemoveView(view);
            }
            catch (Exception ex) when (IsNativeInteropException(ex))
            {
                ReportNativeInteropFailure(nameof(NativeRemoveView), ex);
            }
        }

        private static bool IsNativeInteropException(Exception ex)
        {
            return ex is DllNotFoundException
                or EntryPointNotFoundException
                or BadImageFormatException;
        }

        private static void ReportNativeInteropFailure(string operation, Exception ex)
        {
            string key = $"{operation}:{ex.GetType().FullName}";
            lock (_nativeFailureLock)
            {
                if (!_reportedNativeFailures.Add(key))
                {
                    return;
                }
            }

            EditorLogger.LogError($"[EngineRuntime] Native call '{operation}' failed: {ex.Message}", LogSource.Editor);
        }
    }
}
