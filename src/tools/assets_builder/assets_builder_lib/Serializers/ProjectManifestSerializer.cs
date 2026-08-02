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

        // Write startView GUID (16 bytes)
        string startViewStr = string.Empty;
        if (root.TryGetProperty("start_view", out var svProp1) && svProp1.GetString() is string s1)
            startViewStr = s1;
        else if (root.TryGetProperty("startView", out var svProp2) && svProp2.GetString() is string s2)
            startViewStr = s2;

        writer.WriteGuid(startViewStr);

        // Write GameScripts GUIDs array (16 bytes binary each)
        var gameScriptsList = new List<string>();
        if (root.TryGetProperty("game_scripts", out var gsArr) && gsArr.ValueKind == JsonValueKind.Array)
        {
            foreach (var elem in gsArr.EnumerateArray())
            {
                if (elem.GetString() is string gsGuid)
                    gameScriptsList.Add(gsGuid);
            }
        }
        else if (root.TryGetProperty("game_script", out var gsProp) && gsProp.GetString() is string singleScript && !string.IsNullOrEmpty(singleScript))
        {
            gameScriptsList.Add(singleScript);
        }

        writer.Write((uint)gameScriptsList.Count);
        foreach (var gsGuid in gameScriptsList)
        {
            writer.WriteGuid(gsGuid);
        }

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
            writer.WriteGuid(scGuid);
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
            writer.WriteGuid(vGuid);
        }

        writer.Flush();
        return ms.ToArray();
    }
}
