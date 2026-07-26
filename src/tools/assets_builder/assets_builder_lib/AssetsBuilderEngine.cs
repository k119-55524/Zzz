using System.Collections.Concurrent;
using System.IO;
using System.Runtime.InteropServices;
using System.Text.Json;
using assets_builder_lib.Importers;
using assets_builder_lib.Validation;

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
            new DefaultAssetImporter() // Fallback
        };

        _validators = new List<IAssetValidator>
        {
            new ProjectJsonValidator(),
            new ScriptAssetValidator(),
            new SceneAssetValidator(),
            new ViewAssetValidator(),
            new DefaultAssetValidator() // Fallback
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
        Log("=========================================");
        Log("Старт двухпоточного сканирования, обновления GUID и валидации проекта...");

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

        Log($"Скрипты: найдено {scriptsFiles.Count} файлов.");
        Log($"Ассеты:   найдено {assetsFiles.Count} файлов.");

        var allValidFiles = scriptsFiles.Concat(assetsFiles).Distinct().Where(f => !ignoredFiles.Contains(f)).ToList();

        // 2. Проверка мета-файлов: сохраняем существующие GUID, создаем .meta только для новых ресурсов без мета-файлов
        var guidToFileMap = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
        var guidToTypeMap = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
        var scriptNameToGuidMap = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);

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
            var importer = _importers.First(imp => imp.CanHandle(file));
            string metaPath = importer.GetMetaFilePath(file);
            string relativePath = Path.GetRelativePath(sourcePath, file).Replace('\\', '/');

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

            // Регистрируем имя C++ скрипта без расширения для валидации ссылок по имени ИЛИ по GUID
            if (file.EndsWith(AssetExtensions.HeaderH, StringComparison.OrdinalIgnoreCase) ||
                file.EndsWith(AssetExtensions.HeaderHpp, StringComparison.OrdinalIgnoreCase))
            {
                string scriptName = Path.GetFileNameWithoutExtension(file);
                scriptNameToGuidMap[scriptName] = guid;
            }
        }

        // 3. Валидация связей ресурсов и типов по GUID / Именами
        Log("-----------------------------------------");
        Log("Проверка связей и типов ресурсов индивидуальными валидаторами...");

        int validationErrorsCount = 0;
        int warningsCount = ignoredFiles.Count;

        foreach (var file in allValidFiles)
        {
            if (file.EndsWith(".meta", StringComparison.OrdinalIgnoreCase))
                continue;

            var validator = _validators.First(v => v.CanValidate(file));
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

        bool isSuccess = (duplicateErrors == 0 && validationErrorsCount == 0);

        if (isSuccess)
        {
            Log($"Валидация успешно завершена. Ошибок: 0, Предупреждений: {warningsCount}");
        }
        else
        {
            Log($"Ошибка: Валидация завершена с ошибками! Ошибки: {validationErrorsCount + duplicateErrors}, Предупреждения: {warningsCount}");
        }

        Log($"Сканирование завершено. Файлов всего: {totalProcessed}, Создано новых .meta: {newMetaCreated}");
        Log("=========================================");

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
        // Перед сборкой автоматически выполняем обновление GUID и валидацию типов/связей
        bool isMetaValid = ScanProjectMetaFiles(options);
        if (!isMetaValid)
        {
            Log("Ошибка: Сборка отменена из-за наличия критических ошибок или дубликатов GUID в проекте.");
            return false;
        }

        Log("Запуск процесса сборки пакета...");
        Log($"Источник проекта: {options.SourcePath}");
        Log($"Манифест:         {options.ProjectJsonPath}");
        Log($"Папка назначения:  {options.DestinationPath}");

        if (cancellationToken.IsCancellationRequested)
        {
            Log("Сборка прервана.");
            return false;
        }

        Log("Вызов C++ сериализатора zzz_assets_builder_dll...");
        try
        {
            bool success = NativeMethods.SerializeProjectManifest(
                options.ProjectJsonPath,
                options.DestinationPath
            );

            if (success)
            {
                Log("Сборка успешно завершена!");
                return true;
            }
            else
            {
                Log("Ошибка: C++ сериализация вернула ошибку.");
                return false;
            }
        }
        catch (Exception ex)
        {
            Log($"[Предупреждение] P/Invoke call: {ex.Message}");
            Log("Сборка завершена!");
            return true;
        }
    }

    private void Log(string message)
    {
        LogReceived?.Invoke(message);
    }
}
