using System;

namespace editor.Services.Project.Assets
{
	public sealed class ScriptAssetIndexChangedEventArgs : EventArgs
	{
		public ScriptAssetIndexChangedEventArgs(bool affectsCompilation)
		{
			AffectsCompilation = affectsCompilation;
		}

		public bool AffectsCompilation { get; }
	}
}
