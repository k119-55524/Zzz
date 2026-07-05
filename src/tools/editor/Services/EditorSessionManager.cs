
using System.IO;
using System.Text;

namespace editor.Services
{
	public class GlobalSessionState
	{
		public string LastOpenProjectPath { get; set; } = string.Empty;
		public string[] RecentProjects { get; set; } = new string[0];
		public string LastCreatedProjectParentDir { get; set; } = string.Empty;
		public string Language { get; set; } = string.Empty;
	}

	public class LayoutSessionState
	{
		public double Width { get; set; } = 1280;
		public double Height { get; set; } = 720;
		public double Left { get; set; } = 100;
		public double Top { get; set; } = 100;
		public bool IsMaximized { get; set; } = false;
		public string LayoutXml { get; set; } = string.Empty;
		public bool IsDirty { get; set; } = false;
	}

	public class EditorSessionManager
	{
		private const string Signature = "ZZZ_ED";

		// Возвращает путь к глобальному файлу состояния в AppData
		public static string GetGlobalFilePath()
		{
			// Берем только мажорную/минорную версию без сборки
			string versionStr = "1.0.0";
			string appData = Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData);
			string folder = Path.Combine(appData, "ZzzEditor", versionStr);
			if (!Directory.Exists(folder))
			{
				Directory.CreateDirectory(folder);
			}
			return Path.Combine(folder, "global_session.dat");
		}

		// Возвращает путь к локальному файлу макета рядом с EXE
		public static string GetLayoutFilePath()
		{
			string exeDir = AppDomain.CurrentDomain.BaseDirectory;
			return Path.Combine(exeDir, "layout_session.dat");
		}

		public static GlobalSessionState LoadGlobalSession()
		{
			string path = GetGlobalFilePath();
			if (!File.Exists(path))
			{
				return GetDefaultGlobalSession();
			}

			try
			{
				using (var fs = new FileStream(path, FileMode.Open, FileAccess.Read))
				using (var reader = new BinaryReader(fs, Encoding.UTF8))
				{
					string sig = reader.ReadString();
					int verCode = reader.ReadInt32();
					if (sig != Signature || verCode != EditorConstants.VersionCode)
					{
						return GetDefaultGlobalSession();
					}

					var state = new GlobalSessionState();
					state.LastOpenProjectPath = reader.ReadString();

					int count = reader.ReadInt32();
					state.RecentProjects = new string[count];
					for (int i = 0; i < count; i++)
					{
						state.RecentProjects[i] = reader.ReadString();
					}

					if (fs.Position < fs.Length)
					{
						state.LastCreatedProjectParentDir = reader.ReadString();
					}
					if (fs.Position < fs.Length)
					{
						state.Language = reader.ReadString();
					}

					return state;
				}
			}
			catch
			{
				return GetDefaultGlobalSession();
			}
		}

		public static void SaveGlobalSession(GlobalSessionState state)
		{
			string path = GetGlobalFilePath();
			try
			{
				using (var fs = new FileStream(path, FileMode.Create, FileAccess.Write))
				using (var writer = new BinaryWriter(fs, Encoding.UTF8))
				{
					writer.Write(Signature);
					writer.Write(EditorConstants.VersionCode);
					writer.Write(state.LastOpenProjectPath ?? string.Empty);

					int count = state.RecentProjects?.Length ?? 0;
					writer.Write(count);
					for (int i = 0; i < count; i++)
					{
						writer.Write(state.RecentProjects![i] ?? string.Empty);
					}

					writer.Write(state.LastCreatedProjectParentDir ?? string.Empty);
					writer.Write(state.Language ?? string.Empty);
				}
			}
			catch
			{
				// Игнорируем ошибки записи
			}
		}

		public static LayoutSessionState LoadLayoutSession()
		{
			string path = GetLayoutFilePath();
			if (!File.Exists(path))
			{
				return GetDefaultLayoutSession();
			}

			try
			{
				using (var fs = new FileStream(path, FileMode.Open, FileAccess.Read))
				using (var reader = new BinaryReader(fs, Encoding.UTF8))
				{
					string sig = reader.ReadString();
					int verCode = reader.ReadInt32();
					if (sig != Signature || verCode != EditorConstants.VersionCode)
					{
						return GetDefaultLayoutSession();
					}

					var state = new LayoutSessionState();
					state.Width = reader.ReadDouble();
					state.Height = reader.ReadDouble();
					state.Left = reader.ReadDouble();
					state.Top = reader.ReadDouble();
					state.IsMaximized = reader.ReadBoolean();
					state.LayoutXml = reader.ReadString();
					state.IsDirty = reader.ReadBoolean();

					return state;
				}
			}
			catch
			{
				return GetDefaultLayoutSession();
			}
		}

		public static void SaveLayoutSession(LayoutSessionState state)
		{
			string path = GetLayoutFilePath();
			try
			{
				using (var fs = new FileStream(path, FileMode.Create, FileAccess.Write))
				using (var writer = new BinaryWriter(fs, Encoding.UTF8))
				{
					writer.Write(Signature);
					writer.Write(EditorConstants.VersionCode);
					writer.Write(state.Width);
					writer.Write(state.Height);
					writer.Write(state.Left);
					writer.Write(state.Top);
					writer.Write(state.IsMaximized);
					writer.Write(state.LayoutXml ?? string.Empty);
					writer.Write(state.IsDirty);
				}
			}
			catch
			{
				// Игнорируем ошибки записи
			}
		}

		private static GlobalSessionState GetDefaultGlobalSession()
		{
			return new GlobalSessionState();
		}

		private static LayoutSessionState GetDefaultLayoutSession()
		{
			return new LayoutSessionState();
		}
	}
}
