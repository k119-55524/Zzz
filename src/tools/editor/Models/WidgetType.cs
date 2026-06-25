namespace editor.Models
{
    public enum WidgetType
    {
        World,       // Мир / Вьюпорт рендера сцены
        Game,        // Игра / Вьюпорт геймплея
        Inspector,   // Инспектор свойств объектов
        SceneTree,   // Дерево иерархии объектов
        Assets,      // Браузер файлов и ресурсов
        Console,     // Консоль логов
        Settings,    // Настройки проекта/редактора
        Build        // Конфигурация сборки и экспорта
    }

    public class WidgetMetadata
    {
        // 0 (или RequiredWidgetLimit) - обязательный виджет, строго один, нельзя закрыть.
        public const int RequiredWidgetLimit = 0;

        public WidgetType Type { get; }
        public string TitleKey { get; }
        public string SystemName { get; }
        public int MaxInstancesCount { get; }

        public WidgetMetadata(WidgetType type, string titleKey, string systemName, int maxInstancesCount)
        {
            Type = type;
            TitleKey = titleKey;
            SystemName = systemName;
            MaxInstancesCount = maxInstancesCount;
        }

        public bool IsRequired => MaxInstancesCount == RequiredWidgetLimit;
    }

    public static class WidgetRules
    {
        public static WidgetMetadata GetMetadata(WidgetType type)
        {
            return type switch
            {
                WidgetType.World => new WidgetMetadata(WidgetType.World, "Widget_World_Title", "WorldWidget", WidgetMetadata.RequiredWidgetLimit),
                WidgetType.Game => new WidgetMetadata(WidgetType.Game, "Widget_Game_Title", "GameWidget", WidgetMetadata.RequiredWidgetLimit),
                WidgetType.Inspector => new WidgetMetadata(WidgetType.Inspector, "Widget_Inspector_Title", "InspectorWidget", 1),
                WidgetType.SceneTree => new WidgetMetadata(WidgetType.SceneTree, "Widget_SceneTree_Title", "SceneTreeWidget", 1),
                WidgetType.Assets => new WidgetMetadata(WidgetType.Assets, "Widget_Assets_Title", "AssetsWidget", int.MaxValue),
                WidgetType.Console => new WidgetMetadata(WidgetType.Console, "Widget_Console_Title", "ConsoleWidget", 1),
                WidgetType.Settings => new WidgetMetadata(WidgetType.Settings, "Widget_Settings_Title", "SettingsWidget", 1),
                WidgetType.Build => new WidgetMetadata(WidgetType.Build, "Widget_Build_Title", "BuildWidget", 1),
                _ => throw new System.ArgumentOutOfRangeException(nameof(type), type, null)
            };
        }
    }
}
