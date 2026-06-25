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

        public string LevelName => Level.ToString();

        public string SourceName => Source.ToString();

        public string DetailsText
        {
            get
            {
                var systemTime = DateTimeOffset.FromUnixTimeMilliseconds((long)Timestamp).LocalDateTime.ToString("yyyy-MM-dd HH:mm:ss.fff");
                var details = $"[Время]: {systemTime}\n" +
                              $"[Источник]: {Source}\n" +
                              $"[Уровень]: {Level}\n" +
                              $"[Сообщение]: {Text}";

                if (!string.IsNullOrEmpty(File))
                {
                    details += $"\n[Файл]: {File}";
                }
                if (!string.IsNullOrEmpty(Function))
                {
                    details += $"\n[Функция]: {Function}";
                }
                if (Line > 0)
                {
                    details += $"\n[Строка]: {Line}";
                }

                return details;
            }
        }
    }
}
