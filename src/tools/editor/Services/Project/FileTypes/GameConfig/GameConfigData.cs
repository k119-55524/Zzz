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
		[EditorDisplayName("Version")]
		public string Version { get; set; } = string.Empty;

		[EditorVisibility(EditorVisibility.Editable)]
		[EditorDisplayName("Defines")]
		[EditorCollection(EditorCollectionKind.StringList, AllowDuplicates = false, IsSortable = true, ItemValidation = EditorCollectionItemValidation.CppDefine)]
		public List<string> Defines { get; set; } = new();

		[EditorVisibility(EditorVisibility.Editable)]
		[EditorDisplayName("Log listener")]
		[EditorOptions(EditorOptionsSource.LogListeners, AllowNone = true)]
		public string LogListener { get; set; } = string.Empty;

		[EditorVisibility(EditorVisibility.Editable)]
		[EditorDisplayName("Global scripts")]
		[EditorCollection(
			EditorCollectionKind.AssetGuidList,
			AssetType = AssetResourceType.Script,
			IsSortable = true,
			AllowDuplicates = false)]
		public List<string> GlobalScriptGuids { get; set; } = new();
	}
}
