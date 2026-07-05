
using System.Windows;
using System.Windows.Input;
using System.ComponentModel;
using RemoteLogViewer.Models;
using System.Collections.ObjectModel;

namespace RemoteLogViewer;

public partial class MainWindow : Window, INotifyPropertyChanged
{
	private AppSettings _settings;
	public ObservableCollection<TabViewModel> Tabs { get; set; } = new();

	private TabViewModel? _selectedTab;
	public TabViewModel? SelectedTab
	{
		get => _selectedTab;
		set
		{
			if (_selectedTab != value)
			{
				_selectedTab = value;
				PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(SelectedTab)));
			}
		}
	}

	public event PropertyChangedEventHandler? PropertyChanged;

	public ICommand CloseTabCommand { get; }
	public ICommand StartCommand { get; }
	public ICommand StopCommand { get; }
	public ICommand ClearCommand { get; }
	public ICommand CopyCommand { get; }
	public ICommand ExpandAllCommand { get; }
	public ICommand CollapseAllCommand { get; }

	public MainWindow()
	{
		InitializeComponent();

		_settings = SettingsManager.Load();
		Width = _settings.WindowWidth;
		Height = _settings.WindowHeight;
		Top = _settings.WindowTop;
		Left = _settings.WindowLeft;

		foreach (var tabSettings in _settings.SavedTabs)
		{
			Tabs.Add(new TabViewModel(tabSettings));
		}

		CloseTabCommand = new RelayCommand<TabViewModel>(tab =>
		{
			tab.Dispose();
			Tabs.Remove(tab);
		});

		StartCommand = new RelayCommand<TabViewModel>(tab => tab.Start());
		StopCommand = new RelayCommand<TabViewModel>(tab => tab.Stop());
		ClearCommand = new RelayCommand<TabViewModel>(tab => tab.Clear());
		CopyCommand = new RelayCommand<TabViewModel>(tab => tab.CopyAll());
		ExpandAllCommand = new RelayCommand<TabViewModel>(tab => tab.ExpandAll());
		CollapseAllCommand = new RelayCommand<TabViewModel>(tab => tab.CollapseAll());

		DataContext = this;

		if (Tabs.Count > 0)
		{
			int idx = (_settings.SelectedTabIndex >= 0 && _settings.SelectedTabIndex < Tabs.Count)
				? _settings.SelectedTabIndex
				: 0;
			SelectedTab = Tabs[idx];
		}
	}

	private void BtnNewTab_Click(object sender, RoutedEventArgs e)
	{
		var win = new AddTabWindow(_settings.SavedAddresses) { Owner = this };
		if (win.ShowDialog() == true && win.SelectedAddress != null)
		{
			var tabSettings = new TabSettings { Address = win.SelectedAddress };
			var newTab = new TabViewModel(tabSettings);
			Tabs.Add(newTab);
			SelectedTab = newTab;
		}
	}

	private void BtnManageAddresses_Click(object sender, RoutedEventArgs e)
	{
		var win = new AddressManagerWindow(_settings) { Owner = this };
		win.ShowDialog();
	}

	private void Window_Closing(object sender, CancelEventArgs e)
	{
		_settings.WindowWidth = Width;
		_settings.WindowHeight = Height;
		_settings.WindowTop = Top;
		_settings.WindowLeft = Left;
		_settings.SelectedTabIndex = MainTabControl.SelectedIndex;

		_settings.SavedTabs.Clear();
		foreach (var tab in Tabs)
		{
			_settings.SavedTabs.Add(tab.Settings);
			tab.Dispose();
		}

		SettingsManager.Save(_settings);
	}

	private void Filter_Changed(object sender, RoutedEventArgs e)
	{
		if (MainTabControl.SelectedItem is TabViewModel tab)
		{
			tab.ApplyFilter();
		}
	}

	private void DataGridRow_PreviewMouseLeftButtonUp(object sender, MouseButtonEventArgs e)
	{
		if (sender is System.Windows.Controls.DataGridRow row && row.DataContext is LogEntry entry)
		{
			var depObj = e.OriginalSource as DependencyObject;
			while (depObj != null && depObj != row)
			{
				if (depObj is System.Windows.Controls.Primitives.DataGridDetailsPresenter)
					return; // Clicked inside details, ignore
				depObj = System.Windows.Media.VisualTreeHelper.GetParent(depObj);
			}
			entry.IsExpanded = !entry.IsExpanded;
		}
	}
}

public class RelayCommand<T> : ICommand
{
	private readonly Action<T> _execute;
	public RelayCommand(Action<T> execute) => _execute = execute;
	public bool CanExecute(object? parameter) => true;
	public void Execute(object? parameter) { if (parameter is T t) _execute(t); }
	public event EventHandler? CanExecuteChanged { add { } remove { } }
}