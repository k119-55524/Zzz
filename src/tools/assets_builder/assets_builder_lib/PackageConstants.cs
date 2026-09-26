using System;
using System.Runtime.InteropServices;

namespace assets_builder_lib;

/// <summary>
/// Прослойка над нативной библиотекой assets_builder_dll (Single Source of Truth)
/// для получения путей, имен пакетов, версий и сигнатур архивов движка.
/// </summary>
public static class PackageConstants
{
	/// <summary>
	/// Имя главного бинарного пакета игры (по умолчанию "package.dat").
	/// </summary>
	public static string GamePackageFileName
	{
		get
		{
			try
			{
				IntPtr ptr = NativeMethods.GetGamePackageFileName();
				return ptr != IntPtr.Zero ? Marshal.PtrToStringAnsi(ptr) ?? "package.dat" : "package.dat";
			}
			catch
			{
				return "package.dat";
			}
		}
	}

	/// <summary>
	/// Имя архива ресурсов данных (по умолчанию "data.dat").
	/// </summary>
	public static string DataPackageFileName
	{
		get
		{
			try
			{
				IntPtr ptr = NativeMethods.GetDataPackageFileName();
				return ptr != IntPtr.Zero ? Marshal.PtrToStringAnsi(ptr) ?? "data.dat" : "data.dat";
			}
			catch
			{
				return "data.dat";
			}
		}
	}
}
