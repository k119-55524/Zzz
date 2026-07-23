using editor.Services.Project.Infrastructure;

namespace editor.Services.Project.FileTypes.Assets
{
	public class ViewAssetParser : IEditorConfigParser
	{
		public Type DataType => typeof(ViewAssetData);

		public static ViewAssetData Deserialize(string toml)
		{
			var data = new ViewAssetData();
			foreach (var (key, value) in TomlLineParser.ParseKeyValueLines(toml))
			{
				if (key == "version")
				{
					data.Version = value.Trim('"');
				}
				else if (key == "scene_guid")
				{
					data.SceneGuid = value.Trim('"');
				}
				else if (key == "script_guids")
				{
					data.ScriptGuids = TomlLineParser.ParseStringArray(value);
				}
			}
			return data;
		}

		public static string Serialize(ViewAssetData data)
		{
			var sb = new System.Text.StringBuilder();
			sb.AppendLine($"version = \"{TomlLineParser.Escape(data.Version)}\"");
			sb.AppendLine($"scene_guid = \"{TomlLineParser.Escape(data.SceneGuid)}\"");
			if (data.ScriptGuids.Count > 0)
			{
				sb.AppendLine($"script_guids = {TomlLineParser.SerializeStringArray(data.ScriptGuids)}");
			}
			return sb.ToString();
		}

		public object DeserializeForEditor(string content)
		{
			return Deserialize(content);
		}

		public string SerializeFromEditor(object data)
		{
			return Serialize((ViewAssetData)data);
		}
	}
}
