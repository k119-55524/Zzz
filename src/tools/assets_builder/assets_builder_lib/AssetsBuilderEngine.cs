
using System.IO;
using System.Text.Json;
using assets_builder_lib.Importers;
using assets_builder_lib.Validation;
using System.Collections.Concurrent;

namespace assets_builder_lib;

public class AssetsBuilderEngine
{
	private readonly List<IAssetImporter> _importers;
	private readonly List<IAssetValidator> _validators;

	public event Action<string>? LogReceived;

	public AssetsBuilderEngine()
	{
		_importers = new List<IAssetImporter>
		{
			new ScriptAssetImporter(),
			new SceneAssetImporter(),
			new ViewAssetImporter(),
			new ProjectAssetImporter(),
			new DataAssetImporter()
		};

		_validators = new List<IAssetValidator>
		{
			new ProjectJsonValidator(),
			new ScriptAssetValidator(),
			new SceneAssetValidator(),
			new ViewAssetValidator(),
			new DataAssetValidator()
		};
	}

	public bool ScanProjectMetaFiles(BuildOptions options)
	{
		string sourcePath = options.SourcePath;
		if (string.IsNullOrWhiteSpace(sourcePath) || !Directory.Exists(sourcePath))
		{
			Log($"Ошибка: Путь проекта '{sourcePath}' не существует!");
			return false;
		}

		string scriptsPath = Path.Combine(sourcePath, "Assets", "Scripts");
		string assetsPath = Path.Combine(sourcePath, "Assets");

		var scriptsFiles = new ConcurrentBag<string>();
		var assetsFiles = new ConcurrentBag<string>();
		var ignoredFiles = new ConcurrentBag<string>();

		// 1. Двухпоточный параллельный сбор файлов с проверкой правил расположения в корне
		Parallel.Invoke(
			() =>
			{
				if (Directory.Exists(scriptsPath))
				{
					foreach (var file in Directory.GetFiles(scriptsPath, "*.*", SearchOption.AllDirectories))
					{
						string ext = Path.GetExtension(file);
						if (ext.Equals(".meta", StringComparison.OrdinalIgnoreCase))
						{
							scriptsFiles.Add(file);
							continue;
						}

						if (!AssetExtensions.IsScriptExtension(ext))
						{
							string rel = Path.GetRelativePath(sourcePath, file).Replace('\\', '/');
							Log($"Предупреждение: Неподдерживаемый файл '{rel}' в каталоге Assets/Scripts/ (ожидаются .h, .hpp, .cpp)! Файл проигнорирован.");
							ignoredFiles.Add(file);
							continue;
						}

						scriptsFiles.Add(file);
					}
				}
			},
			() =>
			{
				if (Directory.Exists(assetsPath))
				{
					foreach (var file in Directory.GetFiles(assetsPath, "*.*", SearchOption.AllDirectories))
					{
						// Игнорируем папку Scripts в потоке ассетов
						if (file.StartsWith(scriptsPath, StringComparison.OrdinalIgnoreCase))
							continue;

						// Правило: project.json разрешен ТОЛЬКО в корне! Если он в подпапке Assets, это предупреждение
						if (Path.GetFileName(file).Equals(AssetExtensions.ProjectJsonName, StringComparison.OrdinalIgnoreCase))
						{
							string rel = Path.GetRelativePath(sourcePath, file).Replace('\\', '/');
							Log($"Предупреждение: Файл project.json должен находиться ТОЛЬКО в корне проекта! Файл '{rel}' проигнорирован.");
							ignoredFiles.Add(file);
							continue;
						}

						string ext = Path.GetExtension(file);
						if (ext.Equals(".meta", StringComparison.OrdinalIgnoreCase))
						{
							assetsFiles.Add(file);
							continue;
						}

						// Проверка поддержки расширения ресурса через единый белый список
						if (!AssetExtensions.IsSupportedAssetExtension(ext))
						{
							string rel = Path.GetRelativePath(sourcePath, file).Replace('\\', '/');
							Log($"Предупреждение: Неподдерживаемый тип ресурса '{rel}' (расширение '{ext}')! Файл проигнорирован.");
							ignoredFiles.Add(file);
							continue;
						}

						assetsFiles.Add(file);
					}
				}

				// Добавляем root project.json
				string projJson = options.ProjectJsonPath;
				if (File.Exists(projJson))
				{
					assetsFiles.Add(projJson);
				}

				// Проверка файлов прямо в корне корневой папки (за исключением допустимых)
				foreach (var file in Directory.GetFiles(sourcePath, "*.*", SearchOption.TopDirectoryOnly))
				{
					string fileName = Path.GetFileName(file);
					if (fileName.Equals(AssetExtensions.ProjectJsonName, StringComparison.OrdinalIgnoreCase) ||
						(fileName.StartsWith("project_", StringComparison.OrdinalIgnoreCase) && fileName.EndsWith(".json", StringComparison.OrdinalIgnoreCase)) ||
						fileName.Equals("CMakeLists.txt", StringComparison.OrdinalIgnoreCase) ||
						fileName.StartsWith("RegisterAllScripts", StringComparison.OrdinalIgnoreCase))
					{
						continue; // Разрешенные корневые файлы
					}

					if (!file.EndsWith(".meta", StringComparison.OrdinalIgnoreCase))
					{
						Log($"Предупреждение: Файл '{fileName}' расположен прямо в корне проекта! Все ассеты и скрипты должны находиться в подпапке Assets/. Файл проигнорирован.");
						ignoredFiles.Add(file);
					}
				}
			}
		);

		// Исключаем .meta и .cpp из подсчета статистики количества исходных файлов проекта
		int scriptsCount = scriptsFiles.Count(f => !f.EndsWith(".meta", StringComparison.OrdinalIgnoreCase) && !f.EndsWith(AssetExtensions.SourceCpp, StringComparison.OrdinalIgnoreCase));
		int assetsCount = assetsFiles.Count(f => !f.EndsWith(".meta", StringComparison.OrdinalIgnoreCase));

		Log($"Скрипты: найдено {scriptsCount} файлов.");
		Log($"Ассеты:   найдено {assetsCount} файлов.");

		var allValidFiles = scriptsFiles.Concat(assetsFiles).Distinct().Where(f => !ignoredFiles.Contains(f)).ToList();

		// 2. Проверка мета-файлов: сохраняем существующие GUID, создаем .meta только для новых ресурсов без мета-файлов
		// Имя сцены (имя файла .zscene без расширения) должно быть уникально в проекте
		// для предотвращения коллизий идентификаторов сцен в редакторе и инструментах сборки.
		var sceneNameToFileMap = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
		int sceneNameDuplicateErrors = 0;

		int totalProcessed = 0;
		int newMetaCreated = 0;

		foreach (var file in allValidFiles)
		{
			if (file.EndsWith(".meta", StringComparison.OrdinalIgnoreCase))
				continue;
			if (file.EndsWith(AssetExtensions.SourceCpp, StringComparison.OrdinalIgnoreCase))
				continue;

			totalProcessed++;
			string relativePath = Path.GetRelativePath(sourcePath, file).Replace('\\', '/');
			var importer = _importers.FirstOrDefault(imp => imp.CanHandle(file));
			if (importer == null)
			{
				Log($"Предупреждение: Для файла '{relativePath}' не найден зарегистрированный импортер! Файл пропущен.");
				continue;
			}
			string metaPath = importer.GetMetaFilePath(file);

			string guid;
			string assetType;
			if (File.Exists(metaPath))
			{
				// Читаем существующий GUID и тип ресурса из мета-файла
				(guid, assetType) = ExtractGuidAndTypeFromMeta(metaPath, file);
				Log($"  {relativePath}  ->  GUID: {guid} [{assetType}]");
			}
			else
			{
				// Если мета-файла нет — генерируем новый через нативный Single Source of Truth и сохраняем на диск
				byte[] guidBuf = new byte[40];
				if (!NativeMethods.GenerateGuidNative(guidBuf, (uint)guidBuf.Length))
				{
					throw new InvalidOperationException($"Не удалось сгенерировать GUID через NativeMethods.GenerateGuidNative для файла '{relativePath}'.");
				}
				guid = System.Text.Encoding.ASCII.GetString(guidBuf).TrimEnd('\0');
				string json = importer.GenerateMetaJson(file, guid);
				File.WriteAllText(metaPath, json);
				newMetaCreated++;
				(_, assetType) = ExtractGuidAndTypeFromMeta(metaPath, file);
				Log($"  {relativePath}  ->  Добавлен новый мета-файл GUID: {guid} [{assetType}]");
			}

			// Проверка на дубликат имени сцены (имя файла .zscene без расширения)
			if (assetType.Equals("scene", StringComparison.OrdinalIgnoreCase))
			{
				string sceneName = Path.GetFileNameWithoutExtension(file);
				if (sceneNameToFileMap.TryGetValue(sceneName, out var existingSceneFile))
				{
					sceneNameDuplicateErrors++;
					Log($"Ошибка: Обнаружен дубликат имени сцены '{sceneName}' в файлах:\n     1) {existingSceneFile}\n     2) {relativePath}\n     Имена файлов сцен должны быть уникальными в проекте.");
				}
				else
				{
					sceneNameToFileMap[sceneName] = relativePath;
				}
			}
		}

		// 3. Валидация связей ресурсов и типов
		int validationErrorsCount = 0;
		int warningsCount = ignoredFiles.Count;

		foreach (var file in allValidFiles)
		{
			if (file.EndsWith(".meta", StringComparison.OrdinalIgnoreCase))
				continue;

			var validator = _validators.FirstOrDefault(v => v.CanValidate(file));
			if (validator == null)
				continue;

			var result = validator.Validate(file);
			if (!result.IsValid)
			{
				foreach (var err in result.Errors)
				{
					if (err.IsCritical)
					{
						validationErrorsCount++;
						Log($"Ошибка: {err.Message}");
					}
					else
					{
						warningsCount++;
						Log($"Предупреждение: {err.Message}");
					}
				}
			}
		}

		// 3.1. Нативная предсборочная валидация единого глобального пространства GUID и ссылок (Single Source of Truth)
		byte[] errBuf = new byte[2048];
		string? platformConfig = string.IsNullOrWhiteSpace(options.PlatformConfigFile) ? null : options.PlatformConfigFile;
		if (!NativeMethods.ValidateProjectIdentityNative(sourcePath, errBuf, (uint)errBuf.Length, platformConfig))
		{
			string nativeErr = System.Text.Encoding.UTF8.GetString(errBuf).TrimEnd('\0');
			if (string.IsNullOrWhiteSpace(nativeErr))
				nativeErr = "Нативная проверка идентичности проекта завершилась с ошибкой.";
			validationErrorsCount++;
			Log($"Ошибка: {nativeErr}");
		}

		// 4. Постобработка: удаление осиротевших (устаревших) .meta файлов, у которых удален исходный ресурс
		// или чье расширение не поддерживается белым списком
		int deletedOrphanedMetas = 0;

		foreach (var metaFile in Directory.GetFiles(sourcePath, "*.meta", SearchOption.AllDirectories))
		{
			// Путь целевого файла (удаляем суффикс .meta)
			string targetAssetPath = metaFile.Substring(0, metaFile.Length - 5);
			bool shouldDelete = false;
			string reason = string.Empty;

			if (!File.Exists(targetAssetPath))
			{
				shouldDelete = true;
				reason = "исходный ресурс не существует на диске";
			}
			else
			{
				string ext = Path.GetExtension(targetAssetPath);
				string fileName = Path.GetFileName(targetAssetPath);
				bool isScript = targetAssetPath.StartsWith(scriptsPath, StringComparison.OrdinalIgnoreCase);
				bool isRootProjectJson = fileName.Equals(AssetExtensions.ProjectJsonName, StringComparison.OrdinalIgnoreCase);

				if (isScript)
				{
					if (!AssetExtensions.IsScriptExtension(ext))
					{
						shouldDelete = true;
						reason = $"файл '{ext}' не является скриптом C++";
					}
				}
				else if (!isRootProjectJson && !AssetExtensions.IsSupportedAssetExtension(ext))
				{
					shouldDelete = true;
					reason = $"расширение '{ext}' не поддерживается движком";
				}
			}

			if (shouldDelete)
			{
				try
				{
					File.Delete(metaFile);
					deletedOrphanedMetas++;
					string relMetaPath = Path.GetRelativePath(sourcePath, metaFile).Replace('\\', '/');
					Log($"  Удален некорректный/осиротевший мета-файл: {relMetaPath} ({reason})");
				}
				catch (Exception ex)
				{
					Log($"Предупреждение: Не удалось удалить осиротевший мета-файл '{metaFile}': {ex.Message}");
				}
			}
		}

		bool isSuccess = (sceneNameDuplicateErrors == 0 && validationErrorsCount == 0);

		if (isSuccess)
		{
			Log($"Валидация успешно завершена. Ошибок: 0, Предупреждений: {warningsCount}");
		}
		else
		{
			Log($"Ошибка: Валидация завершена с ошибками! Ошибки: {validationErrorsCount + sceneNameDuplicateErrors}, Предупреждения: {warningsCount}");
		}

		Log($"Сканирование завершено. Ресурсных файлов: {totalProcessed}, Создано новых .meta: {newMetaCreated}, Удалено осиротевших .meta: {deletedOrphanedMetas}");

		return isSuccess;
	}

