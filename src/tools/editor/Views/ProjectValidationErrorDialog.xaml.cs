using System.Collections.Generic;
using System.Windows;

namespace editor.Views
{
    public enum ProjectLoadResolution
    {
        Abort,
        RestoreDefaults
    }

    public class ValidationErrorItem
    {
        public string FilePath { get; set; } = string.Empty;
        public string ErrorMessage { get; set; } = string.Empty;
    }

    public partial class ProjectValidationErrorDialog : Window
    {
        public ProjectLoadResolution Resolution { get; private set; } = ProjectLoadResolution.Abort;

        public ProjectValidationErrorDialog(Window owner, List<ValidationErrorItem> errors)
        {
            Owner = owner;
            InitializeComponent();
            ErrorListView.ItemsSource = errors;
        }

        private void Close_Click(object sender, RoutedEventArgs e)
        {
            Resolution = ProjectLoadResolution.Abort;
            DialogResult = false;
            Close();
        }

        private void Abort_Click(object sender, RoutedEventArgs e)
        {
            Resolution = ProjectLoadResolution.Abort;
            DialogResult = false;
            Close();
        }

        private void RestoreDefaults_Click(object sender, RoutedEventArgs e)
        {
            Resolution = ProjectLoadResolution.RestoreDefaults;
            DialogResult = true;
            Close();
        }
    }
}
