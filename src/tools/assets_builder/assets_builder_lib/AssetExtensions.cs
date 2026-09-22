using System.Runtime.InteropServices;

namespace assets_builder_lib;

public static class AssetExtensions
{
	public const string ProjectJsonName = "project.json";

	public static string GamePackageBinaryName => PackageConstants.GamePackageFileName;

	// Используется отдельно от IsSupportedAssetExtension - диспетчеризация SceneAssetImporter/SceneAssetValidator
	// по конкретному расширению (см. assets_builder_dll/AssetExtensions.h).
	public const string Scene = ".zscene";
	public const string View = ".zview";
	public const string Material = ".zmaterial";
	public const string Shader = ".zshaders";
	public const string Prefab = ".zprefab";

	// C++ скрипты - понятие самого Assets Builder, в core (движке) не существует, поэтому не дублирует
	// никакую C++-константу и не идёт через нативную DLL.
	public const string HeaderH = ".h";
	public const string HeaderHpp = ".hpp";
	public const string SourceCpp = ".cpp";

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

	// Ниже - единственный источник истины по типам ресурсов сборщика: assets_builder_dll/AssetExtensions.h
	// через нативную DLL, без локального дублирования и без C#-фолбэка.

	public static bool IsSupportedDataAssetExtension(string ext)
	{
		if (string.IsNullOrEmpty(ext)) return false;
		if (!ext.StartsWith('.')) ext = "." + ext;
		return NativeMethods.IsSupportedDataAssetExtension(ext);
	}

	public static bool IsSupportedViewExtension(string ext)
	{
		if (string.IsNullOrEmpty(ext)) return false;
		if (!ext.StartsWith('.')) ext = "." + ext;
		return NativeMethods.IsSupportedViewExtension(ext);
	}

	public static bool IsSupportedAssetExtension(string ext)
	{
		if (string.IsNullOrEmpty(ext)) return false;
		if (!ext.StartsWith('.')) ext = "." + ext;
		return NativeMethods.IsSupportedAssetExtension(ext);
	}
}
