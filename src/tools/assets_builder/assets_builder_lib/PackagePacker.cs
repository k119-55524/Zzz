
using System;
using System.IO;
using System.Linq;
using System.Text;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using assets_builder_lib.Serializers;

namespace assets_builder_lib;

public static class PackageAssetTypes
{
	public static uint ProjectManifest => NativeMethods.GetAssetTypeProjectManifest();
	public static uint Scene => NativeMethods.GetAssetTypeScene();
	public static uint View => NativeMethods.GetAssetTypeView();
	public static uint Prefab => NativeMethods.GetAssetTypePrefab();
	public static uint BinaryAsset => NativeMethods.GetAssetTypeBinaryAsset();
}

public static class PackagePacker
{
	private class PendingItem
	{
		public string Name { get; set; } = string.Empty;
		public string Guid { get; set; } = string.Empty;
		public uint Type { get; set; }
		public string FilePath { get; set; } = string.Empty;
	}

	public static bool PackProject(string sourceDir, string destinationDir, Action<string>? logCallback = null)
	{
		try
		{
			string packageFileName = AssetExtensions.GamePackageBinaryName;
			string outPath = Path.Combine(destinationDir, packageFileName);
			string? parentDir = Path.GetDirectoryName(outPath);
			if (!string.IsNullOrEmpty(parentDir))
			{
				Directory.CreateDirectory(parentDir);
			}

			var pendingAssets = new List<PendingItem>();

			// Сканирование сцен (*.zs), вьюх (*.zv) и префабов (*.zp)
			if (Directory.Exists(sourceDir))
			{
				foreach (var file in Directory.GetFiles(sourceDir, "*.*", SearchOption.AllDirectories))
				{
					string fileName = Path.GetFileName(file);
					string ext = Path.GetExtension(file).ToLowerInvariant();
					uint typeVal = 0;
					string nameVal = Path.GetFileNameWithoutExtension(file);

					if (fileName.Equals(AssetExtensions.ProjectJsonName, StringComparison.OrdinalIgnoreCase))
					{
						typeVal = PackageAssetTypes.ProjectManifest;
						nameVal = "ProjectManifest";
					}
					else if (ext == ".zs")
						typeVal = PackageAssetTypes.Scene;
					else if (ext == ".zv")
						typeVal = PackageAssetTypes.View;
					else if (ext == ".zp")
						typeVal = PackageAssetTypes.Prefab;
					else
						continue;

					string metaPath = file + ".meta";
					string guid = ExtractGuidFromMeta(metaPath);

					if (!string.IsNullOrEmpty(guid) && guid != "unknown")
					{
						pendingAssets.Add(new PendingItem
						{
							Name = nameVal,
							Guid = guid,
							Type = typeVal,
							FilePath = file
						});
					}
				}
			}

			// 3. Бинарная сериализация ассетов через цепочку IAssetSerializer
			var serializers = new List<IAssetSerializer>
			{
				new ProjectManifestSerializer(),
				new SceneSerializer(),
				new ViewSerializer(),
				new DefaultRawSerializer()
			};

			byte[] magic = GetMagicBytes();
			uint major = NativeMethods.GetGamePackageMajorVersion();
			uint minor = NativeMethods.GetGamePackageMinorVersion();
			uint patch = NativeMethods.GetGamePackagePatchVersion();

			var validPayloadsData = new List<(PendingItem Item, byte[] Data)>();
			foreach (var item in pendingAssets)
			{
				var serializer = serializers.FirstOrDefault(s => s.CanSerialize(item.FilePath)) ?? serializers.Last();
				byte[] binaryData = serializer.SerializeToBinary(item.FilePath, item.Guid);
				validPayloadsData.Add((item, binaryData));
			}

			// Вычисляем точно размер заголовка и оглавления PackageEntry:
			// PackageHeader: 3 (magic) + 4 (major) + 4 (minor) + 4 (patch) + 4 (count) = 19 байт
			long headerSize = 3 + 4 + 4 + 4 + 4;
			long indexTableSize = 0;

			foreach (var payload in validPayloadsData)
			{
				byte[] nameBytes = Encoding.UTF8.GetBytes(payload.Item.Name);
				// PackageEntry: 4 (length of name) + nameBytes.Length + 16 (guid) + 4 (type) + 8 (offset) + 8 (size)
				indexTableSize += 4 + nameBytes.Length + 16 + 4 + 8 + 8;
			}

			long currentOffset = headerSize + indexTableSize;
			var validPayloads = new List<(PendingItem Item, byte[] Data, long Offset)>();

			foreach (var payload in validPayloadsData)
			{
				validPayloads.Add((payload.Item, payload.Data, currentOffset));
				currentOffset += payload.Data.Length;
			}

			using var fs = new FileStream(outPath, FileMode.Create, FileAccess.Write, FileShare.None);
			using var writer = new BinaryWriter(fs, Encoding.UTF8, leaveOpen: false);

			writer.Write(magic, 0, 3);
			writer.Write(major);
			writer.Write(minor);
			writer.Write(patch);
			writer.Write((uint)validPayloads.Count);

			foreach (var payload in validPayloads)
			{
				byte[] nameBytes = Encoding.UTF8.GetBytes(payload.Item.Name);
				writer.Write((uint)nameBytes.Length);
				writer.Write(nameBytes);
				writer.WriteGuid(payload.Item.Guid);
				writer.Write(payload.Item.Type);
				writer.Write((ulong)payload.Offset);
				writer.Write((ulong)payload.Data.Length);
			}

			// Запись чистых бинарных блоков данных ассетов
			foreach (var payload in validPayloads)
			{
				writer.Write(payload.Data);
			}

			logCallback?.Invoke($"Генерация чистого бинарного пакета '{packageFileName}' в 'assets/' завершена (Записей: {validPayloads.Count}).");
			return true;
		}
		catch (Exception ex)
		{
			logCallback?.Invoke($"Ошибка бинарной сериализации {AssetExtensions.GamePackageBinaryName}: {ex.Message}");
			return false;
		}
	}

	private static byte[] GetMagicBytes()
	{
		IntPtr ptr = NativeMethods.GetGamePackageMagicBytes();
		byte[] magic = new byte[3];
		Marshal.Copy(ptr, magic, 0, 3);
		return magic;
	}

	private static string ExtractGuidFromMeta(string metaPath)
	{
		if (!File.Exists(metaPath))
			return "unknown";

		try
		{
			string json = File.ReadAllText(metaPath);
			int idx = json.IndexOf("\"guid\"");
			if (idx != -1)
			{
				int startQuote = json.IndexOf('"', idx + 6);
				if (startQuote != -1)
				{
					int endQuote = json.IndexOf('"', startQuote + 1);
					if (endQuote != -1)
					{
						return json.Substring(startQuote + 1, endQuote - startQuote - 1);
					}
				}
			}
		}
		catch
		{
			// Ignore
		}

		return "unknown";
	}
}
