using System;
using System.Collections.Generic;
using System.IO;

namespace editor.Services.Project.Infrastructure.UndoRedo
{
	/// <summary>
	/// Команда создания пары hpp/cpp скрипта из шаблонов и .meta файла с GUID, с поддержкой Undo/Redo.
	/// </summary>
	public class CreateScriptCommand : ICommand
	{
		private readonly string _newRelPath;
		private readonly string _className;
		private readonly string _scriptType;
		private readonly string _templatesDir;
		private readonly string _projectRoot;
		private readonly IFileStorage _storage;
		// GUID фиксируется один раз в конструкторе, а не на каждый Execute() - иначе Redo после Undo
		// создавал бы .meta с новым GUID, как будто это другой ассет.
		private readonly string _guid = Guid.NewGuid().ToString();
		private readonly List<string> _createdPaths = new();

		public CreateScriptCommand(string newRelPath, string className, string scriptType, string templatesDir, string projectRoot, IFileStorage storage)
		{
			_newRelPath = newRelPath;
			_className = className;
			_scriptType = scriptType;
			_templatesDir = templatesDir;
			_projectRoot = projectRoot;
			_storage = storage;
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
				includePath = "Game.h";
				baseClass = "zzz::script::Game";
			}
			else if (_scriptType == "Scene")
			{
				includePath = "Scene.h";
				baseClass = "zzz::script::Scene";
			}

			string dateStr = DateTime.Now.ToString("yyyy-MM-dd");

			string Replace(string content) => content
				.Replace("{ClassName}", _className)
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

			var metaData = ScriptMetaFile.CreateNew(_className);
			metaData.Guid = _guid;
			ScriptMetaFile.Save(_storage, finalMetaPath, metaData);
			_createdPaths.Add(finalMetaPath);
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
		}
	}
}
