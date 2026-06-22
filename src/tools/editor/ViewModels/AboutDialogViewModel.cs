using System;
using System.Windows.Input;

namespace editor.ViewModels
{
    public class AboutDialogViewModel : ViewModelBase
    {
        public AboutDialogViewModel()
        {
            OkCommand = new RelayCommand(() => CloseRequested?.Invoke(this, EventArgs.Empty));
        }

        public string VersionText => $"Версия: {EditorConstants.Version}";

        public string BuildDateText => $"Дата сборки: {EditorConstants.BuildDate}";

        public ICommand OkCommand { get; }

        public event EventHandler? CloseRequested;
    }
}
