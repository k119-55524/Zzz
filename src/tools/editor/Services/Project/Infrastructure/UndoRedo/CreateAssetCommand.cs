using System;
using System.IO;

namespace editor.Services.Project.Infrastructure.UndoRedo
{
	public class CreateAssetCommand : IAssetsTreeCommand
	{
		private readonly string _newRelPath;
		private readonly string _projectRoot;
		private readonly IFileStorage _storage;
		private readonly string _extension;
		private readonly string _guid = Guid.NewGuid().ToString();

		public CreateAssetCommand(string newRelPath, string projectRoot, string extension, IFileStorage storage)
		{
			_newRelPath = newRelPath;
			_projectRoot = projectRoot;
			_extension = extension;
			_storage = storage;
		}

		public void Execute()
		{
			string fullPath = Path.Combine(_projectRoot, _newRelPath.Replace('/', Path.DirectorySeparatorChar));
			string dir = Path.GetDirectoryName(fullPath)!;

			if (!_storage.DirectoryExists(dir))
			{
				_storage.CreateDirectory(dir);
			}

			var factory = editor.Services.Project.Infrastructure.Factories.AssetFactoryRegistry.GetFactory(_extension);
			string baseName = Path.GetFileNameWithoutExtension(fullPath);
			factory.CreateAsset(fullPath, baseName, _guid, _storage);
		}

		public void Undo()
		{
			string fullPath = Path.Combine(_projectRoot, _newRelPath.Replace('/', Path.DirectorySeparatorChar));
			if (_storage.FileExists(fullPath))
			{
				_storage.DeleteFile(fullPath);
			}

			string metaPath = fullPath + ".meta";
			if (_storage.FileExists(metaPath))
			{
				_storage.DeleteFile(metaPath);
			}
		}

		public void Redo()
		{
			Execute();
		}
	}
}
