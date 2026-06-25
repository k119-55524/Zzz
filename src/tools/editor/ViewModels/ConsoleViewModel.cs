using System;
using System.Collections.Concurrent;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.IO;
using System.Text.Json;
using System.Windows;
using System.Windows.Data;
using System.Windows.Input;
using System.Windows.Threading;
using editor.Models;
using editor.Services;
using Microsoft.Win32;

namespace editor.ViewModels
{
	public class SourceFilter : ViewModelBase
	{
		private bool _isEnabled = true;
		private bool _showMessage = true;
		private bool _showWarning = true;
		private bool _showError = true;
		private bool _showException = true;
		private bool _showCritical = true;
		private bool _showFatal = true;

		public bool IsEnabled { get => _isEnabled; set { if (SetField(ref _isEnabled, value)) OnFilterChanged(); } }
		public bool ShowMessage { get => _showMessage; set { if (SetField(ref _showMessage, value)) OnFilterChanged(); } }
		public bool ShowWarning { get => _showWarning; set { if (SetField(ref _showWarning, value)) OnFilterChanged(); } }
		public bool ShowError { get => _showError; set { if (SetField(ref _showError, value)) OnFilterChanged(); } }
		public bool ShowException { get => _showException; set { if (SetField(ref _showException, value)) OnFilterChanged(); } }
		public bool ShowCritical { get => _showCritical; set { if (SetField(ref _showCritical, value)) OnFilterChanged(); } }
		public bool ShowFatal { get => _showFatal; set { if (SetField(ref _showFatal, value)) OnFilterChanged(); } }

		public event Action? FilterChanged;
		private void OnFilterChanged() => FilterChanged?.Invoke();

		public bool IsAllowed(LogLevel level)
		{
			return level switch
			{
				LogLevel.Message => ShowMessage,
				LogLevel.Warning => ShowWarning,
				LogLevel.Error => ShowError,
				LogLevel.Exception => ShowException,
				LogLevel.Critical => ShowCritical,
				LogLevel.Fatal => ShowFatal,
				_ => true
			};
		}
	}

	public class ConsoleViewModel : PaneViewModel
	{
		private const int MaxLogCount = 1000;
		private readonly ConcurrentQueue<LogMessage> _incomingQueue = new();
		private readonly DispatcherTimer _updateTimer;
		private string _searchText = string.Empty;

		public ObservableCollection<LogEntryViewModel> Logs { get; } = new();
		public ICollectionView LogsView { get; }

		public SourceFilter EditorFilter { get; } = new();
		public SourceFilter EngineFilter { get; } = new();
		public SourceFilter ScriptsFilter { get; } = new();

		public string SearchText
		{
			get => _searchText;
			set
			{
				if (SetField(ref _searchText, value))
				{
					LogsView.Refresh();
				}
			}
		}

		public ICommand ClearCommand { get; }
		public ICommand ExpandAllCommand { get; }
		public ICommand CollapseAllCommand { get; }
		public ICommand ExportAllCommand { get; }
		public ICommand ExportFilteredCommand { get; }

		public ConsoleViewModel() : base(WidgetType.Console)
		{
			LogsView = CollectionViewSource.GetDefaultView(Logs);
			LogsView.Filter = FilterLog;

			EditorFilter.FilterChanged += OnFiltersChanged;
			EngineFilter.FilterChanged += OnFiltersChanged;
			ScriptsFilter.FilterChanged += OnFiltersChanged;

			ClearCommand = new RelayCommand(Clear);
			ExpandAllCommand = new RelayCommand(ExpandAll);
			CollapseAllCommand = new RelayCommand(CollapseAll);
			ExportAllCommand = new RelayCommand(ExportAll);
			ExportFilteredCommand = new RelayCommand(ExportFiltered);

			EditorLogger.LogReceived += OnLogReceived;

			_updateTimer = new DispatcherTimer(DispatcherPriority.Background)
			{
				Interval = TimeSpan.FromMilliseconds(50)
			};
			_updateTimer.Tick += ProcessIncomingLogs;
			_updateTimer.Start();
		}

		private void OnLogReceived(LogMessage msg)
		{
			_incomingQueue.Enqueue(msg);
		}

