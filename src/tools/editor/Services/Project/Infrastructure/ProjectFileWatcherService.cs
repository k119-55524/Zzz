using System;
using System.IO;

namespace editor.Services.Project.Infrastructure
{
	public enum ProjectFileChangeKind
	{
		Created,
		Deleted,
		Renamed,
		Changed
	}

	public sealed class ProjectFileChangedEventArgs : EventArgs
	{
		public ProjectFileChangedEventArgs(ProjectFileChangeKind kind, string fullPath, string? oldFullPath = null)
		{
			Kind = kind;
			FullPath = fullPath;
			OldFullPath = oldFullPath;
		}

		public ProjectFileChangeKind Kind { get; }
		public string FullPath { get; }
		public string? OldFullPath { get; }
	}

	/// <summary>
	/// Владеет наблюдателем за Assets/ проекта и публикует сырые события файловой системы
	/// сервисам проекта. Потребители сами решают, как в ответ менять кэши, конфиги и UI.
	/// </summary>
	public sealed class ProjectFileWatcherService : IDisposable
	{
		private FileSystemWatcher? _assetsWatcher;

		public string? CurrentProjectRoot { get; private set; }

		public event EventHandler<ProjectFileChangedEventArgs>? FileChanged;

		public void OpenProject(string projectRoot)
		{
			CloseProject();

			CurrentProjectRoot = projectRoot;
			string assetsRoot = Path.Combine(projectRoot, "Assets");
			if (!Directory.Exists(assetsRoot))
			{
				return;
			}

			_assetsWatcher = new FileSystemWatcher(assetsRoot)
			{
				IncludeSubdirectories = true,
				NotifyFilter = NotifyFilters.FileName | NotifyFilters.LastWrite
			};

			_assetsWatcher.Created += OnCreated;
			_assetsWatcher.Deleted += OnDeleted;
			_assetsWatcher.Renamed += OnRenamed;
			_assetsWatcher.Changed += OnChanged;
			_assetsWatcher.EnableRaisingEvents = true;

			EditorLogger.LogInfo($"[Project Watcher] Начато наблюдение за '{assetsRoot}'.");
		}

		public void CloseProject()
		{
			if (_assetsWatcher != null)
			{
				_assetsWatcher.EnableRaisingEvents = false;
				_assetsWatcher.Created -= OnCreated;
				_assetsWatcher.Deleted -= OnDeleted;
				_assetsWatcher.Renamed -= OnRenamed;
				_assetsWatcher.Changed -= OnChanged;
				_assetsWatcher.Dispose();
				_assetsWatcher = null;
			}

			CurrentProjectRoot = null;
		}

		private void OnCreated(object sender, FileSystemEventArgs e)
		{
			FileChanged?.Invoke(this, new ProjectFileChangedEventArgs(ProjectFileChangeKind.Created, e.FullPath));
		}

		private void OnDeleted(object sender, FileSystemEventArgs e)
		{
			FileChanged?.Invoke(this, new ProjectFileChangedEventArgs(ProjectFileChangeKind.Deleted, e.FullPath));
		}

		private void OnRenamed(object sender, RenamedEventArgs e)
		{
			FileChanged?.Invoke(this, new ProjectFileChangedEventArgs(ProjectFileChangeKind.Renamed, e.FullPath, e.OldFullPath));
		}

		private void OnChanged(object sender, FileSystemEventArgs e)
		{
			FileChanged?.Invoke(this, new ProjectFileChangedEventArgs(ProjectFileChangeKind.Changed, e.FullPath));
		}

		public void Dispose()
		{
			CloseProject();
		}
	}
}
