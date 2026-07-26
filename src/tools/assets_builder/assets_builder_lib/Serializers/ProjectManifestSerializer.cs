using System;
using System.Collections.Generic;
using System.IO;
using System.Text.Json;

namespace assets_builder_lib.Serializers;

public class ProjectManifestSerializer : IAssetSerializer
{
    public bool CanSerialize(string filePath)
    {
        return Path.GetFileName(filePath).Equals(AssetExtensions.ProjectJsonName, StringComparison.OrdinalIgnoreCase);
    }

    public byte[] SerializeToBinary(string filePath, string guid)
    {
        using var ms = new MemoryStream();
        using var writer = new BinaryWriter(ms, System.Text.Encoding.UTF8, leaveOpen: true);

        string json = File.ReadAllText(filePath);
        using var doc = JsonDocument.Parse(json);
        var root = doc.RootElement;

        string gameScript = root.TryGetProperty("game_script", out var gsProp) ? gsProp.GetString() ?? "" : "";

        // Write GameScript GUID (16 bytes binary)
        writer.Write(ParseGuidTo16Bytes(gameScript));

        // Write Scenes GUIDs array (16 bytes binary each)
        var scenesList = new List<string>();
        if (root.TryGetProperty("scenes", out var scenesArr) && scenesArr.ValueKind == JsonValueKind.Array)
        {
            foreach (var elem in scenesArr.EnumerateArray())
            {
                if (elem.GetString() is string scGuid)
                    scenesList.Add(scGuid);
            }
        }

        writer.Write((uint)scenesList.Count);
        foreach (var scGuid in scenesList)
        {
            writer.Write(ParseGuidTo16Bytes(scGuid));
        }

        // Write Views GUIDs array (16 bytes binary each)
        var viewsList = new List<string>();
        if (root.TryGetProperty("views", out var viewsArr) && viewsArr.ValueKind == JsonValueKind.Array)
        {
            foreach (var elem in viewsArr.EnumerateArray())
            {
                if (elem.GetString() is string vGuid)
                    viewsList.Add(vGuid);
            }
        }

        writer.Write((uint)viewsList.Count);
        foreach (var vGuid in viewsList)
        {
            writer.Write(ParseGuidTo16Bytes(vGuid));
        }

        writer.Flush();
        return ms.ToArray();
    }

    private static byte[] ParseGuidTo16Bytes(string guidStr)
    {
        if (Guid.TryParse(guidStr, out var parsed))
            return parsed.ToByteArray();

        return new byte[16];
    }
}
