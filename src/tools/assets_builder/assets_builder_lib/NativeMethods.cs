using System;
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
    public static extern IntPtr GetGamePackageMagicBytes();

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern uint GetGamePackageMajorVersion();

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern uint GetGamePackageMinorVersion();

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern uint GetGamePackagePatchVersion();

    // AssetType enum values P/Invoke
    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern uint GetAssetTypeProjectManifest();

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern uint GetAssetTypeScene();

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern uint GetAssetTypeView();

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern uint GetAssetTypePrefab();

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
    public static extern bool PackProjectNative(string sourceDir, string destinationDir, uint targetPlatform, string platformConfigFile);

    // Валидация имени каталога (company_name/app_name из project.json) той же логикой, что и движок
    // (Path::IsValidDirectoryName) - единственный источник истины на стороне C++. Имя передаётся в UTF-8,
    // так как каталоги пользовательских данных могут содержать не-ASCII символы (например, кириллицу).
    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern bool ValidateDirectoryNameNative([MarshalAs(UnmanagedType.LPUTF8Str)] string name);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
    [return: MarshalAs(UnmanagedType.I1)]
    public static extern bool IsSupportedAssetExtension(string ext);
}
