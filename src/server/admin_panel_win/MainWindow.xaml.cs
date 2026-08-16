using System.Windows;

namespace AdminPanelWin
{
    public partial class MainWindow : Window
    {
        private bool _isConnected = false;

        public MainWindow()
        {
            InitializeComponent();
        }

        private void BtnStart_Click(object sender, RoutedEventArgs e)
        {
            _isConnected = true;
            BtnStart.IsEnabled = false;
            BtnStop.IsEnabled = true;
            TxtStatus.Text = "Status: Connected (localhost:9004)";
            Log("Connecting to Server via RPC/Admin socket on localhost:9004...");
        }

        private void BtnStop_Click(object sender, RoutedEventArgs e)
        {
            if (_isConnected)
            {
                _isConnected = false;
                BtnStart.IsEnabled = true;
                BtnStop.IsEnabled = false;
                TxtStatus.Text = "Status: Disconnected";
                Log("Disconnected from Server.");
            }
        }

        private void Log(string message)
        {
            TxtLog.AppendText($"[{System.DateTime.Now:HH:mm:ss}] {message}\n");
            TxtLog.ScrollToEnd();
        }
    }
}
