namespace editor.Services.Project.Assets
{
	public sealed class ScriptAssetInfo
	{
		public string Guid { get; init; } = string.Empty;
		public string ClassName { get; init; } = string.Empty;
		public string Namespace { get; init; } = string.Empty;
		public string QualifiedName => string.IsNullOrWhiteSpace(Namespace)
			? ClassName
			: $"{Namespace}::{ClassName}";
		public string AssetPath { get; init; } = string.Empty;
		public string HppPath { get; init; } = string.Empty;
		public string CppPath { get; init; } = string.Empty;
		public string MetaPath { get; init; } = string.Empty;
	}
}