		private void ProcessIncomingLogs(object? sender, EventArgs e)
		{
			bool addedAny = false;
			while (_incomingQueue.TryDequeue(out var msg))
			{
				var vm = new LogEntryViewModel(msg);
				AddLogSorted(vm);
				addedAny = true;
			}

			if (addedAny)
			{
				while (Logs.Count > MaxLogCount)
				{
					Logs.RemoveAt(0);
				}
			}
		}

		private void AddLogSorted(LogEntryViewModel newLog)
		{
			if (Logs.Count == 0)
			{
				Logs.Add(newLog);
				return;
			}

			if (newLog.Timestamp >= Logs[Logs.Count - 1].Timestamp)
			{
				Logs.Add(newLog);
				return;
			}

			int i = Logs.Count - 1;
			while (i >= 0 && Logs[i].Timestamp > newLog.Timestamp)
			{
				i--;
			}
			Logs.Insert(i + 1, newLog);
		}

		private bool FilterLog(object obj)
		{
			if (obj is not LogEntryViewModel entry) return false;

			SourceFilter filter = entry.Source switch
			{
				LogSource.Editor => EditorFilter,
				LogSource.Engine => EngineFilter,
				LogSource.Scripts => ScriptsFilter,
				_ => null!
			};

			if (filter != null)
			{
				if (!filter.IsEnabled)
					return false;
				if (!filter.IsAllowed(entry.Level))
					return false;
			}

			if (!string.IsNullOrEmpty(SearchText))
			{
				string search = SearchText.ToLower();
				bool matchesText = entry.Text.ToLower().Contains(search) ||
								   entry.File.ToLower().Contains(search) ||
								   entry.Function.ToLower().Contains(search);
				if (!matchesText)
					return false;
			}

			return true;
		}

		private void OnFiltersChanged()
		{
			LogsView.Refresh();
		}

		private void Clear()
		{
			string title = Application.Current.TryFindResource("Console_Clear_Confirm_Title") as string ?? "Clear Logs";
			string message = Application.Current.TryFindResource("Console_Clear_Confirm_Message") as string ?? "Are you sure you want to clear all logs?";
			if (MessageBox.Show(message, title, MessageBoxButton.YesNo, MessageBoxImage.Question) == MessageBoxResult.Yes)
			{
				Logs.Clear();
			}
		}

		private void ExpandAll()
		{
			foreach (var item in Logs)
			{
				item.IsExpanded = true;
			}
		}

		private void CollapseAll()
		{
			foreach (var item in Logs)
			{
				item.IsExpanded = false;
			}
		}

		private void ExportAll()
		{
			ExportLogs(Logs);
		}

		private void ExportFiltered()
		{
			var filteredList = new System.Collections.Generic.List<LogEntryViewModel>();
			foreach (var item in LogsView)
			{
				if (item is LogEntryViewModel entry)
				{
					filteredList.Add(entry);
				}
			}
			ExportLogs(filteredList);
		}

		private void ExportLogs(System.Collections.IEnumerable logList)
		{
			var saveFileDialog = new SaveFileDialog
			{
				Filter = "JSON files (*.json)|*.json|All files (*.*)|*.*",
				DefaultExt = "json",
				FileName = $"logs_{DateTime.Now:yyyyMMdd_HHmmss}.json"
			};

			if (saveFileDialog.ShowDialog() == true)
			{
				try
				{
					var exportData = new System.Collections.Generic.List<object>();
					foreach (var obj in logList)
					{
						if (obj is LogEntryViewModel entry)
						{
							exportData.Add(new
							{
								Timestamp = entry.Timestamp,
								Time = entry.TimeFormatted,
								Source = entry.SourceName,
								Level = entry.LevelName,
								Text = entry.Text,
								File = entry.File,
								Function = entry.Function,
								Line = entry.Line
							});
						}
					}

					string jsonString = JsonSerializer.Serialize(exportData, new JsonSerializerOptions { WriteIndented = true });
					File.WriteAllText(saveFileDialog.FileName, jsonString);
				}
				catch (Exception ex)
				{
					MessageBox.Show($"Не удалось сохранить логи: {ex.Message}", "Ошибка экспорта", MessageBoxButton.OK, MessageBoxImage.Error);
				}
			}
		}
	}
}
