using System.Collections.Generic;

namespace editor.Services.Project
{
    public class ValidationErrorItem
    {
        public string FilePath { get; set; } = string.Empty;
        public string ErrorMessage { get; set; } = string.Empty;
    }

    public class ProjectFileSchema
    {
        public string RelativePath { get; set; } = string.Empty;
        public bool IsRequired { get; set; }
        public IFileParser? Parser { get; set; }
        public string DefaultContent { get; set; } = string.Empty;
    }

    public static class ProjectStructure
    {
        // Обязательные папки проекта
        public static readonly List<string> RequiredDirectories = new()
        {
            "Assets",
            "Configs",
            "Scenes"
        };

        // Схема файлов проекта
        public static readonly List<ProjectFileSchema> RequiredFiles = new()
        {
            new ProjectFileSchema
            {
                RelativePath = "project.zzz",
                IsRequired = true,
                Parser = new ProjectSettingsParser(),
                DefaultContent = "{\n  \"version\": \"1.0.0\",\n  \"name\": \"NewProject\"\n}"
            },
            new ProjectFileSchema
            {
                RelativePath = "Configs/engine.config",
                IsRequired = true,
                Parser = new EngineConfigParser(),
                DefaultContent = "Graphics.Width=1280\nGraphics.Height=720\nGraphics.Fullscreen=false"
            }
        };
    }
}
