using System;
using System.Runtime.InteropServices;

namespace assets_builder_lib;

/// <summary>
/// Прослойка над нативной библиотекой assets_builder_dll (Single Source of Truth)
/// для получения путей, имен пакетов, версий и сигнатур архивов движка.
/// </summary>
public static class PackageConstants
{
	#region Path and File Names

	/// <summary>
	/// Имя каталога игровых ассетов (по умолчанию "assets").
	/// </summary>
	public static string AssetsDirectoryName
	{
		get
		{
			try
			{
				IntPtr ptr = NativeMethods.GetAssetsDirectoryName();
				return ptr != IntPtr.Zero ? Marshal.PtrToStringAnsi(ptr) ?? "assets" : "assets";
			}
			catch
			{
				return "assets";
			}
		}
	}

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
	/// Относительный путь к главному пакету игры (по умолчанию "assets/package.dat").
	/// </summary>
	public static string GamePackageRelativePath
	{
		get
		{
			try
			{
				IntPtr ptr = NativeMethods.GetGamePackageRelativePath();
				return ptr != IntPtr.Zero ? Marshal.PtrToStringAnsi(ptr) ?? "assets/package.dat" : "assets/package.dat";
			}
			catch
			{
				return "assets/package.dat";
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

	/// <summary>
	/// Относительный путь к архиву ресурсов данных (по умолчанию "assets/data.dat").
	/// </summary>
	public static string DataPackageRelativePath
	{
		get
		{
			try
			{
				IntPtr ptr = NativeMethods.GetDataPackageRelativePath();
				return ptr != IntPtr.Zero ? Marshal.PtrToStringAnsi(ptr) ?? "assets/data.dat" : "assets/data.dat";
			}
			catch
			{
				return "assets/data.dat";
			}
		}
	}

	#endregion // Path and File Names

	#region Versions and Signatures

	public static Version GamePackageVersion => new(
		(int)NativeMethods.GetGamePackageMajorVersion(),
		(int)NativeMethods.GetGamePackageMinorVersion(),
		(int)NativeMethods.GetGamePackagePatchVersion());

	public static Version DataPackageVersion => new(
		(int)NativeMethods.GetDataPackageMajorVersion(),
		(int)NativeMethods.GetDataPackageMinorVersion(),
		(int)NativeMethods.GetDataPackagePatchVersion());

	public static byte[] GetGamePackageMagicBytes()
	{
		IntPtr ptr = NativeMethods.GetGamePackageMagicBytes();
		if (ptr == IntPtr.Zero) return new byte[] { (byte)'Z', (byte)'P', (byte)'D' };
		byte[] magic = new byte[3];
		Marshal.Copy(ptr, magic, 0, 3);
		return magic;
	}

	public static byte[] GetDataPackageMagicBytes()
	{
		IntPtr ptr = NativeMethods.GetDataPackageMagicBytes();
		if (ptr == IntPtr.Zero) return new byte[] { (byte)'Z', (byte)'D', (byte)'D' };
		byte[] magic = new byte[3];
		Marshal.Copy(ptr, magic, 0, 3);
		return magic;
	}

	#endregion // Versions and Signatures

	#region Package Entry Types (ePackageDatType)

	public static uint AssetTypeProjectManifest => NativeMethods.GetAssetTypeProjectManifest();
	public static uint AssetTypePrimaryView => NativeMethods.GetAssetTypePrimaryView();
	public static uint AssetTypeScene => NativeMethods.GetAssetTypeScene();
	public static uint AssetTypeChildView => NativeMethods.GetAssetTypeChildView();
	public static uint AssetTypeIndependentView => NativeMethods.GetAssetTypeIndependentView();
	public static uint AssetTypePrefab => NativeMethods.GetAssetTypePrefab();

	#endregion // Package Entry Types
}