	private (string Guid, string Type) ExtractGuidAndTypeFromMeta(string metaPath, string sourceFilePath)
	{
		string guid = "unknown";
		string type = string.Empty;

		try
		{
			string json = File.ReadAllText(metaPath);
			using var doc = JsonDocument.Parse(json);
			var root = doc.RootElement;
			if (root.TryGetProperty("guid", out var guidProp))
			{
				guid = guidProp.GetString() ?? "unknown";
			}
			if (root.TryGetProperty("type", out var typeProp))
			{
				type = typeProp.GetString() ?? string.Empty;
			}
		}
		catch
		{
			// Ignore parse errors
		}

		if (string.IsNullOrEmpty(type))
		{
			string ext = Path.GetExtension(sourceFilePath).TrimStart('.').ToLowerInvariant();
			type = string.IsNullOrEmpty(ext) ? "binary" : ext;
		}

		return (guid, type);
	}

	private static string? FindRepoRoot(string path)
	{
		try
		{
			var dir = new DirectoryInfo(path);
			while (dir != null)
			{
				if (Directory.Exists(Path.Combine(dir.FullName, ".git")) ||
				    (File.Exists(Path.Combine(dir.FullName, "CMakeLists.txt")) && Directory.Exists(Path.Combine(dir.FullName, "src", "engine"))))
				{
					return dir.FullName;
				}
				dir = dir.Parent;
			}
		}
		catch
		{
		}
		return null;
	}

