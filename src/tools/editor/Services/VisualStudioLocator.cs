using System.Diagnostics;
using System.IO;

namespace editor.Services
{
	public static class VisualStudioLocator
	{
		private static string? _cachedDevenvPath;
		private static bool _devenvPathResolved;

		public static string? ResolveDevenvPath()
		{
			if (_devenvPathResolved)
			{
				return _cachedDevenvPath;
			}

			_devenvPathResolved = true;

			try
			{
				string vswherePath = Path.Combine(
					Environment.GetFolderPath(Environment.SpecialFolder.ProgramFilesX86),
					"Microsoft Visual Studio", "Installer", "vswhere.exe");

				if (!File.Exists(vswherePath))
				{
					return null;
				}

				var psi = new ProcessStartInfo
				{
					FileName = vswherePath,
					Arguments = "-latest -prerelease -property productPath",
					RedirectStandardOutput = true,
					RedirectStandardError = true,
					UseShellExecute = false,
					CreateNoWindow = true
				};

				using var process = Process.Start(psi);
				string output = process?.StandardOutput.ReadToEnd().Trim() ?? string.Empty;
				process?.WaitForExit(5000);

				if (!string.IsNullOrEmpty(output) && File.Exists(output))
				{
					_cachedDevenvPath = output;
				}
			}
			catch (Exception ex)
			{
				EditorLogger.LogWarning($"[VisualStudio] Failed to resolve devenv.exe via vswhere: {ex.Message}");
			}

			return _cachedDevenvPath;
		}
	}
}
