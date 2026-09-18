using System.Collections.Generic;
using editor.Models;
using editor.Services.Project.Infrastructure;

namespace editor.Services.Project.FileTypes.Assets
{
	/// <summary>
	/// Данные представления (View), хранящиеся в *.zview файле.
	/// </summary>
	public class ViewAssetData
	{
		[EditorVisibility(EditorVisibility.ReadOnly)]
		[EditorDisplayName("Version")]
		public string Version { get; set; } = "1.0";

		[EditorVisibility(EditorVisibility.Editable)]
		[EditorDisplayName("Target Scene")]
		[EditorAssetGuid(AssetResourceType.Scene)]
		public string SceneGuid { get; set; } = string.Empty;

		[EditorVisibility(EditorVisibility.Editable)]
		[EditorDisplayName("View Scripts")]
		[EditorCollection(
			EditorCollectionKind.AssetGuidList,
			AssetType = AssetResourceType.Script,
			IsSortable = true,
			AllowDuplicates = false)]
		public List<string> ScriptGuids { get; set; } = new();
	}
}
