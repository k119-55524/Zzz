
namespace editor.Services
{
	/// <summary>
	/// Центральный координатор, отслеживающий необходимость пере-сборки скриптов.
	/// Используется FileSystemWatcher (при изменении файлов) и обработчик активации MainWindow
	/// (когда окно получает фокус).
	/// </summary>
	public static class ScriptRebuildCoordinator
	{
		private static volatile bool _pendingRebuild = false;
		private static string? _pendingProjectRoot;

		/// <summary>
		/// Вызывается наблюдателем, когда файл скрипта создаётся, удаляется, переименовывается или изменяется.
		/// Помечает необходимость пере-сборки и сохраняет корневой путь проекта для последующего вызова.
		/// </summary>
		public static void RequestRebuild(string projectRoot)
		{
			_pendingRebuild = true;
			_pendingProjectRoot = projectRoot;
		}

		/// <summary>
		/// Вызывается из MainWindow_Activated. Если пере-сборка была запрошена, пока окно не активно, это инициирует фактическую компиляцию сейчас.
		/// </summary>
		public static async Task ProcessPendingAsync(MainWindow mainWindow)
		{
			if (_pendingRebuild && _pendingProjectRoot != null)
			{
				// Сбросить перед ожиданием, чтобы избежать гонок, если придёт новый запрос.
				_pendingRebuild = false;
				var root = _pendingProjectRoot;
				_pendingProjectRoot = null;
				await mainWindow.CheckAndCompileScriptsAsync(forceRebuild: true, projectRoot: root);
			}
		}
	}
}
