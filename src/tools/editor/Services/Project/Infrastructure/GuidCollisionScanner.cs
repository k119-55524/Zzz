using System.Collections.Generic;

namespace editor.Services.Project.Infrastructure
{
    /// <summary>
    /// Группа путей, в .meta которых найден один и тот же GUID (физическая коллизия идентичности,
    /// например - копия скрипта через Проводник). Разрешение коллизий (диалог выбора владельца GUID,
    /// автоперегенерация для остальных) сознательно отложено до отдельной задачи.
    /// </summary>
    public class GuidCollisionGroup
    {
        public string Guid { get; set; } = string.Empty;
        public List<string> ConflictingAssetMetaPaths { get; set; } = new();
    }

    /// <summary>
    /// Точка интеграции для будущей проверки дублей GUID. Сейчас намеренно не делает ничего -
    /// решение по обработке коллизий ещё не принято (см. обсуждение архитектуры meta/GUID),
    /// поэтому сканер уже встроен в SyncAssetMetaFiles, но возвращает пустой результат.
    /// </summary>
    public static class GuidCollisionScanner
    {
        public static List<GuidCollisionGroup> Scan(Dictionary<string, List<string>> guidToPaths)
        {
            return new List<GuidCollisionGroup>();
        }
    }
}
