namespace BuildConfigurator.Models;

public class AppSettings
{
    public string BuildDirectory { get; set; } = string.Empty;
    public string CmakeRootPath { get; set; } = string.Empty;
    public string RebuildCommand { get; set; } = "cmake -B \"{buildDir}\" -S \"{cmakeRoot}\"";
}
