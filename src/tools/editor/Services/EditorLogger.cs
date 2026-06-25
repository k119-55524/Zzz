using System;

namespace editor.Services
{
    public enum LogSource
    {
        Editor,
        Engine,
        Scripts
    }

    public enum LogLevel
    {
        None = 0,
        Message = 1 << 0,
        Warning = 1 << 1,
        Error = 1 << 2,
        Exception = 1 << 3,
        Critical = 1 << 4,
        Fatal = 1 << 5,
        All = 0xFF
    }

    public struct LogMessage
    {
        public ulong Timestamp;
        public LogSource Source;
        public LogLevel Level;
        public string Text;
        public string File;
        public string Function;
        public uint Line;
    }

    public static class EditorLogger
    {
        public static event Action<LogMessage>? LogReceived;

        public static void Log(LogSource source, LogLevel level, string text, string file = "", string function = "", uint line = 0)
        {
            var msg = new LogMessage
            {
                Timestamp = (ulong)DateTimeOffset.UtcNow.ToUnixTimeMilliseconds(),
                Source = source,
                Level = level,
                Text = text,
                File = file,
                Function = function,
                Line = line
            };
            LogReceived?.Invoke(msg);
        }

        public static void LogInfo(string text, LogSource source = LogSource.Editor, string file = "", string function = "", uint line = 0)
        {
            Log(source, LogLevel.Message, text, file, function, line);
        }

        public static void LogWarning(string text, LogSource source = LogSource.Editor, string file = "", string function = "", uint line = 0)
        {
            Log(source, LogLevel.Warning, text, file, function, line);
        }

        public static void LogError(string text, LogSource source = LogSource.Editor, string file = "", string function = "", uint line = 0)
        {
            Log(source, LogLevel.Error, text, file, function, line);
        }
    }
}
