using System.Collections.Generic;
using editor.Models;
using editor.Services.Project.Infrastructure;

namespace editor.Services.Project.FileTypes.Assets
{
	/// <summary>
	/// Данные сцены, хранящиеся в *.zs файле.
	/// </summary>
	public class SceneAssetData
	{
		[EditorVisibility(EditorVisibility.ReadOnly)]
		[EditorDisplayName("Version")]
		public string Version { get; set; } = "1.0";

		[EditorVisibility(EditorVisibility.Editable)]
		[EditorDisplayName("Scene Scripts")]
		[EditorCollection(
			EditorCollectionKind.AssetGuidList,
			AssetType = AssetResourceType.Script,
			IsSortable = true,
			AllowDuplicates = false)]
		public List<string> ScriptGuids { get; set; } = new();
	}
}