	private static string ToCMakePath(string fullPath, string? repoRoot)
	{
		if (!string.IsNullOrEmpty(repoRoot))
		{
			string normFull = Path.GetFullPath(fullPath);
			string normRoot = Path.GetFullPath(repoRoot);
			if (normFull.StartsWith(normRoot, StringComparison.OrdinalIgnoreCase))
			{
				string rel = Path.GetRelativePath(normRoot, normFull).Replace('\\', '/');
				return $"${{CMAKE_SOURCE_DIR}}/{rel}";
			}
		}
		return fullPath.Replace('\\', '/');
	}

	public void GenerateScriptsCmake(string sourcePath, string destinationPath, string platformConfigFile = "")
	{
		try
		{
			var removedScriptGuids = LoadRemovedScriptGuids(sourcePath, platformConfigFile);
			string? repoRoot = FindRepoRoot(sourcePath) ?? FindRepoRoot(AppDomain.CurrentDomain.BaseDirectory);

			string scriptsPath = Path.Combine(sourcePath, "Assets", "Scripts");
			var cppFiles = new List<string>();
			var hppFiles = new List<string>();

			if (Directory.Exists(scriptsPath))
			{
				foreach (var file in Directory.GetFiles(scriptsPath, "*.*", SearchOption.AllDirectories))
				{
					string ext = Path.GetExtension(file).ToLowerInvariant();

					if (IsScriptExcluded(file, removedScriptGuids))
					{
						continue;
					}

					if (ext == ".cpp" || ext == ".c")
					{
						cppFiles.Add(file);
					}
					else if (ext == ".h" || ext == ".hpp")
					{
						hppFiles.Add(file);
					}
				}
			}

			// Генерация RegisterAllScripts.cpp с полной регистрацией классов и GUID напрямую в C# Сборщике
			var registerLines = new List<string>();
			var headerIncludes = new List<string>();

			foreach (var headerFile in hppFiles)
			{
				string className = Path.GetFileNameWithoutExtension(headerFile);
				var (scriptGuid, scriptNamespace) = GetScriptMetaInfo(headerFile);

				string qualifiedName = string.IsNullOrWhiteSpace(scriptNamespace) ? className : $"{scriptNamespace}::{className}";
				string relHeader = Directory.Exists(scriptsPath)
					? Path.GetRelativePath(scriptsPath, headerFile).Replace('\\', '/')
					: Path.GetFileName(headerFile);
				headerIncludes.Add($"#include \"{relHeader}\"");

				if (!string.IsNullOrWhiteSpace(scriptGuid))
				{
					registerLines.Add($"    if (auto g = zzz::core::Guid::Parse(\"{scriptGuid}\"))\n        registry.Register<{qualifiedName}>(\"{qualifiedName}\", *g);\n    else\n        registry.Register<{qualifiedName}>(\"{qualifiedName}\");");
				}
				else
				{
					registerLines.Add($"    registry.Register<{qualifiedName}>(\"{qualifiedName}\");");
				}
			}

			string registerCppPath = Path.Combine(destinationPath, "RegisterAllScripts.cpp");
			var regSb = new System.Text.StringBuilder();
			regSb.AppendLine("// RegisterAllScripts.cpp — сгенерировано Assets Builder");
			regSb.AppendLine("#include <core/Core.h>");
			regSb.AppendLine("#include <ScriptRegistry.h>");
			regSb.AppendLine();
			foreach (var inc in headerIncludes)
			{
				regSb.AppendLine(inc);
			}
			regSb.AppendLine();
			regSb.AppendLine("extern \"C\" void RegisterAllScripts(zzz::core::ScriptRegistry& registry)");
			regSb.AppendLine("{");
			foreach (var line in registerLines)
			{
				regSb.AppendLine(line);
			}
			regSb.AppendLine("}");

			File.WriteAllText(registerCppPath, regSb.ToString());

			var sb = new System.Text.StringBuilder();
			sb.AppendLine("# Автогенерируемый файл от Assets Builder");
			sb.AppendLine("set(GAME_SCRIPT_SOURCES");
			foreach (var cpp in cppFiles)
			{
				sb.AppendLine($"    \"{ToCMakePath(cpp, repoRoot)}\"");
			}
			sb.AppendLine("    \"${CMAKE_CURRENT_LIST_DIR}/RegisterAllScripts.cpp\"");
			sb.AppendLine(")");
			sb.AppendLine();
			sb.AppendLine("set(GAME_SCRIPT_HEADERS");
			foreach (var hpp in hppFiles)
			{
				sb.AppendLine($"    \"{ToCMakePath(hpp, repoRoot)}\"");
			}
			sb.AppendLine(")");
			sb.AppendLine();
			sb.AppendLine("set(GAME_SCRIPT_INCLUDES");
			if (Directory.Exists(scriptsPath))
			{
				sb.AppendLine($"    \"{ToCMakePath(scriptsPath, repoRoot)}\"");
			}
			sb.AppendLine($"    \"{ToCMakePath(sourcePath, repoRoot)}\"");
			sb.AppendLine("    \"${CMAKE_CURRENT_LIST_DIR}\"");
			sb.AppendLine(")");

			string cmakeFilePath = Path.Combine(destinationPath, "Scripts.cmake");
			File.WriteAllText(cmakeFilePath, sb.ToString());
			Log($"Сгенерирован C++ файл регистрации: RegisterAllScripts.cpp и Scripts.cmake ({cppFiles.Count + 1} .cpp, {hppFiles.Count} .hpp)");
		}
		catch (Exception ex)
		{
			Log($"Предупреждение при генерации Scripts.cmake: {ex.Message}");
		}
	}

