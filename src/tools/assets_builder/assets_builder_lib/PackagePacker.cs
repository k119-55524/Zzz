using System;
using System.IO;

namespace assets_builder_lib;

public static class PackagePacker
{
	public static bool PackProject(string sourceDir, string destinationDir, Action<string>? logCallback = null)
	{
		try
		{
			bool result = NativeMethods.PackProjectNative(sourceDir, destinationDir);
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
