using System.Runtime.InteropServices;

namespace assets_builder_lib;

internal static class NativeMethods
{
    private const string DllName = "assets_builder_dll";

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr GetBuilderEngineVersion();

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr GetGamePackageFileName();

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern bool SerializeProjectManifest(string projectJsonPath, string outputBinaryPath);
}
