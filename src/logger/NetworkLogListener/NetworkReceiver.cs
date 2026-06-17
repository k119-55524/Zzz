using System;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using NetworkLogListener.Models;

namespace NetworkLogListener;

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
                // New connection accepted = Session Started
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
                    // Read 4 bytes length
                    byte[] lengthBuffer = new byte[4];
                    int read = await stream.ReadAsync(lengthBuffer, 0, 4, token);
                    if (read == 0) break; // Client disconnected
                    
                    uint length = BitConverter.ToUInt32(lengthBuffer, 0);
                    
                    // Read payload
                    byte[] payload = new byte[length];
                    int totalRead = 0;
                    while (totalRead < length)
                    {
                        read = await stream.ReadAsync(payload, totalRead, (int)length - totalRead, token);
                        if (read == 0) throw new EndOfStreamException();
                        totalRead += read;
                    }
                    
                    // Parse payload
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

    private void ParseLogEntry(byte[] payload)
    {
        try
        {
            using var ms = new MemoryStream(payload);
            using var reader = new BinaryReader(ms);
            
            var timestampMs = reader.ReadUInt64(); // Assuming timestamp is ms
            var typeRaw = reader.ReadUInt64();
            var text = ReadSizePrefixedString(reader);
            var file = ReadSizePrefixedString(reader);
            var function = ReadSizePrefixedString(reader);
            var line = reader.ReadUInt32();
            
            // Assume timestamp is milliseconds since epoch or system boot, convert to DateTime
            // For now, let's just use DateTime.Now to reflect when it arrived, since engine timestamp might be relative
            // Actually, let's try to add it to a base date. If it's a small number, it's relative.
            
            var entry = new LogEntry
            {
                Timestamp = DateTime.Now, // Use local time for UI display
                Level = (LogLevel)typeRaw,
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
        ulong size = reader.ReadUInt64();
        if (size == 0) return string.Empty;
        
        byte[] stringBytes = reader.ReadBytes((int)size);
        return Encoding.UTF8.GetString(stringBytes);
    }
}
