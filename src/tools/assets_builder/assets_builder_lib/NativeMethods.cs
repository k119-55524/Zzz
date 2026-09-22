using System;
using System.Runtime.InteropServices;

namespace assets_builder_lib;

internal static class NativeMethods
{
    private const string DllName = "assets_builder_dll";

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr GetAssetsDirectoryName();

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr GetGamePackageFileName();

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr GetGamePackageRelativePath();

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr GetGamePackageMagicBytes();

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern uint GetGamePackageMajorVersion();

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern uint GetGamePackageMinorVersion();

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern uint GetGamePackagePatchVersion();

    // Data package constants
    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr GetDataPackageFileName();

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr GetDataPackageRelativePath();

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr GetDataPackageMagicBytes();

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern uint GetDataPackageMajorVersion();

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern uint GetDataPackageMinorVersion();

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern uint GetDataPackagePatchVersion();

    // AssetType enum values P/Invoke
    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern uint GetAssetTypeProjectManifest();

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern uint GetAssetTypePrimaryView();

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern uint GetAssetTypeScene();

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern uint GetAssetTypeChildView();

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern uint GetAssetTypeIndependentView();

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern uint GetAssetTypePrefab();

    // inBuildTimestamp - unix-время (UTC, миллисекунды), записанное в заголовки package.dat/data.dat при
    // упаковке (см. DatFileHeader::GetBuildTime() и PackagePacker::PackProject).
    // outBuildTimestamp - фактически записанное значение.
    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
    public static extern bool PackProjectNative(string sourceDir, string destinationDir, uint targetPlatform, string platformConfigFile, ulong inBuildTimestamp, out ulong outBuildTimestamp);

    // Валидация имени каталога (company_name/app_name из project.json) той же логикой, что и движок
    // (Path::IsValidDirectoryName) - единственный источник истины на стороне C++. Имя передаётся в UTF-8,
    // так как каталоги пользовательских данных могут содержать не-ASCII символы (например, кириллицу).
    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern bool ValidateDirectoryNameNative([MarshalAs(UnmanagedType.LPUTF8Str)] string name);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
    [return: MarshalAs(UnmanagedType.I1)]
    public static extern bool IsSupportedAssetExtension(string ext);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
    [return: MarshalAs(UnmanagedType.I1)]
    public static extern bool IsSupportedDataAssetExtension(string ext);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
    [return: MarshalAs(UnmanagedType.I1)]
    public static extern bool IsSupportedViewExtension(string ext);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    public static extern bool GenerateGuidNative([Out] byte[] outBuffer, uint bufferSize);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    public static extern bool ValidateProjectIdentityNative(
        [MarshalAs(UnmanagedType.LPUTF8Str)] string projectDir,
        [Out] byte[] errorBuffer,
        uint errorBufferSize,
        [MarshalAs(UnmanagedType.LPUTF8Str)] string? platformConfigFile = null);
}
