using System.IO;
using System.Windows;
using System.Windows.Controls;

namespace editor.Views
{
    public partial class NewProjectDialog : Window
    {
        public string ProjectName { get; private set; } = string.Empty;
        public string ParentDirectory { get; private set; } = string.Empty;
        public string TargetProjectDirectory => Path.Combine(ParentDirectory, ProjectName);
        public string TargetProjectFile => Path.Combine(TargetProjectDirectory, "project.zzz");

        public NewProjectDialog(Window owner, string initialParentDir)
        {
            Owner = owner;
            InitializeComponent();
            
            if (!string.IsNullOrEmpty(initialParentDir) && System.IO.Directory.Exists(initialParentDir))
            {
                ParentDirectory = initialParentDir;
            }
            else
            {
                ParentDirectory = System.Environment.GetFolderPath(System.Environment.SpecialFolder.MyDocuments);
            }
            ProjectPathTextBox.Text = ParentDirectory;
        }

        private string GetLocString(string key)
        {
            return Application.Current?.TryFindResource(key) as string ?? string.Empty;
        }

        private void Browse_Click(object sender, RoutedEventArgs e)
        {
            var dialog = new Microsoft.Win32.OpenFolderDialog
            {
                Title = GetLocString("Dialog_NewProject_BrowseFolder_Title"),
                InitialDirectory = ParentDirectory
            };

            if (dialog.ShowDialog() == true)
            {
                ParentDirectory = dialog.FolderName;
                ProjectPathTextBox.Text = ParentDirectory;
                ValidateInput(sender, null!);
            }
        }

        private void ValidateInput(object sender, TextChangedEventArgs e)
        {
            string projectName = ProjectNameTextBox.Text.Trim();
            ErrorTextBlock.Visibility = Visibility.Collapsed;
            CreateButton.IsEnabled = false;

            if (string.IsNullOrEmpty(projectName))
            {
                return;
            }

            if (!App.ProjectService.IsValidProjectName(projectName, out string nameError))
            {
                ShowError(nameError);
                return;
            }

            string targetDir = Path.Combine(ParentDirectory, projectName);
            if (Directory.Exists(targetDir) && Directory.GetFileSystemEntries(targetDir).Length > 0)
            {
                string format = GetLocString("Validation_Folder_Exists");
                ShowError(string.Format(format, projectName));
                return;
            }

            CreateButton.IsEnabled = true;
        }

        private void ShowError(string msg)
        {
            ErrorTextBlock.Text = $"⚠️ {msg}";
            ErrorTextBlock.Visibility = Visibility.Visible;
        }

        private void Create_Click(object sender, RoutedEventArgs e)
        {
            ProjectName = ProjectNameTextBox.Text.Trim();
            DialogResult = true;
            Close();
        }

        private void Cancel_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = false;
            Close();
        }
    }
}
