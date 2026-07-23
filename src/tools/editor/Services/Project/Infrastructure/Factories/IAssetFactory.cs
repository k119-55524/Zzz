using editor.Services.Project.Infrastructure;

namespace editor.Services.Project.Infrastructure.Factories
{
    public interface IAssetFactory
    {
        string Extension { get; }
        
        /// <summary>
        /// Создаёт основной файл ассета и соответствующий ему .meta файл.
        /// </summary>
        void CreateAsset(string fullPath, string baseName, string guid, IFileStorage storage);
    }
}
