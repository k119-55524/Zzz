using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.ComTypes;

namespace editor.Services
{
	public static class VisualStudioDebuggerService
	{
		private const int RpcCallRejected = unchecked((int)0x80010001);
		private const int MaxAttachAttempts = 8;
		private const int AttachRetryDelayMs = 500;

		[DllImport("ole32.dll")]
		private static extern int GetRunningObjectTable(int reserved, out IRunningObjectTable prot);

		[DllImport("ole32.dll")]
		private static extern int CreateBindCtx(int reserved, out IBindCtx ppbc);

		public static bool AttachToCurrentProcess(string? currentProjectPath)
		{
			if (string.IsNullOrWhiteSpace(currentProjectPath))
			{
				EditorLogger.LogError("[Debugger] Cannot attach Visual Studio: no editor project is open.");
				return false;
			}

			if (!TryGetUserScriptsSolutionPath(currentProjectPath, out string solutionPath))
			{
				EditorLogger.LogError("[Debugger] Cannot attach Visual Studio: user-scripts solution was not found. Build/open scripts first.");
				return false;
			}

			dynamic? targetDte = EnsureUserScriptsSolutionOpen(solutionPath);
			if (targetDte == null)
			{
				return false;
			}

			var dteInstances = GetDTEInstances();
			for (int attempt = 1; attempt <= MaxAttachAttempts; attempt++)
			{
				if (AttachDebugger(targetDte, dteInstances, attempt))
				{
					return true;
				}

				if (attempt < MaxAttachAttempts)
				{
					System.Threading.Thread.Sleep(AttachRetryDelayMs);
				}
			}

			EditorLogger.LogError("[Debugger] Failed to attach Visual Studio to editor process.");
			return false;
		}

		public static bool DetachFromCurrentProcess(string? currentProjectPath)
		{
			var dteInstances = GetDTEInstances();
			if (dteInstances.Count == 0)
			{
				EditorLogger.LogWarning("[Debugger] Visual Studio is not running; nothing to detach.");
				return true;
			}

			dynamic? targetDte = FindProjectDte(dteInstances, currentProjectPath) ?? FindTargetDte(dteInstances, currentProjectPath);
			if (targetDte == null)
			{
				return false;
			}

			for (int attempt = 1; attempt <= MaxAttachAttempts; attempt++)
			{
				if (DetachDebugger(targetDte, attempt))
				{
					return true;
				}

				if (attempt < MaxAttachAttempts)
				{
					System.Threading.Thread.Sleep(AttachRetryDelayMs);
				}
			}

			EditorLogger.LogError("[Debugger] Failed to detach Visual Studio from editor process.");
			return false;
		}

		public static void LogCurrentDebuggerState(string? currentProjectPath)
		{
			var dteInstances = GetDTEInstances();
			if (dteInstances.Count == 0)
			{
				EditorLogger.LogWarning("[Debugger] Cannot inspect debugger state: Visual Studio is not running.");
				return;
			}

			dynamic? targetDte = FindProjectDte(dteInstances, currentProjectPath) ?? FindTargetDte(dteInstances, currentProjectPath);
			if (targetDte == null)
			{
				EditorLogger.LogWarning("[Debugger] Cannot inspect debugger state: target Visual Studio instance was not found.");
				return;
			}

			LogDebuggerState(targetDte);
		}

		private static dynamic? EnsureUserScriptsSolutionOpen(string solutionPath)
		{
			for (int i = 0; i < 2; i++)
			{
				var dteInstances = GetDTEInstances();
				dynamic? existingDte = FindSolutionDte(dteInstances, solutionPath);
				if (existingDte != null)
				{
					return existingDte;
				}

				if (i == 0)
				{
					string devenvPath = VisualStudioLocator.ResolveDevenvPath() ?? "devenv";
					EditorLogger.LogInfo($"[Debugger] Opening user-scripts solution in Visual Studio: {solutionPath}");
					try
					{
						Process.Start(new ProcessStartInfo
						{
							FileName = devenvPath,
							Arguments = $"\"{solutionPath}\"",
							UseShellExecute = true
						});
					}
					catch (Exception ex)
					{
						EditorLogger.LogError($"[Debugger] Failed to start Visual Studio for user-scripts solution: {ex.Message}");
						return null;
					}
				}

				for (int wait = 0; wait < 40; wait++)
				{
					System.Threading.Thread.Sleep(500);
					dynamic? dte = FindSolutionDte(GetDTEInstances(), solutionPath);
					if (dte != null)
					{
						return dte;
					}
				}
			}

			EditorLogger.LogError("[Debugger] User-scripts Visual Studio solution did not become available through DTE.");
			return null;
		}