	public void CopyHeaderFiles(string sourcePath, string outputIncludeDir, string platformConfigFile = "")
	{
		try
		{
			var removedScriptGuids = LoadRemovedScriptGuids(sourcePath, platformConfigFile);

			string scriptsPath = Path.Combine(sourcePath, "Assets", "Scripts");
			if (Directory.Exists(scriptsPath))
			{
				foreach (var file in Directory.GetFiles(scriptsPath, "*.*", SearchOption.AllDirectories))
				{
					string ext = Path.GetExtension(file).ToLowerInvariant();
					if (ext == ".h" || ext == ".hpp")
					{
						if (IsScriptExcluded(file, removedScriptGuids))
						{
							continue;
						}

						string relPath = Path.GetRelativePath(scriptsPath, file);
						string targetPath = Path.Combine(outputIncludeDir, relPath);
						Directory.CreateDirectory(Path.GetDirectoryName(targetPath)!);
						File.Copy(file, targetPath, overwrite: true);
						Log($"  C++ Заголовок экспортирован в: include/{relPath.Replace('\\', '/')}");
					}
				}
			}

			// Проверяем корневые заголовочные файлы (.h / .hpp)
			foreach (var file in Directory.GetFiles(sourcePath, "*.*", SearchOption.TopDirectoryOnly))
			{
				string ext = Path.GetExtension(file).ToLowerInvariant();
				if (ext == ".h" || ext == ".hpp")
				{
					string targetPath = Path.Combine(outputIncludeDir, Path.GetFileName(file));
					File.Copy(file, targetPath, overwrite: true);
					Log($"  C++ Заголовок экспортирован в: include/{Path.GetFileName(file)}");
				}
			}
		}
		catch (Exception ex)
		{
			Log($"Предупреждение при экспорте заголовочных файлов: {ex.Message}");
		}
	}

