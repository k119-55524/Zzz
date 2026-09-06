using System.Runtime.InteropServices;

namespace assets_builder_lib;

public static class AssetExtensions
{
	public const string ProjectJsonName = "project.json";

	public static string GamePackageBinaryName
	{
		get
		{
			try
			{
				IntPtr ptr = NativeMethods.GetGamePackageFileName();
				return ptr != IntPtr.Zero ? Marshal.PtrToStringAnsi(ptr) ?? "game.dat" : "game.dat";
			}
			catch
			{
				return "game.dat";
			}
		}
	}

	public const string Scene = ".zs";
	public const string PrimaryView = ".zav";
	public const string ChildView = ".zcv";
	public const string IndependentView = ".ziv";
	public const string Prefab = ".zp";

	// Data assets (data.dat / dedicated folders)
	public const string MeshObj = ".obj";
	public const string TexturePng = ".png";
	public const string Material = ".zmat";
	public const string ShaderHlsl = ".hlsl";
	public const string Meta = ".meta";

	// C++ scripts
	public const string HeaderH = ".h";
	public const string HeaderHpp = ".hpp";
	public const string SourceCpp = ".cpp";

	private static readonly HashSet<string> s_KnownAssetExtensions = new(StringComparer.OrdinalIgnoreCase)
	{
		Scene,
		PrimaryView,
		ChildView,
		IndependentView,
		Prefab,
		MeshObj,
		TexturePng,
		Material,
		ShaderHlsl
	};

	private static readonly HashSet<string> s_KnownDataAssetExtensions = new(StringComparer.OrdinalIgnoreCase)
	{
		Prefab,
		MeshObj,
		TexturePng,
		Material,
		ShaderHlsl
	};

	private static readonly HashSet<string> s_KnownScriptExtensions = new(StringComparer.OrdinalIgnoreCase)
	{
		HeaderH,
		HeaderHpp,
		SourceCpp
	};

	public static bool IsScriptExtension(string ext)
	{
		if (string.IsNullOrEmpty(ext)) return false;
		if (!ext.StartsWith('.')) ext = "." + ext;
		return s_KnownScriptExtensions.Contains(ext);
	}

	public static bool IsSupportedDataAssetExtension(string ext)
	{
		if (string.IsNullOrEmpty(ext)) return false;
		if (!ext.StartsWith('.')) ext = "." + ext;
		return s_KnownDataAssetExtensions.Contains(ext);
	}

	public static bool IsSupportedViewExtension(string ext)
	{
		if (string.IsNullOrEmpty(ext)) return false;
		if (!ext.StartsWith('.')) ext = "." + ext;
		return ext.Equals(PrimaryView, StringComparison.OrdinalIgnoreCase) ||
		       ext.Equals(ChildView, StringComparison.OrdinalIgnoreCase) ||
		       ext.Equals(IndependentView, StringComparison.OrdinalIgnoreCase);
	}

	public static bool IsSupportedAssetExtension(string ext)
	{
		if (string.IsNullOrEmpty(ext)) return false;
		if (!ext.StartsWith('.')) ext = "." + ext;

		try
		{
			return NativeMethods.IsSupportedAssetExtension(ext);
		}
		catch
		{
			return s_KnownAssetExtensions.Contains(ext);
		}
	}
}
