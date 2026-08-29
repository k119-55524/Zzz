using System;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using RemoteLogViewer.Models;

namespace RemoteLogViewer;

public class NetworkReceiver
{
    private readonly string _ip;
    private readonly int _port;
    private TcpListener? _listener;
    private CancellationTokenSource? _cts;
    
    public event EventHandler<ConnectionState>? StateChanged;
    public event EventHandler<LogEntry>? LogReceived;
    public event EventHandler? SessionStarted;

    public NetworkReceiver(string ip, int port)
    {
        _ip = ip;
        _port = port;
    }

    public void Start()
    {
        if (_listener != null) return;

        try
        {
            IPAddress address = _ip == "0.0.0.0" ? IPAddress.Any : IPAddress.Parse(_ip);
            _listener = new TcpListener(address, _port);
            _listener.Start();
            
            _cts = new CancellationTokenSource();
            _ = AcceptLoopAsync(_cts.Token);
            
            StateChanged?.Invoke(this, ConnectionState.Waiting);
        }
        catch (Exception ex)
        {
            System.Diagnostics.Debug.WriteLine($"Failed to start listener: {ex.Message}");
            StateChanged?.Invoke(this, ConnectionState.Disconnected);
            _listener?.Stop();
            _listener = null;
        }
    }

    public void Stop()
    {
        _cts?.Cancel();
        _listener?.Stop();
        _listener = null;
        StateChanged?.Invoke(this, ConnectionState.Disconnected);
    }

    private async Task AcceptLoopAsync(CancellationToken token)
    {
        try
        {
            while (!token.IsCancellationRequested)
            {
                var client = await _listener!.AcceptTcpClientAsync(token);
                // Принято новое подключение = начало сессии
                SessionStarted?.Invoke(this, EventArgs.Empty);
                StateChanged?.Invoke(this, ConnectionState.Connected);
                
                _ = ReadClientAsync(client, token);
            }
        }
        catch (OperationCanceledException) { /* Ignored */ }
        catch (Exception ex)
        {
            System.Diagnostics.Debug.WriteLine($"Accept loop error: {ex.Message}");
        }
        finally
        {
            if (_listener != null)
            {
                Stop();
            }
        }
    }

    private async Task ReadClientAsync(TcpClient client, CancellationToken token)
    {
        using (client)
        using (var stream = client.GetStream())
        using (var reader = new BinaryReader(stream))
        {
            try
            {
                while (!token.IsCancellationRequested)
                {
                    // Читаем 4 байта длины
                    byte[] lengthBuffer = new byte[4];
                    int read = await stream.ReadAsync(lengthBuffer, 0, 4, token);
                    if (read == 0) break; // Клиент отключился

                    uint length = BitConverter.ToUInt32(lengthBuffer, 0);

                    // Читаем полезную нагрузку
                    byte[] payload = new byte[length];
                    int totalRead = 0;
                    while (totalRead < length)
                    {
                        read = await stream.ReadAsync(payload, totalRead, (int)length - totalRead, token);
                        if (read == 0) throw new EndOfStreamException();
                        totalRead += read;
                    }

                    // Разбираем полезную нагрузку
                    ParseLogEntry(payload);
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Client read error: {ex.Message}");
            }
            finally
            {
                StateChanged?.Invoke(this, ConnectionState.Waiting);
            }
        }
    }

    // Формат протокола v2 (см. c_LogProtocolVersion в Constants.h движка и LogEntry::Serialize):
    // [Version(u32), Timestamp(u64), Type(u64), CategoryGroup(u8), CategoryName(string), CategoryIsGuaranteed(bool/u8),
    //  Text(string), File(string), Function(string), Line(u32)]. Строки - u32-префикс длины + UTF8-байты (см.
    // Serializer::Serialize(std::string) - ДО этого рефакторинга здесь ошибочно читался u64-префикс).
    private void ParseLogEntry(byte[] payload)
    {
        try
        {
            using var ms = new MemoryStream(payload);
            using var reader = new BinaryReader(ms);

            var protocolVersion = reader.ReadUInt32();
            if (protocolVersion != Constants.LogProtocolVersion)
            {
                System.Diagnostics.Debug.WriteLine(
                    $"Unsupported log protocol version: {protocolVersion} (expected {Constants.LogProtocolVersion}). Entry skipped.");
                return;
            }

            var timestampMs = reader.ReadUInt64(); // Предполагаем, что timestamp в мс
            var typeRaw = reader.ReadUInt64();
            var categoryGroupRaw = reader.ReadByte();
            var categoryName = ReadSizePrefixedString(reader);
            var categoryIsGuaranteed = reader.ReadByte() != 0;
            var text = ReadSizePrefixedString(reader);
            var file = ReadSizePrefixedString(reader);
            var function = ReadSizePrefixedString(reader);
            var line = reader.ReadUInt32();

            // Предполагаем, что timestamp - миллисекунды от эпохи или от загрузки системы, конвертируем в DateTime
            // Пока просто используем DateTime.Now (момент получения), т.к. timestamp движка может быть относительным
            // На самом деле стоило бы попробовать прибавить его к базовой дате - если число маленькое, оно относительное.

            var entry = new LogEntry
            {
                Timestamp = DateTime.Now, // Локальное время для отображения в UI
                Level = (LogLevel)typeRaw,
                Category = categoryName,
                CategoryGroup = categoryGroupRaw == 0 ? "Engine" : "User",
                CategoryIsGuaranteed = categoryIsGuaranteed,
                MessageSummary = text.Length > 100 ? text.Substring(0, 100) + "..." : text,
                MessageFull = text,
                File = file,
                Function = function,
                Line = line
            };

            LogReceived?.Invoke(this, entry);
        }
        catch (Exception ex)
        {
            System.Diagnostics.Debug.WriteLine($"Parse error: {ex.Message}");
        }
    }

    private string ReadSizePrefixedString(BinaryReader reader)
    {
        uint size = reader.ReadUInt32();
        if (size == 0) return string.Empty;

        byte[] stringBytes = reader.ReadBytes((int)size);
        return Encoding.UTF8.GetString(stringBytes);
    }
}
