using System;
using System.Collections.Generic;
using System.IO;
using System.Text.Json;
using assets_builder_lib.Models;

namespace assets_builder_lib.Validation;

public class ProjectValidationResult
{
    public bool IsValid => Errors.Count == 0;
    public List<string> Errors { get; } = new();
    public List<string> Warnings { get; } = new();
    public ProjectManifestModel? Manifest { get; set; }
    public PresetsContainer? PresetsContainer { get; set; }
}

public static class ProjectValidator
{
    public static ProjectValidationResult Validate(string projectPath)
    {
        var result = new ProjectValidationResult();

        if (string.IsNullOrWhiteSpace(projectPath) || !Directory.Exists(projectPath))
        {
            result.Errors.Add($"Папка проекта не существует: '{projectPath}'");
            return result;
        }

        string projJsonPath = Path.Combine(projectPath, "project.json");
        if (!File.Exists(projJsonPath))
        {
            result.Errors.Add("В корне проекта отсутствует обязательный манифест 'project.json'");
            return result;
        }

        ProjectManifestModel? manifest = null;
        try
        {
            string json = File.ReadAllText(projJsonPath);
            manifest = JsonSerializer.Deserialize<ProjectManifestModel>(json);
        }
        catch (Exception ex)
        {
            result.Errors.Add($"Ошибка синтаксиса 'project.json': {ex.Message}");
            return result;
        }

        if (manifest == null)
        {
            result.Errors.Add("Не удалось десериализовать 'project.json'");
            return result;
        }

        result.Manifest = manifest;

        // 1. Проверка обязательных полей для рантайма
        if (string.IsNullOrWhiteSpace(manifest.CompanyName))
        {
            result.Errors.Add("В 'project.json' не заполнено обязательное поле 'company_name'");
        }

        if (string.IsNullOrWhiteSpace(manifest.AppName))
        {
            result.Errors.Add("В 'project.json' не заполнено обязательное поле 'app_name'");
        }

        // Проверка start_scene: если поле заполнено, сцена должна присутствовать в списке scenes (если список не пуст)
        if (!string.IsNullOrWhiteSpace(manifest.StartScene) && manifest.Scenes.Count > 0)
        {
            if (!manifest.Scenes.Contains(manifest.StartScene))
            {
                result.Warnings.Add($"В 'project.json' стартовая сцена '{manifest.StartScene}' отсутствует в общем списке сцен проекта.");
            }
        }

        // 2. Проверка блока build_settings
        if (manifest.BuildSettings == null || string.IsNullOrWhiteSpace(manifest.BuildSettings.PresetsFile))
        {
            result.Errors.Add("В 'project.json' отсутствует блок 'build_settings' со ссылкой на 'presets_file'");
            return result;
        }

        string presetsPath = Path.Combine(projectPath, manifest.BuildSettings.PresetsFile);
        if (!File.Exists(presetsPath))
        {
            result.Errors.Add($"Файл пресетов не найден по пути: '{manifest.BuildSettings.PresetsFile}'");
            return result;
        }

        // 3. Проверка синтаксиса и содержимого presets.json
        PresetsContainer? presetsContainer = null;
        try
        {
            string presetsJson = File.ReadAllText(presetsPath);
            presetsContainer = JsonSerializer.Deserialize<PresetsContainer>(presetsJson);
        }
        catch (Exception ex)
        {
            result.Errors.Add($"Ошибка синтаксиса файла пресетов '{manifest.BuildSettings.PresetsFile}': {ex.Message}");
            return result;
        }

        if (presetsContainer == null || presetsContainer.Presets == null || presetsContainer.Presets.Count == 0)
        {
            result.Errors.Add($"Файл пресетов '{manifest.BuildSettings.PresetsFile}' не содержит ни одного набора сборки");
            return result;
        }

        result.PresetsContainer = presetsContainer;

        // Проверка наличия активного пресета
        string activePresetName = manifest.BuildSettings.ActivePreset;
        var activePreset = presetsContainer.Presets.Find(p => p.Name.Equals(activePresetName, StringComparison.OrdinalIgnoreCase))
                           ?? presetsContainer.Presets[0];

        // Проверка файлов платформ в активном пресете
        foreach (var target in activePreset.Targets)
        {
            if (target.IsEnabled && !string.IsNullOrWhiteSpace(target.ConfigFile))
            {
                string configFullPath = Path.Combine(projectPath, target.ConfigFile);
                if (!File.Exists(configFullPath))
                {
                    result.Warnings.Add($"Таргет '{target.Name}' ({target.Platform}): платформенный файл не найден по пути '{target.ConfigFile}'");
                }
            }
        }

        return result;
    }
}