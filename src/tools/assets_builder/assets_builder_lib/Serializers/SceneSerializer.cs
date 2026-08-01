using System;
using System.Collections.Generic;
using System.IO;
using System.Text.Json;

namespace assets_builder_lib.Serializers;

public class SceneSerializer : IAssetSerializer
{
    public bool CanSerialize(string filePath)
    {
        return filePath.EndsWith(".zs", StringComparison.OrdinalIgnoreCase);
    }

    public byte[] SerializeToBinary(string filePath, string guid)
    {
        using var ms = new MemoryStream();
        using var writer = new BinaryWriter(ms, System.Text.Encoding.UTF8, leaveOpen: true);

        string json = File.ReadAllText(filePath);
        using var doc = JsonDocument.Parse(json);
        var root = doc.RootElement;

        string sceneName = Path.GetFileNameWithoutExtension(filePath);
        writer.WriteStringUtf8(sceneName);

        var scriptsList = new List<string>();
        if (root.TryGetProperty("scripts", out var scArr) && scArr.ValueKind == JsonValueKind.Array)
        {
            foreach (var elem in scArr.EnumerateArray())
            {
                if (elem.GetString() is string scGuid)
                    scriptsList.Add(scGuid);
            }
        }
        else if (root.TryGetProperty("script", out var scProp) && scProp.GetString() is string singleScript && !string.IsNullOrEmpty(singleScript))
        {
            scriptsList.Add(singleScript);
        }

        writer.Write((uint)scriptsList.Count);
        foreach (var scGuid in scriptsList)
        {
            writer.WriteGuid(scGuid);
        }

        writer.Flush();
        return ms.ToArray();
    }
}
