using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using editor.Services.Project.Infrastructure;

namespace editor.Services.Project.Assets
{
	/// <summary>
	/// Рантайм-кэш скриптовых ассетов, обнаруженных под Assets/. Владеет синхронизацией
	/// .meta файлов скриптов и даёт коду UI/сборки/game-config единый источник поиска по GUID.
	/// </summary>
	public sealed class ScriptAssetIndexService
	{
		private readonly IFileStorage _storage;
		private readonly ProjectFileWatcherService _watcher;
		private readonly ScriptAssetWatchHandler _scriptWatchHandler = new();
		private readonly Dictionary<string, ScriptAssetInfo> _byGuid = new(StringComparer.OrdinalIgnoreCase);
		private readonly Dictionary<string, List<ScriptAssetInfo>> _byClassName = new(StringComparer.Ordinal);

		private string? _projectRoot;

		public ScriptAssetIndexService(IFileStorage storage, ProjectFileWatcherService watcher)
		{
			_storage = storage;
			_watcher = watcher;
			_watcher.FileChanged += OnProjectFileChanged;
		}

		public IReadOnlyDictionary<string, ScriptAssetInfo> ByGuid => _byGuid;
		public IReadOnlyDictionary<string, List<ScriptAssetInfo>> ByClassName => _byClassName;

		public event EventHandler<ScriptAssetIndexChangedEventArgs>? Changed;

		public void OpenProject(string projectRoot)
		{
			_projectRoot = projectRoot;

			EditorLogger.LogInfo("[Script Index] Запуск полного скана проекта на .meta файлы скриптов...");
			var vfs = new ProjectFileSystem(_storage);
			vfs.SyncScriptMetaFiles(projectRoot);

			Rebuild(affectsCompilation: false);
		}

		public void CloseProject()
		{
			_projectRoot = null;
			_byGuid.Clear();
			_byClassName.Clear();
			Changed?.Invoke(this, new ScriptAssetIndexChangedEventArgs(affectsCompilation: false));
		}

		public bool TryGetByGuid(string guid, out ScriptAssetInfo info)
		{
			return _byGuid.TryGetValue(guid, out info!);
		}

		public void Rebuild(bool affectsCompilation = false)
		{
			_byGuid.Clear();
			_byClassName.Clear();

			if (string.IsNullOrEmpty(_projectRoot))
			{
				Changed?.Invoke(this, new ScriptAssetIndexChangedEventArgs(affectsCompilation));
				return;
			}

			string assetsRoot = Path.Combine(_projectRoot, "Assets");
			ScanDirectory(assetsRoot);

			foreach (var duplicate in _byClassName.Where(pair => pair.Value.Count > 1))
			{
				string paths = string.Join(", ", duplicate.Value.Select(info => info.HppPath));
				EditorLogger.LogError($"[Script Index] Дублирующееся имя класса скрипта '{duplicate.Key}' найдено в: {paths}.");
			}

			Changed?.Invoke(this, new ScriptAssetIndexChangedEventArgs(affectsCompilation));
		}

		private void OnProjectFileChanged(object? sender, ProjectFileChangedEventArgs e)
		{
			if (string.IsNullOrEmpty(_projectRoot))
			{
				return;
			}

			bool affectsCompilation = IsScriptSourceFile(e.FullPath) ||
				(e.OldFullPath != null && IsScriptSourceFile(e.OldFullPath));
			bool affectsIndex = affectsCompilation || IsScriptMetaFile(e.FullPath) ||
				(e.OldFullPath != null && IsScriptMetaFile(e.OldFullPath));

			if (!affectsIndex)
			{
				return;
			}

			switch (e.Kind)
			{
				case ProjectFileChangeKind.Created:
					HandleExternalCreate(e.FullPath);
					break;
				case ProjectFileChangeKind.Deleted:
					HandleExternalDelete(e.FullPath);
					break;
				case ProjectFileChangeKind.Renamed:
					if (e.OldFullPath != null)
					{
						HandleExternalDelete(e.OldFullPath);
					}
					HandleExternalCreate(e.FullPath);
					break;
			}

			Rebuild(affectsCompilation);
		}

		private void HandleExternalCreate(string fullPath)
		{
			if (_scriptWatchHandler.CanHandle(fullPath))
			{
				_scriptWatchHandler.OnCreated(fullPath, _storage);
			}
		}

		private void HandleExternalDelete(string fullPath)
		{
			if (_scriptWatchHandler.CanHandle(fullPath))
			{
				_scriptWatchHandler.OnDeleted(fullPath, _storage);
			}
		}

		private void ScanDirectory(string currentPath)
		{
			if (!_storage.DirectoryExists(currentPath))
			{
				return;
			}

			var entries = _storage.GetFileSystemEntries(currentPath);
			foreach (var entry in entries)
			{
				if (_storage.DirectoryExists(entry))
				{
					ScanDirectory(entry);
					continue;
				}

				if (Path.GetExtension(entry).Equals(".meta", StringComparison.OrdinalIgnoreCase))
				{
					AddMeta(entry);
				}
			}
		}

		private void AddMeta(string metaPath)
		{
			if (string.IsNullOrEmpty(_projectRoot))
			{
				return;
			}

			string basePath = Path.Combine(
				Path.GetDirectoryName(metaPath) ?? string.Empty,
				Path.GetFileNameWithoutExtension(metaPath));
			string hppPath = basePath + ".hpp";
			if (!_storage.FileExists(hppPath))
			{
				return;
			}

			var meta = ScriptMetaFile.Load(_storage, metaPath);
			if (meta == null || string.IsNullOrWhiteSpace(meta.Guid))
			{
				return;
			}

			string cppPath = basePath + ".cpp";
			var info = new ScriptAssetInfo
			{
				Guid = meta.Guid,
				ClassName = meta.ClassName,
				HppPath = ToProjectRelativePath(hppPath),
				CppPath = _storage.FileExists(cppPath) ? ToProjectRelativePath(cppPath) : string.Empty,
				MetaPath = ToProjectRelativePath(metaPath)
			};

			if (_byGuid.TryGetValue(info.Guid, out var existing))
			{
				EditorLogger.LogError($"[Script Index] Дублирующийся GUID скрипта '{info.Guid}' найден в '{existing.MetaPath}' и '{info.MetaPath}'.");
				return;
			}

			_byGuid[info.Guid] = info;
			if (!_byClassName.TryGetValue(info.ClassName, out var classItems))
			{
				classItems = new List<ScriptAssetInfo>();
				_byClassName[info.ClassName] = classItems;
			}
			classItems.Add(info);
		}

		private string ToProjectRelativePath(string fullPath)
		{
			return Path.GetRelativePath(_projectRoot!, fullPath).Replace('\\', '/');
		}

		private static bool IsScriptSourceFile(string fullPath)
		{
			string ext = Path.GetExtension(fullPath);
			return ext.Equals(".hpp", StringComparison.OrdinalIgnoreCase) ||
				ext.Equals(".cpp", StringComparison.OrdinalIgnoreCase);
		}

		private static bool IsScriptMetaFile(string fullPath)
		{
			return Path.GetExtension(fullPath).Equals(".meta", StringComparison.OrdinalIgnoreCase);
		}
	}
}
