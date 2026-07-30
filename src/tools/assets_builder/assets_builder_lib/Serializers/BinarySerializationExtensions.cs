using System;
using System.IO;
using System.Text;

namespace assets_builder_lib.Serializers;

public static class BinarySerializationExtensions
{
	/// <summary>
	/// Парсит строковое представление GUID в 16-байтовый бинарный массив (Big-Endian network order).
	/// </summary>
	public static byte[] ParseGuidTo16Bytes(string guidStr)
	{
		if (string.IsNullOrWhiteSpace(guidStr))
			return new byte[16];

		string hex = guidStr.Replace("-", "").Trim();
		if (hex.Length != 32)
			return new byte[16];

		byte[] bytes = new byte[16];
		for (int i = 0; i < 16; i++)
		{
			bytes[i] = Convert.ToByte(hex.Substring(i * 2, 2), 16);
		}
		return bytes;
	}

	/// <summary>
	/// Сериализует GUID строку как 16 чистых байт.
	/// </summary>
	public static void WriteGuid(this BinaryWriter writer, string guidStr)
	{
		writer.Write(ParseGuidTo16Bytes(guidStr));
	}

	/// <summary>
	/// Сериализует UTF-8 строку с предварительной записью ее длины uint32.
	/// </summary>
	public static void WriteStringUtf8(this BinaryWriter writer, string str)
	{
		byte[] bytes = Encoding.UTF8.GetBytes(str ?? string.Empty);
		writer.Write((uint)bytes.Length);
		if (bytes.Length > 0)
		{
			writer.Write(bytes);
		}
	}
}
