using editor.Services.Project.Infrastructure;

namespace editor.Services.Project.FileTypes.Assets
{
	public class SceneAssetParser : IEditorConfigParser
	{
		public Type DataType => typeof(SceneAssetData);

		public static SceneAssetData Deserialize(string toml)
		{
			var data = new SceneAssetData();
			foreach (var (key, value) in TomlLineParser.ParseKeyValueLines(toml))
			{
				if (key == "version")
				{
					data.Version = value.Trim('"');
				}
				else if (key == "script_guids")
				{
					data.ScriptGuids = TomlLineParser.ParseStringArray(value);
				}
			}
			return data;
		}

		public static string Serialize(SceneAssetData data)
		{
			var sb = new System.Text.StringBuilder();
			sb.AppendLine($"version = \"{TomlLineParser.Escape(data.Version)}\"");
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
			return Serialize((SceneAssetData)data);
		}
	}
}
