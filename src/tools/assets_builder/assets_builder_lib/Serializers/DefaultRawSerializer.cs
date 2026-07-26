using System.IO;

namespace assets_builder_lib.Serializers;

public class DefaultRawSerializer : IAssetSerializer
{
    public bool CanSerialize(string filePath) => true;

    public byte[] SerializeToBinary(string filePath, string guid)
    {
        return File.ReadAllBytes(filePath);
    }
}
