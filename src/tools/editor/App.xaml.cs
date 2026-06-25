using System.Configuration;
using System.Data;
using System.Windows;
using editor.Services;
using editor.Services.Project;

namespace editor;

/// <summary>
/// Interaction logic for App.xaml
/// </summary>
public partial class App : Application
{
    public static EngineService EngineService { get; } = new EngineService();
    public static ProjectService ProjectService { get; } = new ProjectService();
}