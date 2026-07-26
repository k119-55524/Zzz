using System.IO;

namespace assets_builder_lib.Serializers;

public interface IAssetSerializer
{
    bool CanSerialize(string filePath);
    byte[] SerializeToBinary(string filePath, string guid);
}
