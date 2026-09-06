
using System.IO;
using System.Text.Json;
using assets_builder_lib.Importers;
using assets_builder_lib.Validation;
using System.Collections.Concurrent;
using System.Runtime.InteropServices;

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

	public string GetVersion()
	{
		try
		{
			IntPtr ptr = NativeMethods.GetBuilderEngineVersion();
			return ptr != IntPtr.Zero ? Marshal.PtrToStringAnsi(ptr) ?? "1.0.0" : "1.0.0";
		}
		catch
		{
			return "1.0.0";
		}
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
		var guidToFileMap = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
		var guidToTypeMap = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
		var scriptNameToGuidMap = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
		// Имя сцены (имя файла .zs без расширения) должно быть уникально в проекте - SceneManager::LoadSceneByName
		// ищет сцену по имени в package.dat, и дубликат имени сделал бы такой поиск неоднозначным.
		var sceneNameToFileMap = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
		int sceneNameDuplicateErrors = 0;

		int totalProcessed = 0;
		int newMetaCreated = 0;
		int duplicateErrors = 0;

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
				// Если мета-файла нет — генерируем новый и сохраняем на диск
				guid = Guid.NewGuid().ToString();
				string json = importer.GenerateMetaJson(file, guid);
				File.WriteAllText(metaPath, json);
				newMetaCreated++;
				(_, assetType) = ExtractGuidAndTypeFromMeta(metaPath, file);
				Log($"  {relativePath}  ->  Добавлен новый мета-файл GUID: {guid} [{assetType}]");
			}

			// Проверка на дубликат GUID
			if (!string.IsNullOrEmpty(guid) && guid != "unknown")
			{
				if (guidToFileMap.TryGetValue(guid, out var existingFile))
				{
					duplicateErrors++;
					Log($"Ошибка: Обнаружен дубликат GUID '{guid}' в файлах:\n     1) {existingFile}\n     2) {relativePath}");
				}
				else
				{
					guidToFileMap[guid] = relativePath;
					guidToTypeMap[guid] = assetType;
				}
			}

			// Проверка на дубликат имени сцены (имя файла .zs без расширения - см. PackagePacker.cpp,
			// где оно используется как PackageEntry.Name для сцены в package.dat)
			if (assetType.Equals("scene", StringComparison.OrdinalIgnoreCase))
			{
				string sceneName = Path.GetFileNameWithoutExtension(file);
				if (sceneNameToFileMap.TryGetValue(sceneName, out var existingSceneFile))
				{
					sceneNameDuplicateErrors++;
					Log($"Ошибка: Обнаружен дубликат имени сцены '{sceneName}' в файлах:\n     1) {existingSceneFile}\n     2) {relativePath}\n     SceneManager::LoadSceneByName не сможет однозначно определить нужную сцену.");
				}
				else
				{
					sceneNameToFileMap[sceneName] = relativePath;
				}
			}

			// Регистрируем имя C++ скрипта без расширения для валидации ссылок по имени ИЛИ по GUID
			if (file.EndsWith(AssetExtensions.HeaderH, StringComparison.OrdinalIgnoreCase) ||
				file.EndsWith(AssetExtensions.HeaderHpp, StringComparison.OrdinalIgnoreCase))
			{
				string scriptName = Path.GetFileNameWithoutExtension(file);
				scriptNameToGuidMap[scriptName] = guid;
			}
		}

		// 3. Валидация связей ресурсов и типов по GUID
		int validationErrorsCount = 0;
		int warningsCount = ignoredFiles.Count;

		foreach (var file in allValidFiles)
		{
			if (file.EndsWith(".meta", StringComparison.OrdinalIgnoreCase))
				continue;

			var validator = _validators.FirstOrDefault(v => v.CanValidate(file));
			if (validator == null)
				continue;

			var result = validator.Validate(file, guidToFileMap, guidToTypeMap, scriptNameToGuidMap);
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

		bool isSuccess = (duplicateErrors == 0 && sceneNameDuplicateErrors == 0 && validationErrorsCount == 0);

		if (isSuccess)
		{
			Log($"Валидация успешно завершена. Ошибок: 0, Предупреждений: {warningsCount}");
		}
		else
		{
			Log($"Ошибка: Валидация завершена с ошибками! Ошибки: {validationErrorsCount + duplicateErrors + sceneNameDuplicateErrors}, Предупреждения: {warningsCount}");
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

	public bool BuildPackage(BuildOptions options, CancellationToken cancellationToken = default)
	{
		// 1. Автоматическое предварительное сканирование, генерация .meta и валидация
		bool isMetaValid = ScanProjectMetaFiles(options);
		if (!isMetaValid)
		{
			Log("Ошибка: Сборка отменена из-за наличия критических ошибок или дубликатов GUID в проекте.");
			return false;
		}

		Log($"Старт сборки пакета (Платформа: {options.TargetPlatform})...");
		Log($"Источник проекта: {options.SourcePath}");
		Log($"Манифест:         {options.ProjectJsonPath}");
		Log($"Папка назначения:  {options.DestinationPath}");

		if (cancellationToken.IsCancellationRequested)
		{
			Log("Сборка прервана.");
			return false;
		}

		// 2. Полная очистка папки назначения (DestinationPath)
		try
		{
			if (Directory.Exists(options.DestinationPath))
			{
				Log($"Очистка папки назначения: {options.DestinationPath}");
				Directory.Delete(options.DestinationPath, recursive: true);
			}

			string assetsDir = Path.Combine(options.DestinationPath, "assets");
			string includeDir = Path.Combine(options.DestinationPath, "include");
			string dataDir = Path.Combine(assetsDir, "data");

			Directory.CreateDirectory(assetsDir);
			Directory.CreateDirectory(includeDir);

			// Каталоги медиа-ресурсов (assets/data/<category>) - см. PackageConstants.h в движке
			// (c_TexturesDirectoryName и т.д.). Создаются заранее пустыми, чтобы Path::GetTexturesDirectory()
			// и аналогичные методы в движке всегда указывали на существующий каталог, даже если ресурсы
			// соответствующей категории в проекте пока отсутствуют.
			foreach (var category in new[] { "textures", "video", "audio", "fonts", "custom" })
			{
				Directory.CreateDirectory(Path.Combine(dataDir, category));
			}

			Log("Создана чистая структура папок (assets/, assets/data/{textures,video,audio,fonts,custom} и include/).");
		}
		catch (Exception ex)
		{
			Log($"Ошибка очистки/создания папки назначения: {ex.Message}");
			return false;
		}

		// 3. Экспорт C++ заголовочных файлов (.h / .hpp) в подпапку include/
		Log("Экспорт C++ заголовочных файлов (.h/.hpp) в подпапку include/...");
		CopyHeaderFiles(options.SourcePath, Path.Combine(options.DestinationPath, "include"));

		// 4. Вызов C# запаковщика PackagePacker для генерации бинарного пакета структуры игры
		Log($"Сериализация бинарного пакета игры '{AssetExtensions.GamePackageBinaryName}'...");
		bool packageSuccess = PackagePacker.PackProject(options.SourcePath, options.DestinationPath, options.TargetPlatform, Log);

		// 5. Генерация Scripts.cmake в корне папки назначения (options.DestinationPath)
		GenerateScriptsCmake(options.SourcePath, options.DestinationPath);

		if (packageSuccess)
		{
			Log($"Сборка пакета успешно завершена! Пакадж: {AssetExtensions.GamePackageBinaryName}");
			return true;
		}
		else
		{
			Log("Ошибка: Сериализация бинарного пакета вернула ошибку.");
			return false;
		}
	}

	public void GenerateScriptsCmake(string sourcePath, string destinationPath)
	{
		try
		{
			string scriptsPath = Path.Combine(sourcePath, "Assets", "Scripts");
			var cppFiles = new List<string>();
			var hppFiles = new List<string>();

			if (Directory.Exists(scriptsPath))
			{
				foreach (var file in Directory.GetFiles(scriptsPath, "*.*", SearchOption.AllDirectories))
				{
					string ext = Path.GetExtension(file).ToLowerInvariant();
					string fullPathNormalized = file.Replace('\\', '/');

					if (ext == ".cpp" || ext == ".c")
					{
						cppFiles.Add(fullPathNormalized);
					}
					else if (ext == ".h" || ext == ".hpp")
					{
						hppFiles.Add(fullPathNormalized);
					}
				}
			}

			// ГенерацияRegisterAllScripts.cpp с полной регистрацией классов и GUID напрямую в C# Сборщике
			var registerLines = new List<string>();
			var headerIncludes = new List<string>();

			foreach (var headerFile in hppFiles)
			{
				string className = Path.GetFileNameWithoutExtension(headerFile);
				string metaFile = headerFile + ".meta";
				if (!File.Exists(metaFile))
				{
					metaFile = Path.Combine(Path.GetDirectoryName(headerFile)!, className + ".meta");
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

				string qualifiedName = string.IsNullOrWhiteSpace(scriptNamespace) ? className : $"{scriptNamespace}::{className}";
				headerIncludes.Add($"#include \"{headerFile}\"");

				if (!string.IsNullOrWhiteSpace(scriptGuid))
				{
					registerLines.Add($"    if (auto g = zzz::core::Guid::Parse(\"{scriptGuid}\"))\n        registry.Register<{qualifiedName}>(\"{qualifiedName}\", *g);\n    else\n        registry.Register<{qualifiedName}>(\"{qualifiedName}\");");
				}
				else
				{
					registerLines.Add($"    registry.Register<{qualifiedName}>(\"{qualifiedName}\");");
				}
			}

			string registerCppPath = Path.Combine(destinationPath, "RegisterAllScripts.cpp").Replace('\\', '/');
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
			cppFiles.Add(registerCppPath);

			var sb = new System.Text.StringBuilder();
			sb.AppendLine("# Автогенерируемый файл от Assets Builder");
			sb.AppendLine("set(GAME_SCRIPT_SOURCES");
			foreach (var cpp in cppFiles)
			{
				sb.AppendLine($"    \"{cpp}\"");
			}
			sb.AppendLine(")");
			sb.AppendLine();
			sb.AppendLine("set(GAME_SCRIPT_HEADERS");
			foreach (var hpp in hppFiles)
			{
				sb.AppendLine($"    \"{hpp}\"");
			}
			sb.AppendLine(")");
			sb.AppendLine();
			sb.AppendLine("set(GAME_SCRIPT_INCLUDES");
			if (Directory.Exists(scriptsPath))
			{
				sb.AppendLine($"    \"{scriptsPath.Replace('\\', '/')}\"");
			}
			sb.AppendLine($"    \"{sourcePath.Replace('\\', '/')}\"");
			sb.AppendLine($"    \"{destinationPath.Replace('\\', '/')}\"");
			sb.AppendLine(")");

			string cmakeFilePath = Path.Combine(destinationPath, "Scripts.cmake");
			File.WriteAllText(cmakeFilePath, sb.ToString());
			Log($"Сгенерирован C++ файл регистрации: RegisterAllScripts.cpp и Scripts.cmake ({cppFiles.Count} .cpp, {hppFiles.Count} .hpp)");
		}
		catch (Exception ex)
		{
			Log($"Предупреждение при генерации Scripts.cmake: {ex.Message}");
		}
	}

	public void CopyHeaderFiles(string sourcePath, string outputIncludeDir)
	{
		try
		{
			string scriptsPath = Path.Combine(sourcePath, "Assets", "Scripts");
			if (Directory.Exists(scriptsPath))
			{
				foreach (var file in Directory.GetFiles(scriptsPath, "*.*", SearchOption.AllDirectories))
				{
					string ext = Path.GetExtension(file).ToLowerInvariant();
					if (ext == ".h" || ext == ".hpp")
					{
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

	private void Log(string message)
	{
		LogReceived?.Invoke(message);
	}
}
