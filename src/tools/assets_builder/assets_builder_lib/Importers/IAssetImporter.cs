namespace assets_builder_lib.Importers;

public interface IAssetImporter
{
    bool CanHandle(string filePath);
    string GetMetaFilePath(string filePath);
    string GenerateMetaJson(string filePath, string guid);
}
