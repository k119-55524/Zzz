using System;
using editor.Services;

namespace editor.ViewModels
{
    public class LogEntryViewModel : ViewModelBase
    {
        private bool _isExpanded;

        public ulong Timestamp { get; }
        public LogSource Source { get; }
        public LogLevel Level { get; }
        public string Text { get; }
        public string File { get; }
        public string Function { get; }
        public uint Line { get; }

        public LogEntryViewModel(LogMessage msg)
        {
            Timestamp = msg.Timestamp;
            Source = msg.Source;
            Level = msg.Level;
            Text = msg.Text;
            File = msg.File;
            Function = msg.Function;
            Line = msg.Line;
        }

        public string TimeFormatted
        {
            get
            {
                var dt = DateTimeOffset.FromUnixTimeMilliseconds((long)Timestamp).LocalDateTime;
                return dt.ToString("HH:mm:ss.fff");
            }
        }

        public bool IsExpanded
        {
            get => _isExpanded;
            set => SetField(ref _isExpanded, value);
        }

        private static string GetLocString(string key)
        {
            if (System.Windows.Application.Current == null) return string.Empty;
            if (System.Windows.Application.Current.Dispatcher.CheckAccess())
            {
                return System.Windows.Application.Current.TryFindResource(key) as string ?? string.Empty;
            }
            return System.Windows.Application.Current.Dispatcher.Invoke(() => System.Windows.Application.Current.TryFindResource(key) as string) ?? string.Empty;
        }

        // Краткая строка для свёрнутого вида: только первая строка, остальное доступно через
        // DetailsText при разворачивании - иначе многострочные сообщения (например, ошибки CMake)
        // ломают список логов на несколько визуальных строк.
        public string SummaryText
        {
            get
            {
                int newlineIndex = Text.IndexOfAny(new[] { '\r', '\n' });
                return newlineIndex >= 0 ? Text.Substring(0, newlineIndex) + " …" : Text;
            }
        }

        public string LevelName => GetLocString($"LogLevel_{Level}") is string s && !string.IsNullOrEmpty(s) ? s : Level.ToString();

        public string SourceName => GetLocString($"LogSource_{Source}") is string s && !string.IsNullOrEmpty(s) ? s : Source.ToString();

        public string DetailsText
        {
            get
            {
                var systemTime = DateTimeOffset.FromUnixTimeMilliseconds((long)Timestamp).LocalDateTime.ToString("yyyy-MM-dd HH:mm:ss.fff");
                var details = $"[{GetLocString("Console_Details_Time")}]: {systemTime}\n" +
                              $"[{GetLocString("Console_Details_Source")}]: {SourceName}\n" +
                              $"[{GetLocString("Console_Details_Level")}]: {LevelName}\n" +
                              $"[{GetLocString("Console_Details_Message")}]: {Text}";

                if (!string.IsNullOrEmpty(File))
                {
                    details += $"\n[{GetLocString("Console_Details_File")}]: {File}";
                }
                if (!string.IsNullOrEmpty(Function))
                {
                    details += $"\n[{GetLocString("Console_Details_Function")}]: {Function}";
                }
                if (Line > 0)
                {
                    details += $"\n[{GetLocString("Console_Details_Line")}]: {Line}";
                }

                return details;
            }
        }

        public void RefreshLocalization()
        {
            OnPropertyChanged(nameof(LevelName));
            OnPropertyChanged(nameof(SourceName));
            OnPropertyChanged(nameof(DetailsText));
        }
    }
}
