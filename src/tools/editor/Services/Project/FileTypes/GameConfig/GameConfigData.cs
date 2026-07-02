using System.Collections.Generic;
using editor.Models;
using editor.Services.Project.Infrastructure;

namespace editor.Services.Project.FileTypes.GameConfig
{
	/// <summary>
	/// Game-wide configuration stored in Configs/game_config.toml.
	/// </summary>
	public class GameConfigData
	{
		[EditorVisibility(EditorVisibility.ReadOnly)]
		public string Version { get; set; } = string.Empty;

		[EditorVisibility(EditorVisibility.Editable)]
		[EditorCollection(EditorCollectionKind.StringList, AllowDuplicates = false)]
		public List<string> Defines { get; set; } = new();

		[EditorVisibility(EditorVisibility.Editable)]
		[EditorOptions(EditorOptionsSource.LogListeners, AllowNone = true)]
		public string LogListener { get; set; } = string.Empty;

		[EditorVisibility(EditorVisibility.Editable)]
		[EditorCollection(
			EditorCollectionKind.AssetGuidList,
			AssetType = AssetResourceType.Script,
			IsSortable = true,
			AllowDuplicates = false)]
		public List<string> GlobalScriptGuids { get; set; } = new();
	}
}
