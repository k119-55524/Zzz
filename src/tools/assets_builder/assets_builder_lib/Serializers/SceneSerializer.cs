using System;
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

        string sceneScript = root.TryGetProperty("script", out var scProp) ? scProp.GetString() ?? "" : "";

        // Write SceneScript GUID (16 bytes binary)
        writer.WriteGuid(sceneScript);

        writer.Flush();
        return ms.ToArray();
    }
}
