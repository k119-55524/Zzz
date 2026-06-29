using System;
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
        [DllImport("editorDLL.dll", EntryPoint = "Initialize", CallingConvention = CallingConvention.Cdecl)]
        private static extern bool NativeInitialize(LogCallback callback);

        [DllImport("editorDLL.dll", EntryPoint = "Deinitialize", CallingConvention = CallingConvention.Cdecl)]
        private static extern void NativeDeinitialize();

        [DllImport("editorDLL.dll", EntryPoint = "Tick", CallingConvention = CallingConvention.Cdecl)]
        private static extern void NativeTick();

        [DllImport("editorDLL.dll", EntryPoint = "ClearEngine", CallingConvention = CallingConvention.Cdecl)]
        private static extern void NativeClearEngine();

        [DllImport("editorDLL.dll", EntryPoint = "AddView", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr NativeAddView(IntPtr hwnd);

        [DllImport("editorDLL.dll", EntryPoint = "RemoveView", CallingConvention = CallingConvention.Cdecl)]
        private static extern void NativeRemoveView(IntPtr view);

        [DllImport("editorDLL.dll", EntryPoint = "SetProjectPath", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern void NativeSetProjectPath(string projectPath);

        [DllImport("editorDLL.dll", EntryPoint = "ReloadScripts", CallingConvention = CallingConvention.Cdecl)]
        private static extern void NativeReloadScripts();

        public static bool TryInitialize(LogCallback callback) => NativeInitialize(callback);

        public static void SetProjectPath(string projectPath)
        {
            try
            {
                NativeSetProjectPath(projectPath);
            }
            catch (EntryPointNotFoundException) {}
            catch (DllNotFoundException) {}
        }

        public static void ReloadScripts()
        {
            try
            {
                NativeReloadScripts();
            }
            catch (EntryPointNotFoundException) {}
            catch (DllNotFoundException) {}
        }

        public static void Shutdown()
        {
            try
            {
                NativeDeinitialize();
            }
            catch (EntryPointNotFoundException)
            {
            }
            catch (DllNotFoundException)
            {
            }
        }

        public static void Tick()
        {
            try
            {
                NativeTick();
            }
            catch (EntryPointNotFoundException)
            {
            }
            catch (DllNotFoundException)
            {
            }
        }

        public static void ClearEngine()
        {
            try
            {
                NativeClearEngine();
            }
            catch (EntryPointNotFoundException)
            {
            }
            catch (DllNotFoundException)
            {
            }
        }

        public static IntPtr AddView(IntPtr hwnd)
        {
            try
            {
                return NativeAddView(hwnd);
            }
            catch (EntryPointNotFoundException)
            {
            }
            catch (DllNotFoundException)
            {
            }
            return IntPtr.Zero;
        }

        public static void RemoveView(IntPtr view)
        {
            try
            {
                NativeRemoveView(view);
            }
            catch (EntryPointNotFoundException)
            {
            }
            catch (DllNotFoundException)
            {
            }
        }
    }
}
