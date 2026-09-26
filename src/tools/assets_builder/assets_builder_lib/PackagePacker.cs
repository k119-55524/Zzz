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
	public static bool PackProject(string sourceDir, string destinationDir, eTargetPlatform targetPlatform, ulong inBuildTimestamp, out ulong buildTimestamp, string platformConfigFile, Action<string>? logCallback, out string errorMessage)
	{
		buildTimestamp = 0;
		errorMessage = string.Empty;
		try
		{
			byte[] errBuf = new byte[2048];
			bool result = NativeMethods.PackProjectNative(sourceDir, destinationDir, (uint)targetPlatform, platformConfigFile, inBuildTimestamp, out buildTimestamp, errBuf, (uint)errBuf.Length);
			if (result)
			{
				logCallback?.Invoke($"Генерация чистого бинарного пакета '{AssetExtensions.GamePackageBinaryName}' в 'assets/' завершена.");
			}
			else
			{
				int len = 0;
				while (len < errBuf.Length && errBuf[len] != 0) len++;
				errorMessage = System.Text.Encoding.UTF8.GetString(errBuf, 0, len);
				if (string.IsNullOrWhiteSpace(errorMessage))
					errorMessage = "Ошибка упаковки проекта через нативную DLL.";
				logCallback?.Invoke($"Ошибка упаковки проекта: {errorMessage}");
			}
			return result;
		}
		catch (Exception ex)
		{
			errorMessage = ex.Message;
			logCallback?.Invoke($"Ошибка упаковывания {AssetExtensions.GamePackageBinaryName}: {ex.Message}");
			return false;
		}
	}

	public static bool PackProject(string sourceDir, string destinationDir, eTargetPlatform targetPlatform, ulong inBuildTimestamp, out ulong buildTimestamp, string platformConfigFile = "", Action<string>? logCallback = null)
	{
		return PackProject(sourceDir, destinationDir, targetPlatform, inBuildTimestamp, out buildTimestamp, platformConfigFile, logCallback, out _);
	}

	public static bool BeginBuildSession(string projectDir, out string errorMessage)

	{
		errorMessage = string.Empty;
		byte[] buffer = new byte[2048];
		bool result = NativeMethods.BeginBuildSessionNative(projectDir, buffer, (uint)buffer.Length);
		if (!result)
		{
			int len = 0;
			while (len < buffer.Length && buffer[len] != 0) len++;
			errorMessage = System.Text.Encoding.UTF8.GetString(buffer, 0, len);
		}
		return result;
	}

	public static void EndBuildSession()
	{
		NativeMethods.EndBuildSessionNative();
	}
}