		private static bool TryGetUserScriptsSolutionPath(string projectPath, out string solutionPath)
		{
			solutionPath = string.Empty;
			try
			{
				string buildDir = Path.Combine(projectPath, ".editor", "build");
				if (!Directory.Exists(buildDir))
				{
					return false;
				}

				string[] slnxFiles = Directory.GetFiles(buildDir, "*.slnx");
				string[] slnFiles = Directory.GetFiles(buildDir, "*.sln");
				solutionPath = slnxFiles.FirstOrDefault() ?? slnFiles.FirstOrDefault() ?? string.Empty;
				return !string.IsNullOrEmpty(solutionPath);
			}
			catch (Exception ex)
			{
				EditorLogger.LogWarning($"[Debugger] Failed to locate user-scripts solution: {ex.Message}");
				return false;
			}
		}

		private static dynamic? FindProjectDte(List<dynamic> dteInstances, string? currentProjectPath)
		{
			if (string.IsNullOrWhiteSpace(currentProjectPath))
			{
				return null;
			}

			if (TryGetUserScriptsSolutionPath(currentProjectPath, out string solutionPath))
			{
				return FindSolutionDte(dteInstances, solutionPath);
			}

			string? normalizedCurrentProject = NormalizePath(currentProjectPath);
			foreach (var dte in dteInstances)
			{
				try
				{
					string currentSolutionPath = dte.Solution.FullName;
					string? normalizedSolutionPath = NormalizePath(currentSolutionPath);
					if (normalizedCurrentProject != null &&
					    normalizedSolutionPath != null &&
					    normalizedSolutionPath.StartsWith(normalizedCurrentProject, StringComparison.OrdinalIgnoreCase))
					{
						return dte;
					}
				}
				catch
				{
				}
			}

			return null;
		}

		private static dynamic? FindSolutionDte(List<dynamic> dteInstances, string solutionPath)
		{
			string? normalizedExpected = NormalizePath(solutionPath);
			foreach (var dte in dteInstances)
			{
				try
				{
					string currentSolutionPath = dte.Solution.FullName;
					string? normalizedCurrent = NormalizePath(currentSolutionPath);
					if (normalizedExpected != null &&
					    normalizedCurrent != null &&
					    string.Equals(normalizedCurrent, normalizedExpected, StringComparison.OrdinalIgnoreCase))
					{
						return dte;
					}
				}
				catch
				{
				}
			}

			return null;
		}

		private static dynamic? FindTargetDte(List<dynamic> dteInstances, string? currentProjectPath)
		{
			dynamic? targetDte = null;
			dynamic? engineDte = null;
			string? normalizedCurrentProject = NormalizePath(currentProjectPath);
			string? normalizedEnginePath = NormalizePath(System.IO.Path.GetFullPath(System.IO.Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "..", "..", "..")));

			foreach (var dte in dteInstances)
			{
				try
				{
					string solutionPath = dte.Solution.FullName;
					if (string.IsNullOrEmpty(solutionPath))
					{
						continue;
					}

					string? normalizedSolutionPath = NormalizePath(solutionPath);
					if (normalizedSolutionPath == null)
					{
						continue;
					}

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
				catch
				{
					// Ignore COM errors while scanning Visual Studio instances.
				}
			}

			targetDte ??= engineDte;
			if (targetDte == null && dteInstances.Count > 0)
			{
				if (dteInstances.Count > 1)
				{
					EditorLogger.LogWarning("[Debugger] Several Visual Studio instances are open, but none matches the current project or engine. Using the first one.");
				}

				targetDte = dteInstances[0];
			}

			return targetDte;
		}

