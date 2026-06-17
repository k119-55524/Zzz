using System;
using System.Collections.ObjectModel;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using NetworkLogListener.Models;

namespace NetworkLogListener;

public partial class AddressManagerWindow : Window
{
    private AppSettings _settings;
    public ObservableCollection<ListenAddress> Addresses { get; set; }

    public AddressManagerWindow(AppSettings settings)
    {
        InitializeComponent();
        _settings = settings;
        Addresses = new ObservableCollection<ListenAddress>(_settings.SavedAddresses);
        LstAddresses.ItemsSource = Addresses;
    }

    private void BtnAdd_Click(object sender, RoutedEventArgs e)
    {
        var name = TxtName.Text.Trim();
        var ip = TxtIp.Text.Trim();
        
        if (string.IsNullOrEmpty(ip) || !System.Net.IPAddress.TryParse(ip, out _))
        {
            MessageBox.Show(this, "Введите корректный IP Адрес (например, 127.0.0.1).", "Ошибка", MessageBoxButton.OK, MessageBoxImage.Warning);
            return;
        }

        if (!int.TryParse(TxtPort.Text.Trim(), out int port) || port <= 0 || port > 65535)
        {
            MessageBox.Show(this, "Некорректный порт (от 1 до 65535).", "Ошибка", MessageBoxButton.OK, MessageBoxImage.Warning);
            return;
        }

        var addr = new ListenAddress { Name = name, IpAddress = ip, Port = port };
        Addresses.Add(addr);
        
        _settings.SavedAddresses = Addresses.ToList();
        SettingsManager.Save(_settings);

        TxtName.Clear();
    }

    private void BtnDelete_Click(object sender, RoutedEventArgs e)
    {
        if (sender is Button btn && btn.Tag is ListenAddress addr)
        {
            if (addr.IsBuiltIn)
            {
                MessageBox.Show(this, "Базовый адрес Localhost нельзя удалить.", "Внимание", MessageBoxButton.OK, MessageBoxImage.Information);
                return;
            }

            Addresses.Remove(addr);
            _settings.SavedAddresses = Addresses.ToList();
            SettingsManager.Save(_settings);
        }
    }

    private void BtnClose_Click(object sender, RoutedEventArgs e)
    {
        Close();
    }
}
