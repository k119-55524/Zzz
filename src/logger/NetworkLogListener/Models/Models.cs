using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace NetworkLogListener.Models;

public enum LogLevel : byte
{
    None = 0,
    Message = 1,
    Warning = 2,
    Error = 4,
    Exception = 8,
    Critical = 16,
    Fatal = 32,
    All = 255
}

public class LogEntry : INotifyPropertyChanged
{
    public int Id { get; set; }
    public DateTime Timestamp { get; set; }
    public LogLevel Level { get; set; }
    public string MessageSummary { get; set; } = string.Empty;
    public string MessageFull { get; set; } = string.Empty;
    public string File { get; set; } = string.Empty;
    public string Function { get; set; } = string.Empty;
    public uint Line { get; set; }
    
    private bool _isExpanded;
    [System.Text.Json.Serialization.JsonIgnore]
    public bool IsExpanded
    {
        get => _isExpanded;
        set { if (_isExpanded != value) { _isExpanded = value; PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(IsExpanded))); } }
    }
    
    public event PropertyChangedEventHandler? PropertyChanged;
}

public class ListenAddress : INotifyPropertyChanged
{
    private string _name = string.Empty;
    private string _ipAddress = Constants.LocalhostIp;
    private int _port = Constants.DefaultPort;

    public string Name
    {
        get => _name;
        set { if (_name != value && value.Length <= 50) { _name = value; OnPropertyChanged(); } }
    }

    public string IpAddress
    {
        get => _ipAddress;
        set { if (_ipAddress != value) { _ipAddress = value; OnPropertyChanged(); } }
    }

    public int Port
    {
        get => _port;
        set { if (_port != value) { _port = value; OnPropertyChanged(); } }
    }

    public event PropertyChangedEventHandler? PropertyChanged;
    protected void OnPropertyChanged([CallerMemberName] string? propertyName = null) =>
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        
    [System.Text.Json.Serialization.JsonIgnore]
    public bool IsBuiltIn { get; set; }

    public override string ToString() => string.IsNullOrWhiteSpace(Name) ? $"{IpAddress}:{Port}" : $"{Name} ({IpAddress}:{Port})";
}

public class TabFilters
{
    public bool ShowMessage { get; set; } = true;
    public bool ShowWarning { get; set; } = true;
    public bool ShowError { get; set; } = true;
    public bool ShowException { get; set; } = true;
    public bool ShowCritical { get; set; } = true;
    public bool ShowFatal { get; set; } = true;
    public string MessageFilter { get; set; } = string.Empty;
    public string FileFilter { get; set; } = string.Empty;
    public string FunctionFilter { get; set; } = string.Empty;
}

public class TabSettings
{
    public string TabId { get; set; } = Guid.NewGuid().ToString();
    public ListenAddress Address { get; set; } = new ListenAddress();
    public bool AutoConnect { get; set; } = true;
    public bool AutoClearOnStart { get; set; } = true;
    public TabFilters Filters { get; set; } = new TabFilters();
}

public class AppSettings
{
    public double WindowWidth { get; set; } = 1024;
    public double WindowHeight { get; set; } = 768;
    public double WindowTop { get; set; } = 100;
    public double WindowLeft { get; set; } = 100;
    public int SelectedTabIndex { get; set; } = 0;
    
    public List<ListenAddress> SavedAddresses { get; set; } = new List<ListenAddress>
    {
        new ListenAddress { Name = "Localhost", IpAddress = Constants.LocalhostIp, Port = Constants.DefaultPort }
    };
    
    public List<TabSettings> SavedTabs { get; set; } = new List<TabSettings>();
}
