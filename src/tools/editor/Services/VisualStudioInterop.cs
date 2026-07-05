
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.ComTypes;

namespace editor.Services
{
	public static class VisualStudioInterop
	{
		[DllImport("ole32.dll")]
		private static extern int GetRunningObjectTable(uint reserved, out IRunningObjectTable pprot);

		[DllImport("ole32.dll")]
		private static extern int CreateBindCtx(uint reserved, out IBindCtx ppbc);

		public static bool OpenFileInSolution(string slnPath, string[] filesToOpen)
		{
			try
			{
				IRunningObjectTable? rot = null;
				GetRunningObjectTable(0, out rot);
				if (rot == null) return false;

				rot.EnumRunning(out IEnumMoniker enumMoniker);
				enumMoniker.Reset();
				IMoniker[] monikers = new IMoniker[1];
				IntPtr fetched = IntPtr.Zero;

				CreateBindCtx(0, out IBindCtx bindCtx);

				// Поиск всех запущенных инстансов DTE
				while (enumMoniker.Next(1, monikers, fetched) == 0)
				{
					monikers[0].GetDisplayName(bindCtx, null, out string displayName);
					if (displayName != null && displayName.StartsWith("!VisualStudio.DTE."))
					{
						rot.GetObject(monikers[0], out object dteObject);
						if (dteObject != null)
						{
							try
							{
								// Получаем свойства через позднюю привязку (dynamic / reflection)
								// чтобы не добавлять COM-референсы на EnvDTE в проект
								dynamic dte = dteObject;
								dynamic solution = dte.Solution;
								if (solution != null)
								{
									string currentSlnPath = solution.FullName;
									if (!string.IsNullOrEmpty(currentSlnPath) &&
										currentSlnPath.Equals(slnPath, StringComparison.OrdinalIgnoreCase))
									{
										// Нашли нужный инстанс VS, открываем файлы!
										dynamic itemOperations = dte.ItemOperations;
										int newlyOpenedCount = 0;

										foreach (string rawFile in filesToOpen)
										{
											string file = System.IO.Path.GetFullPath(rawFile);
											bool opened = false;
											
											// 1. Проверяем, не открыт ли уже документ
											try
											{
												foreach (dynamic doc in dte.Documents)
												{
													string docPath = System.IO.Path.GetFullPath(doc.FullName);
													if (string.Equals(docPath, file, StringComparison.OrdinalIgnoreCase))
													{
														doc.Activate();
														opened = true;
														break;
													}
												}
											}
											catch (Exception ex)
											{
												EditorLogger.LogWarning($"[VisualStudioInterop] Не удалось проверить открытые документы для {file}: {ex.Message}");
											}

											// 2. Ищем через ProjectItem
											if (!opened)
											{
												try
												{
													dynamic item = solution.FindProjectItem(file);
													if (item != null)
													{
														dynamic window = item.Open();
														if (window != null)
														{
															window.Visible = true;
															window.Activate();
															opened = true;
															newlyOpenedCount++;
														}
													}
												}
												catch (Exception ex)
												{
													EditorLogger.LogWarning($"[VisualStudioInterop] Не удалось найти или открыть ProjectItem для {file}: {ex.Message}");
												}
											}

											// 3. Фолбек на ItemOperations.OpenFile
											if (!opened)
											{
												try
												{
													// Constants.vsViewKindPrimary == "{00000000-0000-0000-0000-000000000000}"
													itemOperations.OpenFile(file, "{00000000-0000-0000-0000-000000000000}");
													newlyOpenedCount++;
												}
												catch (Exception ex)
												{
													EditorLogger.LogError($"[VisualStudioInterop] Не удалось открыть файл через ItemOperations {file}: {ex.Message}");
												}
											}
										}

										// Пробуем вывести окно VS на передний план
										try
										{
											dynamic mainWindow = dte.MainWindow;
											mainWindow.Activate();
										}
										catch (Exception ex)
										{
											EditorLogger.LogWarning($"[VisualStudioInterop] Не удалось активировать главное окно: {ex.Message}");
										}

										return true;
									}
								}
							}
							catch (Exception ex)
							{
								// Игнорируем ошибки доступа к COM-объектам конкретного инстанса
								EditorLogger.LogWarning($"[VisualStudioInterop] Не удалось получить доступ к инстансу DTE: {ex.Message}");
							}
						}
					}
				}
			}
			catch (Exception ex)
			{
				EditorLogger.LogError($"[VisualStudioInterop] COM-исключение: {ex.Message}");
			}

			return false;
		}
	}
}
