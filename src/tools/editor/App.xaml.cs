using System.Configuration;
using System.Data;
using System.Windows;
using editor.Services;
using editor.Services.Project;
using editor.Services.Project.Assets;
using editor.Services.Project.Infrastructure;

namespace editor;

/// <summary>
/// Interaction logic for App.xaml
/// </summary>
public partial class App : Application
{
    public static EngineService EngineService { get; } = new EngineService();
    public static ProjectService ProjectService { get; } = new ProjectService();
    public static ProjectFileWatcherService ProjectFileWatcherService { get; } = new ProjectFileWatcherService();
    public static ScriptAssetIndexService ScriptAssetIndexService { get; } = new ScriptAssetIndexService(ProjectService.Storage, ProjectFileWatcherService);
    public static SelectionService SelectionService { get; } = new SelectionService();

    protected override void OnStartup(StartupEventArgs e)
    {
        base.OnStartup(e);
        var globalState = EditorSessionManager.LoadGlobalSession();
        LocalizationManager.Initialize(globalState.Language);
    }
}