		private static bool AttachDebugger(dynamic dte, List<dynamic> dteInstances, int attempt)
		{
			try
			{
				EditorLogger.LogInfo($"[Debugger] Attach attempt {attempt}/{MaxAttachAttempts}: {dte.Solution.FullName}");
				int currentProcessId = Process.GetCurrentProcess().Id;
				foreach (dynamic process in dte.Debugger.LocalProcesses)
				{
					if (process.ProcessID != currentProcessId)
					{
						continue;
					}

					EditorLogger.LogInfo($"[Debugger] Found editor process: {process.Name} (PID: {process.ProcessID})");
					try
					{
						try
						{
							dynamic engines = dte.Debugger.Transports.Item("Default").Engines;
							dynamic? nativeEngine = null;
							foreach (dynamic engine in engines)
							{
								string name = engine.Name;
								if (name.Contains("Native", StringComparison.OrdinalIgnoreCase) ||
								    name.Contains("Native Code", StringComparison.OrdinalIgnoreCase) ||
								    name.Contains("Натив", StringComparison.OrdinalIgnoreCase) ||
								    name.Contains("Собствен", StringComparison.OrdinalIgnoreCase))
								{
									nativeEngine = engine;
									break;
								}
							}

							if (nativeEngine != null)
							{
								process.Attach2(new object[] { nativeEngine });
								LogDebuggerState(dte);
								if (IsCurrentProcessDebuggedBy(dte))
								{
									EditorLogger.LogInfo("[Debugger] Visual Studio attached to editor (Native Engine).");
									return true;
								}

								EditorLogger.LogError("[Debugger] Native attach returned without an error, but this Visual Studio instance does not list editor.exe as a debugged process.");
								return false;
							}
						}
						catch (COMException comEx) when (comEx.ErrorCode == RpcCallRejected)
						{
							EditorLogger.LogWarning("[Debugger] Visual Studio rejected Native Attach because it is busy (RPC_E_CALL_REJECTED), retrying...");
							return false;
						}
						catch
						{
						}

						process.Attach();
						LogDebuggerState(dte);
						if (IsCurrentProcessDebuggedBy(dte))
						{
							EditorLogger.LogInfo("[Debugger] Visual Studio attached to editor (Default Engine).");
							return true;
						}

						EditorLogger.LogError("[Debugger] Attach returned without an error, but this Visual Studio instance does not list editor.exe as a debugged process.");
						return false;
					}
					catch (COMException comEx)
					{
						EditorLogger.LogWarning($"[Debugger] Attach COMException: HRESULT 0x{comEx.ErrorCode:X}, Message: {comEx.Message}");
						if (comEx.ErrorCode == RpcCallRejected)
						{
							EditorLogger.LogWarning("[Debugger] Visual Studio rejected Attach because it is busy (RPC_E_CALL_REJECTED), retrying...");
							return false;
						}

						if (comEx.ErrorCode == unchecked((int)0x89710016) ||
						    comEx.ErrorCode == unchecked((int)0x80040001) ||
						    comEx.Message.Contains("already attached", StringComparison.OrdinalIgnoreCase) ||
						    comEx.Message.Contains("already being debugged", StringComparison.OrdinalIgnoreCase))
						{
							LogDebuggerState(dte);
							if (IsCurrentProcessDebuggedBy(dte))
							{
								EditorLogger.LogInfo("[Debugger] Visual Studio is already attached to editor.");
								return true;
							}

							if (TryDetachOtherVisualStudioDebugger(dteInstances, dte))
							{
								EditorLogger.LogWarning("[Debugger] Detached another Visual Studio instance from editor.exe; retrying attach to the user-scripts solution.");
								return false;
							}

							EditorLogger.LogError("[Debugger] editor.exe is already being debugged, but not by this Visual Studio instance. Close the other debugger/session or restart editor.exe, then press the key again.");
							return false;
						}

						throw;
					}
				}

				EditorLogger.LogWarning("[Debugger] Could not find editor process in Visual Studio LocalProcesses.");
				return false;
			}
			catch (Exception ex)
			{
				EditorLogger.LogError($"[Debugger] Attach failed: {ex.Message}");
				return false;
			}
		}

