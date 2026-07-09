using System;
using System.Collections.Generic;
using System.IO;

namespace editor.Services.Project.Infrastructure.UndoRedo
{
	/// <summary>
	/// Команда создания пары hpp/cpp скрипта из шаблонов и .meta файла с GUID, с поддержкой Undo/Redo.
	/// </summary>
	public class CreateScriptCommand : IAssetsTreeCommand
	{
		private readonly string _newRelPath;
		private readonly string _className;
		private readonly string _scriptNamespace;
		private readonly string _scriptType;
		private readonly string _templatesDir;
		private readonly string _projectRoot;
		private readonly IFileStorage _storage;
		// GUID фиксируется один раз в конструкторе, а не на каждый Execute() - иначе Redo после Undo
		// создавал бы .meta с новым GUID, как будто это другой ассет.
		private readonly string _guid = Guid.NewGuid().ToString();
		private readonly List<string> _createdPaths = new();

		private readonly Action? _onScriptChanged;

		public CreateScriptCommand(string newRelPath, string className, string scriptType, string scriptNamespace, string templatesDir, string projectRoot, IFileStorage storage, Action? onScriptChanged = null)
		{
			_newRelPath = newRelPath;
			_className = className;
			_scriptNamespace = scriptNamespace?.Trim() ?? string.Empty;
			_scriptType = scriptType;
			_templatesDir = templatesDir;
			_projectRoot = projectRoot;
			_storage = storage;
			_onScriptChanged = onScriptChanged;
		}

		public void Execute()
		{
			_createdPaths.Clear();

			string hppTemplatePath = Path.Combine(_templatesDir, $"{_scriptType}.hpp.template");
			string cppTemplatePath = Path.Combine(_templatesDir, $"{_scriptType}.cpp.template");

			if (!_storage.FileExists(hppTemplatePath) || !_storage.FileExists(cppTemplatePath))
			{
				throw new IOException($"Template files for type '{_scriptType}' not found in: {_templatesDir}");
			}

			string hppContent = _storage.ReadAllText(hppTemplatePath);
			string cppContent = _storage.ReadAllText(cppTemplatePath);

			string includePath = "Script.h";
			string baseClass = "zzz::script::Script";
			if (_scriptType == "Game")
			{
				includePath = "GameScript.h";
				baseClass = "zzz::script::GameScript";
			}
			else if (_scriptType == "Scene")
			{
				includePath = "SceneScript.h";
				baseClass = "zzz::script::SceneScript";
			}

			string dateStr = DateTime.Now.ToString("yyyy-MM-dd");
			string[] namespaceParts = SplitNamespace(_scriptNamespace);
			string namespaceOpenHpp = BuildNamespaceOpenHpp(namespaceParts);
			string namespaceCloseHpp = BuildNamespaceCloseHpp(namespaceParts);
			string namespaceOpenCpp = BuildNamespaceOpenCpp(namespaceParts);
			string namespaceCloseCpp = BuildNamespaceCloseCpp(namespaceParts);
			string namespaceIndent = namespaceParts.Length == 0 ? string.Empty : "\t";

			string Replace(string content) => content
				.Replace("{ClassName}", _className)
				.Replace("{NamespaceOpenHpp}", namespaceOpenHpp)
				.Replace("{NamespaceCloseHpp}", namespaceCloseHpp)
				.Replace("{NamespaceOpenCpp}", namespaceOpenCpp)
				.Replace("{NamespaceCloseCpp}", namespaceCloseCpp)
				.Replace("{NamespaceIndent}", namespaceIndent)
				.Replace("{BaseClass}", baseClass)
				.Replace("{IncludePath}", includePath)
				.Replace("{Date}", dateStr);

			string finalHpp = Replace(hppContent);
			string finalCpp = Replace(cppContent);

			string relativeDir = Path.GetDirectoryName(_newRelPath) ?? "";
			string absoluteDir = Path.Combine(_projectRoot, relativeDir);
			_storage.CreateDirectory(absoluteDir);

			string finalHppPath = Path.Combine(absoluteDir, _className + ".hpp");
			string finalCppPath = Path.Combine(absoluteDir, _className + ".cpp");
			string finalMetaPath = Path.Combine(absoluteDir, _className + ".meta");

			if (_storage.FileExists(finalHppPath) || _storage.FileExists(finalCppPath) || _storage.FileExists(finalMetaPath))
			{
				throw new IOException("Файл с таким именем уже существует.");
			}

			_storage.WriteAllText(finalHppPath, finalHpp);
			_createdPaths.Add(finalHppPath);

			_storage.WriteAllText(finalCppPath, finalCpp);
			_createdPaths.Add(finalCppPath);

			var metaData = ScriptMetaFile.CreateNew(_className, _scriptNamespace);
			metaData.Guid = _guid;
			ScriptMetaFile.Save(_storage, finalMetaPath, metaData);
			_createdPaths.Add(finalMetaPath);

			_onScriptChanged?.Invoke();
		}

		private static string[] SplitNamespace(string scriptNamespace)
		{
			if (string.IsNullOrWhiteSpace(scriptNamespace))
			{
				return Array.Empty<string>();
			}

			return scriptNamespace
				.Split(new[] { "::" }, StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries);
		}

		private static string BuildNamespaceOpenHpp(string[] namespaceParts)
		{
			if (namespaceParts.Length == 0)
			{
				return string.Empty;
			}

			var sb = new System.Text.StringBuilder();
			string indent = string.Empty;
			foreach (string part in namespaceParts)
			{
				sb.Append(indent).Append("namespace ").Append(part).Append("\r\n");
				sb.Append(indent).Append("{\r\n");
				indent += "\t";
			}
			return sb.ToString();
		}

		private static string BuildNamespaceCloseHpp(string[] namespaceParts)
		{
			if (namespaceParts.Length == 0)
			{
				return string.Empty;
			}

			var sb = new System.Text.StringBuilder();
			for (int i = namespaceParts.Length - 1; i >= 0; i--)
			{
				sb.Append(new string('\t', i)).Append("}\r\n");
			}
			return sb.ToString();
		}

		private static string BuildNamespaceOpenCpp(string[] namespaceParts)
		{
			if (namespaceParts.Length == 0)
			{
				return string.Empty;
			}

			return "namespace " + string.Join(" { namespace ", namespaceParts) + " {\r\n\r\n";
		}

		private static string BuildNamespaceCloseCpp(string[] namespaceParts)
		{
			if (namespaceParts.Length == 0)
			{
				return string.Empty;
			}

			return new string('}', namespaceParts.Length) + "\r\n";
		}

		public void Undo()
		{
			for (int i = _createdPaths.Count - 1; i >= 0; i--)
			{
				string path = _createdPaths[i];
				if (_storage.FileExists(path))
				{
					_storage.DeleteFile(path);
				}
			}
			_createdPaths.Clear();

			_onScriptChanged?.Invoke();
		}
	}
}
