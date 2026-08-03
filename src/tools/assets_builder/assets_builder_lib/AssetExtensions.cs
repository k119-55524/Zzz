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
	public const string View = ".zv";
	public const string HeaderH = ".h";
	public const string HeaderHpp = ".hpp";
	public const string SourceCpp = ".cpp";
}
