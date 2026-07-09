using System.IO;
using System.Threading;

namespace editor.Services.Project.Infrastructure
{
	/// <summary>
	/// Физическая реализация IFileStorage, использующая стандартный ввод-вывод System.IO.
	/// </summary>
	public class PhysicalFileStorage : IFileStorage
	{
		private const int ReadRetryMaxAttempts = 50;
		private const int ReadRetryDelayMs = 100;

		public bool FileExists(string path)
		{
			return File.Exists(path);
		}

		public bool DirectoryExists(string path)
		{
			return Directory.Exists(path);
		}

		public string ReadAllText(string path)
		{
			// FileSystemWatcher присылает Changed сразу же, как только ОС фиксирует запись, но
			// IDE/антивирус может доли секунды удерживать хендл файла
			for (int attempt = 1; attempt < ReadRetryMaxAttempts; attempt++)
			{
				try
				{
					return File.ReadAllText(path);
				}
				catch (IOException) when (File.Exists(path))
				{
					Thread.Sleep(ReadRetryDelayMs);
				}
			}

			return File.ReadAllText(path);
		}

		public void WriteAllText(string path, string content)
		{
			File.WriteAllText(path, content);
		}

		public void CreateDirectory(string path)
		{
			Directory.CreateDirectory(path);
		}

		public string[] GetFileSystemEntries(string path)
		{
			return Directory.GetFileSystemEntries(path);
		}

		public void DeleteFile(string path)
		{
			if (File.Exists(path))
			{
				File.Delete(path);
			}
		}

		public void DeleteDirectory(string path, bool recursive)
		{
			if (Directory.Exists(path))
			{
				Directory.Delete(path, recursive);
			}
		}

		public void MoveFile(string sourcePath, string destPath)
		{
			File.Move(sourcePath, destPath);
		}

		public void MoveDirectory(string sourcePath, string destPath)
		{
			Directory.Move(sourcePath, destPath);
		}
	}
}
