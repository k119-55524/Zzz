using System;
using System.Runtime.InteropServices;

namespace editor.Services
{
    public static class EngineRuntime
    {
        [DllImport("editorDLL.dll", EntryPoint = "Initialize", CallingConvention = CallingConvention.Cdecl)]
        private static extern bool NativeInitialize(IntPtr hwnd);

        [DllImport("editorDLL.dll", EntryPoint = "Deinitialize", CallingConvention = CallingConvention.Cdecl)]
        private static extern void NativeDeinitialize();

        [DllImport("editorDLL.dll", EntryPoint = "Tick", CallingConvention = CallingConvention.Cdecl)]
        private static extern void NativeTick();

        [DllImport("editorDLL.dll", EntryPoint = "ClearEngine", CallingConvention = CallingConvention.Cdecl)]
        private static extern void NativeClearEngine();

        [DllImport("editorDLL.dll", EntryPoint = "AddView", CallingConvention = CallingConvention.Cdecl)]
        private static extern void NativeAddView(IntPtr hwnd);

        [DllImport("editorDLL.dll", EntryPoint = "RemoveView", CallingConvention = CallingConvention.Cdecl)]
        private static extern void NativeRemoveView(IntPtr hwnd);

        public static bool TryInitialize(IntPtr renderHandle) => NativeInitialize(renderHandle);

        public static void Shutdown() => NativeDeinitialize();

        public static void Tick() => NativeTick();

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

        public static void AddView(IntPtr hwnd)
        {
            try
            {
                NativeAddView(hwnd);
            }
            catch (EntryPointNotFoundException)
            {
                // DLL пока не экспортирует этот метод
            }
            catch (DllNotFoundException)
            {
                // DLL не найдена
            }
        }

        public static void RemoveView(IntPtr hwnd)
        {
            try
            {
                NativeRemoveView(hwnd);
            }
            catch (EntryPointNotFoundException)
            {
                // DLL пока не экспортирует этот метод
            }
            catch (DllNotFoundException)
            {
                // DLL не найдена
            }
        }
    }
}
