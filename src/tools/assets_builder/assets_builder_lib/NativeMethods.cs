using System;
using System.Runtime.InteropServices;

namespace assets_builder_lib;

internal static class NativeMethods
{
    private const string DllName = "assets_builder_dll";

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr GetGamePackageFileName();

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr GetDataPackageFileName();

    // inBuildTimestamp - unix-время (UTC, миллисекунды), записанное в заголовки package.dat/data.dat при
    // упаковке (см. DatFileHeader::GetBuildTime() и PackagePacker::PackProject).
    // outBuildTimestamp - фактически записанное значение.
    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    public static extern bool PackProjectNative(
        [MarshalAs(UnmanagedType.LPUTF8Str)] string sourceDir,
        [MarshalAs(UnmanagedType.LPUTF8Str)] string destinationDir,
        uint targetPlatform,
        [MarshalAs(UnmanagedType.LPUTF8Str)] string? platformConfigFile,
        ulong inBuildTimestamp,
        out ulong outBuildTimestamp,
        [Out] byte[]? errorBuffer = null,
        uint errorBufferSize = 0);

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

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    public static extern bool BeginBuildSessionNative(
        [MarshalAs(UnmanagedType.LPUTF8Str)] string projectDir,
        [Out] byte[] errorBuffer,
        uint errorBufferSize);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void EndBuildSessionNative();
}

