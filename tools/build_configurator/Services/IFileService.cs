using BuildConfigurator.Models;

namespace BuildConfigurator.Services;

public interface IFileService
{
    AppData LoadData();
    void SaveData(AppData data);
    string GetDataFilePath();
}