	private static (string Guid, string Namespace) GetScriptMetaInfo(string scriptFilePath)
	{
		string className = Path.GetFileNameWithoutExtension(scriptFilePath);
		string metaFile = scriptFilePath + ".meta";
		if (!File.Exists(metaFile))
		{
			metaFile = Path.Combine(Path.GetDirectoryName(scriptFilePath)!, className + ".h.meta");
			if (!File.Exists(metaFile))
				metaFile = Path.Combine(Path.GetDirectoryName(scriptFilePath)!, className + ".meta");
		}

		string scriptGuid = "";
		string scriptNamespace = "";
		if (File.Exists(metaFile))
		{
			try
			{
				string metaJson = File.ReadAllText(metaFile);
				using var doc = System.Text.Json.JsonDocument.Parse(metaJson);
				if (doc.RootElement.TryGetProperty("guid", out var gElem))
					scriptGuid = gElem.GetString() ?? "";
				if (doc.RootElement.TryGetProperty("namespace", out var nElem))
					scriptNamespace = nElem.GetString() ?? "";
			}
			catch { }
		}

		return (scriptGuid, scriptNamespace);
	}

	private static bool IsScriptExcluded(string scriptFilePath, HashSet<string> removedScriptGuids)
	{
		if (removedScriptGuids.Count == 0)
			return false;

		var (guid, _) = GetScriptMetaInfo(scriptFilePath);
		return !string.IsNullOrWhiteSpace(guid) && removedScriptGuids.Contains(guid);
	}

	private static HashSet<string> LoadRemovedScriptGuids(string sourcePath, string platformConfigFile)
	{
		var removedScriptGuids = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
		if (!string.IsNullOrWhiteSpace(platformConfigFile))
		{
			string configFullPath = Path.IsPathRooted(platformConfigFile) ? platformConfigFile : Path.Combine(sourcePath, platformConfigFile);
			if (File.Exists(configFullPath))
			{
				try
				{
					string cfgJson = File.ReadAllText(configFullPath);
					using var doc = System.Text.Json.JsonDocument.Parse(cfgJson);
					if (doc.RootElement.TryGetProperty("remove_scripts", out var remProp) && remProp.ValueKind == System.Text.Json.JsonValueKind.Array)
					{
						foreach (var elem in remProp.EnumerateArray())
						{
							string? s = elem.GetString();
							if (!string.IsNullOrWhiteSpace(s))
								removedScriptGuids.Add(s);
						}
					}
				}
				catch { }
			}
		}
		return removedScriptGuids;
	}

	private void Log(string message)
	{
		LogReceived?.Invoke(message);
	}
}
