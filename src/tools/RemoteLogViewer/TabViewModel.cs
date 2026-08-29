using System;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Windows;
using System.Windows.Data;
using System.Text;
using RemoteLogViewer.Models;

namespace RemoteLogViewer;

public enum ConnectionState
{
	Disconnected,
	Waiting,
	Connected
}

public class TabViewModel : INotifyPropertyChanged
{
	private TabSettings _settings;
	private NetworkReceiver _receiver;
	private ConnectionState _state = ConnectionState.Disconnected;

	public TabSettings Settings => _settings;
	public ListenAddress Address => _settings.Address;

	public ObservableCollection<LogEntry> Logs { get; } = new ObservableCollection<LogEntry>();
	public ICollectionView LogsView { get; }

	// Динамический список фильтров по категориям - пополняется по мере поступления новых категорий в этой
	// вкладке (сама вкладка не знает заранее полный список категорий движка). Видимость каждой категории
	// подмешивается из Filters.CategoryVisibility (персистентно, composite-ключ = имя категории), см. Models.cs.
	public ObservableCollection<CategoryFilterItem> CategoryFilters { get; } = new ObservableCollection<CategoryFilterItem>();

	public ConnectionState State
	{
		get => _state;
		private set { if (_state != value) { _state = value; OnPropertyChanged(); OnPropertyChanged(nameof(StatusText)); OnPropertyChanged(nameof(StatusColor)); } }
	}

	public string StatusText => State switch
	{
		ConnectionState.Connected => "Подключен",
		ConnectionState.Waiting => "В ожидании",
		_ => "Отключен"
	};

	public string StatusColor => State switch
	{
		ConnectionState.Connected => "#00CC00", // Зелёный
		ConnectionState.Waiting => "#CCCC00",   // Жёлтый
		_ => "#808080"                          // Серый
	};

	public bool IsAutoConnect
	{
		get => _settings.AutoConnect;
		set { if (_settings.AutoConnect != value) { _settings.AutoConnect = value; OnPropertyChanged(); } }
	}

	public bool IsAutoClear
	{
		get => _settings.AutoClearOnStart;
		set { if (_settings.AutoClearOnStart != value) { _settings.AutoClearOnStart = value; OnPropertyChanged(); } }
	}

	public TabFilters Filters => _settings.Filters;

	public TabViewModel(TabSettings settings)
	{
		_settings = settings;
		_receiver = new NetworkReceiver(settings.Address.IpAddress, settings.Address.Port);

		_receiver.StateChanged += (s, state) => Application.Current.Dispatcher.InvokeAsync(() => State = state);
		_receiver.LogReceived += OnLogReceived;
		_receiver.SessionStarted += OnSessionStarted;

		LogsView = CollectionViewSource.GetDefaultView(Logs);
		LogsView.Filter = FilterLog;

		if (IsAutoConnect)
		{
			Start();
		}
	}

	private bool _hasUserStopped = false;

	public void Start()
	{
		_receiver.Start();
	}

	public void Stop()
	{
		_hasUserStopped = true;
		_receiver.Stop();
	}

	public void Clear() => Logs.Clear();

	public void ExpandAll()
	{
		foreach (var entry in Logs)
			entry.IsExpanded = true;
	}

	public void CollapseAll()
	{
		foreach (var entry in Logs)
			entry.IsExpanded = false;
	}

	public void CopyAll()
	{
		var sb = new StringBuilder();
		foreach (var item in LogsView)
		{
			if (item is LogEntry entry)
			{
				sb.AppendLine($">>>>> {entry.Id} - {entry.Timestamp:HH:mm:ss.fff} {entry.Level}");
				sb.AppendLine($"    Message: {entry.MessageFull?.TrimEnd()}");
				if (!string.IsNullOrEmpty(entry.File)) sb.AppendLine($"    File: {entry.File}");
				if (entry.Line > 0) sb.AppendLine($"    Line: {entry.Line}");
				if (!string.IsNullOrEmpty(entry.Function)) sb.AppendLine($"    Function: {entry.Function}");
				sb.AppendLine();
			}
		}
		if (sb.Length > 0)
		{
			Clipboard.SetText(sb.ToString());
		}
	}

	public void ApplyFilter()
	{
		LogsView.Refresh();
	}

	private void OnSessionStarted(object? sender, EventArgs e)
	{
		if (IsAutoClear && !_hasUserStopped)
		{
			Application.Current.Dispatcher.InvokeAsync(() => Clear());
		}
		_hasUserStopped = false;
	}

	private void OnLogReceived(object? sender, LogEntry entry)
	{
		Application.Current.Dispatcher.InvokeAsync(() =>
		{
			EnsureCategoryFilter(entry.Category, entry.CategoryIsGuaranteed);
			entry.Id = Logs.Count + 1;
			Logs.Add(entry);
		});
	}

	// Регистрирует категорию в списке фильтров вкладки при первом появлении (вызывается из UI-потока, см.
	// OnLogReceived). Начальная видимость берётся из ранее сохранённых настроек (Filters.CategoryVisibility),
	// иначе категория видна по умолчанию (opt-out, как и на движке).
	private void EnsureCategoryFilter(string categoryName, bool isGuaranteed)
	{
		if (string.IsNullOrEmpty(categoryName))
			return;

		foreach (var existing in CategoryFilters)
		{
			if (existing.Name == categoryName)
				return;
		}

		bool isVisible = !Filters.CategoryVisibility.TryGetValue(categoryName, out var savedVisible) || savedVisible;
		var item = new CategoryFilterItem(categoryName, isGuaranteed, isVisible);
		item.PropertyChanged += (s, e) =>
		{
			Filters.CategoryVisibility[item.Name] = item.IsVisible;
			ApplyFilter();
		};
		CategoryFilters.Add(item);
	}

	private bool FilterLog(object obj)
	{
		if (obj is not LogEntry entry) return false;

		bool typeMatch = entry.Level switch
		{
			LogLevel.Message => Filters.ShowMessage,
			LogLevel.Warning => Filters.ShowWarning,
			LogLevel.Error => Filters.ShowError,
			LogLevel.Exception => Filters.ShowException,
			LogLevel.Critical => Filters.ShowCritical,
			LogLevel.Fatal => Filters.ShowFatal,
			_ => Filters.ShowMessage
		};

		if (!typeMatch) return false;

		// View Filter по категории - независим от рантайм-фильтра движка (см. комментарий у
		// TabFilters.CategoryVisibility в Models.cs): отсутствие записи в словаре = категория видна.
		if (!string.IsNullOrEmpty(entry.Category) &&
			Filters.CategoryVisibility.TryGetValue(entry.Category, out bool categoryVisible) && !categoryVisible)
			return false;

		if (!string.IsNullOrWhiteSpace(Filters.MessageFilter) && !entry.MessageFull.Contains(Filters.MessageFilter, StringComparison.OrdinalIgnoreCase))
			return false;

		if (!string.IsNullOrWhiteSpace(Filters.FileFilter) && !entry.File.Contains(Filters.FileFilter, StringComparison.OrdinalIgnoreCase))
			return false;

		if (!string.IsNullOrWhiteSpace(Filters.FunctionFilter) && !entry.Function.Contains(Filters.FunctionFilter, StringComparison.OrdinalIgnoreCase))
			return false;

		return true;
	}

	public void Dispose()
	{
		_receiver.Stop();
	}

	public event PropertyChangedEventHandler? PropertyChanged;
	protected void OnPropertyChanged([CallerMemberName] string? propertyName = null) =>
		PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
}
