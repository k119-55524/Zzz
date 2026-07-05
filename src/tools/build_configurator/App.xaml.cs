using System.IO;
using System.Windows;
using System.Windows.Threading;

namespace BuildConfigurator;

public partial class App : Application
{
    protected override void OnStartup(StartupEventArgs e)
    {
        DispatcherUnhandledException += (_, args) =>
        {
            File.WriteAllText(
                Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "crash.log"),
                args.Exception.ToString());
            args.Handled = true;
            Shutdown(1);
        };
        AppDomain.CurrentDomain.UnhandledException += (_, args) =>
        {
            File.WriteAllText(
                Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "crash.log"),
                args.ExceptionObject.ToString());
        };
        base.OnStartup(e);
    }
}
