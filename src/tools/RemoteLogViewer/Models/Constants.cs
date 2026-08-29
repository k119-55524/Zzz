namespace RemoteLogViewer;

public static class Constants
{
    public const string LocalhostIp = "127.0.0.1";
    public const int DefaultPort = 3030;

    // Версия бинарного протокола передачи логов (см. c_LogProtocolVersion в src/core/utils/Constants.h движка) -
    // должна совпадать с движковой стороной, т.к. формат LogEntry::Serialize меняется вместе с ней.
    public const uint LogProtocolVersion = 2;
}
