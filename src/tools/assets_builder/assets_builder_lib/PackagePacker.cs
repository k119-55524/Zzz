using System;
using System.IO;

namespace assets_builder_lib;

public static class PackagePacker
{
	/// <summary>
	/// Упаковывает проект. <paramref name="inBuildTimestamp"/> - unix-время (UTC, миллисекунды), которое записывается
	/// в заголовки package.dat и data.dat. Если 0, генерируется автоматически.
	/// <paramref name="buildTimestamp"/> - фактически записанное значение.
	/// </summary>
	public static bool PackProject(string sourceDir, string destinationDir, eTargetPlatform targetPlatform, ulong inBuildTimestamp, out ulong buildTimestamp, string platformConfigFile = "", Action<string>? logCallback = null)
	{
		buildTimestamp = 0;
		try
		{
			bool result = NativeMethods.PackProjectNative(sourceDir, destinationDir, (uint)targetPlatform, platformConfigFile, inBuildTimestamp, out buildTimestamp);
			if (result)
			{
				logCallback?.Invoke($"Генерация чистого бинарного пакета '{AssetExtensions.GamePackageBinaryName}' в 'assets/' завершена.");
			}
			else
			{
				logCallback?.Invoke($"Ошибка упаковки проекта через нативную DLL.");
			}
			return result;
		}
		catch (Exception ex)
		{
			logCallback?.Invoke($"Ошибка упаковывания {AssetExtensions.GamePackageBinaryName}: {ex.Message}");
			return false;
		}
	}
}
