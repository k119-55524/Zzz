using System;
using System.Collections.Generic;
using System.IO;
using System.Text.Json;

namespace assets_builder_lib.Serializers;

public class ViewSerializer : IAssetSerializer
{
	public bool CanSerialize(string filePath)
	{
		return filePath.EndsWith(".zv", StringComparison.OrdinalIgnoreCase);
	}

	public byte[] SerializeToBinary(string filePath, string guid)
	{
		using var ms = new MemoryStream();
		using var writer = new BinaryWriter(ms, System.Text.Encoding.UTF8, leaveOpen: true);

		string viewName = Path.GetFileNameWithoutExtension(filePath);
		writer.WriteStringUtf8(viewName);

		string json = File.ReadAllText(filePath);
		using var doc = JsonDocument.Parse(json);
		var root = doc.RootElement;

		uint width = root.GetProperty("width").GetUInt32();
		uint height = root.GetProperty("height").GetUInt32();

		writer.Write(width);
		writer.Write(height);

		string sceneGuid = root.TryGetProperty("scene", out var scProp) ? scProp.GetString() ?? "" : "";
		writer.WriteGuid(sceneGuid);

		// ViewScripts GUIDs array (16 bytes binary each)
		var scriptsList = new List<string>();
		if (root.TryGetProperty("scripts", out var scArr) && scArr.ValueKind == JsonValueKind.Array)
		{
			foreach (var elem in scArr.EnumerateArray())
			{
				if (elem.GetString() is string scGuid)
					scriptsList.Add(scGuid);
			}
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