		private static bool DetachDebugger(dynamic dte, int attempt)
		{
			try
			{
				EditorLogger.LogInfo($"[Debugger] Detach attempt {attempt}/{MaxAttachAttempts}: {dte.Solution.FullName}");
				int currentProcessId = Process.GetCurrentProcess().Id;
				foreach (dynamic process in dte.Debugger.DebuggedProcesses)
				{
					if (process.ProcessID != currentProcessId)
					{
						continue;
					}

					EditorLogger.LogInfo($"[Debugger] Detaching Visual Studio from editor process (PID: {process.ProcessID}).");
					TryDetachProcess(process);
					return true;
				}

				EditorLogger.LogInfo("[Debugger] Visual Studio is not attached to editor process.");
				return true;
			}
			catch (COMException comEx) when (comEx.ErrorCode == RpcCallRejected)
			{
				EditorLogger.LogWarning("[Debugger] Visual Studio rejected Detach because it is busy (RPC_E_CALL_REJECTED), retrying...");
				return false;
			}
			catch (Exception ex)
			{
				EditorLogger.LogError($"[Debugger] Detach failed: {ex.Message}");
				return false;
			}
		}

		private static bool TryDetachOtherVisualStudioDebugger(List<dynamic> dteInstances, dynamic targetDte)
		{
			int currentProcessId = Process.GetCurrentProcess().Id;

			foreach (dynamic dte in dteInstances)
			{
				try
				{
					if (ReferenceEquals(dte, targetDte))
					{
						continue;
					}

					foreach (dynamic process in dte.Debugger.DebuggedProcesses)
					{
						if (process.ProcessID != currentProcessId)
						{
							continue;
						}

						string solutionPath = SafeGet(() => dte.Solution.FullName);
						EditorLogger.LogWarning($"[Debugger] editor.exe is currently debugged by another Visual Studio instance: '{solutionPath}'. Detaching it before attaching the user-scripts solution.");
						TryDetachProcess(process);
						return true;
					}
				}
				catch (COMException comEx) when (comEx.ErrorCode == RpcCallRejected)
				{
					EditorLogger.LogWarning("[Debugger] Another Visual Studio instance rejected debugger inspection/detach because it is busy (RPC_E_CALL_REJECTED).");
				}
				catch (Exception ex)
				{
					EditorLogger.LogWarning($"[Debugger] Failed to inspect another Visual Studio instance for debugger ownership: {ex.Message}");
				}
			}

			return false;
		}

		private static void TryDetachProcess(dynamic process)
		{
			try
			{
				process.Detach(false);
			}
			catch
			{
				process.Detach();
			}
		}

		private static void LogDebuggerState(dynamic dte)
		{
			LogVisualStudioDebugState(dte);
			LogLoadedScriptModules();
			LogBreakpoints(dte);
		}

		private static bool IsCurrentProcessDebuggedBy(dynamic dte)
		{
			try
			{
				int currentProcessId = Process.GetCurrentProcess().Id;
				foreach (dynamic process in dte.Debugger.DebuggedProcesses)
				{
					if (process.ProcessID == currentProcessId)
					{
						return true;
					}
				}
			}
			catch (Exception ex)
			{
				EditorLogger.LogWarning($"[Debugger] Failed to verify Visual Studio debugged processes: {ex.Message}");
			}

			return false;
		}

