using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.ComTypes;

namespace editor.Services
{
	public static class VisualStudioDebuggerService
	{
		[DllImport("ole32.dll")]
		private static extern int GetRunningObjectTable(int reserved, out IRunningObjectTable prot);

		[DllImport("ole32.dll")]
		private static extern int CreateBindCtx(int reserved, out IBindCtx ppbc);

		public static void AttachToCurrentProcess(string? currentProjectPath)
		{
			var dteInstances = GetDTEInstances();
			if (dteInstances.Count == 0)
			{
				EditorLogger.LogWarning("Visual Studio не запущена. Пожалуйста, запустите ее и откройте скрипты.");
				return;
			}

			dynamic? targetDte = null;

			// Normalize project paths for comparison
			string? normalizedCurrentProject = NormalizePath(currentProjectPath);
			string? normalizedEnginePath = NormalizePath(System.IO.Path.GetFullPath(System.IO.Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "..", "..", "..")));

			dynamic? engineDte = null;

			foreach (var dte in dteInstances)
			{
				try
				{
					string solutionPath = dte.Solution.FullName;
					if (!string.IsNullOrEmpty(solutionPath))
					{
						string? normalizedSolutionPath = NormalizePath(solutionPath);
						if (normalizedSolutionPath != null)
						{
							if (normalizedCurrentProject != null && normalizedSolutionPath.StartsWith(normalizedCurrentProject, StringComparison.OrdinalIgnoreCase))
							{
								targetDte = dte;
								break;
							}
							if (normalizedEnginePath != null && normalizedSolutionPath.StartsWith(normalizedEnginePath, StringComparison.OrdinalIgnoreCase))
							{
								engineDte = dte;
							}
						}
					}
				}
				catch { /* Ignore errors retrieving solution path */ }
			}

			if (targetDte == null)
			{
				targetDte = engineDte;
			}

			if (targetDte == null)
			{
				if (dteInstances.Count > 1)
					EditorLogger.LogWarning("Найдено несколько открытых студий, но ни в одной не открыт текущий проект или движок. Прикрепляемся к первой попавшейся.");
				targetDte = dteInstances[0];
			}

			AttachDebugger(targetDte);
		}

		private static void AttachDebugger(dynamic dte)
		{
			try
			{
				EditorLogger.LogInfo($"[Debugger] Попытка прикрепить отладчик из студии: {dte.Solution.FullName}");
				int currentProcessId = Process.GetCurrentProcess().Id;
				foreach (dynamic process in dte.Debugger.LocalProcesses)
				{
					if (process.ProcessID == currentProcessId)
					{
						EditorLogger.LogInfo($"[Debugger] Найден целевой процесс: {process.Name} (PID: {process.ProcessID})");
						try
						{
							process.Attach();
							EditorLogger.LogInfo("[Debugger] Отладчик Visual Studio успешно прикреплен (Default Engine)!");
							return;
						}
						catch (COMException comEx)
						{
							EditorLogger.LogWarning($"[Debugger] COMException при прикреплении: HRESULT 0x{comEx.ErrorCode:X}, Message: {comEx.Message}");
							// HRESULT 0x89710016 = Debugger is already attached
							if (comEx.ErrorCode == unchecked((int)0x89710016) || comEx.Message.Contains("уже присоединен") || comEx.Message.Contains("already attached"))
							{
								EditorLogger.LogInfo("[Debugger] Отладчик Visual Studio уже прикреплен к этому процессу.");
								return;
							}
							throw;
						}
					}
				}
				EditorLogger.LogWarning("[Debugger] Не удалось найти текущий процесс в Visual Studio.");
			}
			catch (Exception ex)
			{
				EditorLogger.LogError($"Ошибка прикрепления отладчика: {ex.Message}");
			}
		}

		private static List<dynamic> GetDTEInstances()
		{
			var dteInstances = new List<dynamic>();
			IRunningObjectTable? rot = null;
			IEnumMoniker? enumMoniker = null;
			try
			{
				if (GetRunningObjectTable(0, out rot) == 0 && rot != null)
				{
					rot.EnumRunning(out enumMoniker);
					if (enumMoniker != null)
					{
						IMoniker[] monikers = new IMoniker[1];
						IntPtr fetched = IntPtr.Zero;

						while (enumMoniker.Next(1, monikers, fetched) == 0)
						{
							IBindCtx? bindCtx = null;
							try
							{
								CreateBindCtx(0, out bindCtx);
								if (bindCtx != null)
								{
									monikers[0].GetDisplayName(bindCtx, null, out string name);
									if (name.StartsWith("!VisualStudio.DTE.", StringComparison.OrdinalIgnoreCase))
									{
										rot.GetObject(monikers[0], out object comObject);
										if (comObject != null)
										{
											dteInstances.Add(comObject);
										}
									}
								}
							}
							catch { }
							finally
							{
								if (bindCtx != null) Marshal.ReleaseComObject(bindCtx);
							}
						}
					}
				}
			}
			finally
			{
				if (enumMoniker != null) Marshal.ReleaseComObject(enumMoniker);
				if (rot != null) Marshal.ReleaseComObject(rot);
			}

			return dteInstances;
		}

		private static string? NormalizePath(string? path)
		{
			if (string.IsNullOrEmpty(path)) return null;
			try
			{
				return System.IO.Path.GetFullPath(path).Replace('\\', '/');
			}
			catch { return null; }
		}
	}
}
