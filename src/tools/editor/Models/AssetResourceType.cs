namespace editor.Models
{
	// Типы ресурсов ассетов. Пока чисто внутренний классификатор для фильтра в Project/Assets -
	// привязки конкретных файлов к типу (по расширению и т.п.) ещё нет, это будущая задача.
	public enum AssetResourceType
	{
		Texture,
		Model,
		Audio,
		Scene,
		View,
		Script,
		Shader,
		Material
	}

	public static class AssetResourceTypeRules
	{
		public const string ExtScene = ".zscene";
		public const string ExtView = ".zview";
		public const string ExtMaterial = ".zmaterial";
		public const string ExtShader = ".zshaders";
		public const string ExtPrefab = ".zprefab";
		public const string ExtScriptCpp = ".cpp";
		public const string ExtScriptHpp = ".hpp";

		// Условное имя папки первого уровня в Assets/, по которому фильтр сейчас сопоставляет
		// тип с деревом (пока нет классификации по файлам - см. AssetResourceType).
		public static string GetFolderName(AssetResourceType type)
		{
			return type switch
			{
				AssetResourceType.Texture => "Textures",
				AssetResourceType.Model => "Models",
				AssetResourceType.Audio => "Audio",
				AssetResourceType.Scene => "Scenes",
				AssetResourceType.View => "Views",
				AssetResourceType.Script => "Scripts",
				AssetResourceType.Shader => "Shaders",
				AssetResourceType.Material => "Materials",
				_ => throw new System.ArgumentOutOfRangeException(nameof(type), type, null)
			};
		}

		public static string GetTitleKey(AssetResourceType type)
		{
			return type switch
			{
				AssetResourceType.Texture => "ResourceType_Texture",
				AssetResourceType.Model => "ResourceType_Model",
				AssetResourceType.Audio => "ResourceType_Audio",
				AssetResourceType.Scene => "ResourceType_Scene",
				AssetResourceType.View => "ResourceType_View",
				AssetResourceType.Script => "ResourceType_Script",
				AssetResourceType.Shader => "ResourceType_Shader",
				AssetResourceType.Material => "ResourceType_Material",
				_ => throw new System.ArgumentOutOfRangeException(nameof(type), type, null)
			};
		}

		public static string[] GetExtensions(AssetResourceType type)
		{
			return type switch
			{
				AssetResourceType.Scene => new[] { ExtScene },
				AssetResourceType.View => new[] { ExtView },
				AssetResourceType.Script => new[] { ExtScriptCpp, ExtScriptHpp },
				AssetResourceType.Texture => new[] { ".png", ".jpg", ".jpeg", ".tga", ".bmp" },
				AssetResourceType.Model => new[] { ".fbx", ".obj", ".gltf", ".glb" },
				AssetResourceType.Audio => new[] { ".wav", ".mp3", ".ogg" },
				AssetResourceType.Shader => new[] { ExtShader, ".hlsl", ".glsl", ".shader" },
				AssetResourceType.Material => new[] { ExtMaterial, ".mat" },
				_ => System.Array.Empty<string>()
			};
		}
	}
}