		private static void LogVisualStudioDebugState(dynamic dte)
		{
			try
			{
				string currentMode = SafeGet(() => dte.Debugger.CurrentMode);
				int currentProcessId = Process.GetCurrentProcess().Id;
				int count = 0;
				bool ownsEditor = false;
				foreach (dynamic process in dte.Debugger.DebuggedProcesses)
				{
					count++;
					int processId = process.ProcessID;
					string processName = SafeGet(() => process.Name);
					if (processId == currentProcessId)
					{
						ownsEditor = true;
					}

					EditorLogger.LogInfo($"[Debugger] VS debugged process {count}: pid={processId}; name='{processName}'");
				}

				EditorLogger.LogInfo($"[Debugger] VS CurrentMode={currentMode}; debuggedProcessCount={count}; ownsEditor={ownsEditor}");
			}
			catch (Exception ex)
			{
				EditorLogger.LogWarning($"[Debugger] Failed to inspect Visual Studio debug state: {ex.Message}");
			}
		}

		private static void LogLoadedScriptModules()
		{
			try
			{
				bool found = false;
				using Process currentProcess = Process.GetCurrentProcess();
				foreach (ProcessModule module in currentProcess.Modules)
				{
					string fileName = System.IO.Path.GetFileName(module.FileName);
					bool isScriptModule =
						fileName.Equals("scripts.dll", StringComparison.OrdinalIgnoreCase) ||
						(fileName.StartsWith("scripts_temp_", StringComparison.OrdinalIgnoreCase) &&
						 fileName.EndsWith(".dll", StringComparison.OrdinalIgnoreCase));

					if (!isScriptModule)
					{
						continue;
					}

					found = true;
					string pdbPath = System.IO.Path.ChangeExtension(module.FileName, ".pdb");
					bool pdbExists = System.IO.File.Exists(pdbPath);
					EditorLogger.LogInfo($"[Debugger] Loaded script module: {module.FileName}; matching PDB: {(pdbExists ? pdbPath : "missing")}");
				}

				if (!found)
				{
					EditorLogger.LogWarning("[Debugger] No scripts.dll/scripts_temp_*.dll module is loaded yet. Start/reload scripts before expecting user breakpoints to bind.");
				}
			}
			catch (Exception ex)
			{
				EditorLogger.LogWarning($"[Debugger] Failed to inspect loaded script modules: {ex.Message}");
			}
		}

		private static void LogBreakpoints(dynamic dte)
		{
			try
			{
				int count = 0;
				foreach (dynamic breakpoint in dte.Debugger.Breakpoints)
				{
					count++;
					string enabled = SafeGet(() => breakpoint.Enabled);
					string file = SafeGet(() => breakpoint.File);
					string line = SafeGet(() => breakpoint.FileLine);
					string name = SafeGet(() => breakpoint.Name);
					string functionName = SafeGet(() => breakpoint.FunctionName);
					EditorLogger.LogInfo($"[Debugger] Breakpoint {count}: enabled={enabled}; file='{file}'; line={line}; name='{name}'; function='{functionName}'");
				}

				if (count == 0)
				{
					EditorLogger.LogWarning("[Debugger] Visual Studio reports zero breakpoints. Set script breakpoints in the opened user-scripts solution before pressing Play.");
				}
			}
			catch (Exception ex)
			{
				EditorLogger.LogWarning($"[Debugger] Failed to inspect Visual Studio breakpoints: {ex.Message}");
			}
		}

		private static string SafeGet(Func<object?> getter)
		{
			try
			{
				return getter()?.ToString() ?? string.Empty;
			}
			catch (Exception ex)
			{
				return $"<unavailable: {ex.Message}>";
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
							catch
							{
							}
							finally
							{
								if (bindCtx != null)
								{
									Marshal.ReleaseComObject(bindCtx);
								}
							}
						}
					}
				}
			}
			finally
			{
				if (enumMoniker != null)
				{
					Marshal.ReleaseComObject(enumMoniker);
				}

				if (rot != null)
				{
					Marshal.ReleaseComObject(rot);
				}
			}

			return dteInstances;
		}

		private static string? NormalizePath(string? path)
		{
			if (string.IsNullOrEmpty(path))
			{
				return null;
			}

			try
			{
				return System.IO.Path.GetFullPath(path).Replace('\\', '/');
			}
			catch
			{
				return null;
			}
		}
	}
}
