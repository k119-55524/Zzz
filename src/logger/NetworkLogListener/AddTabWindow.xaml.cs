using System;
using System.Collections.Generic;
using System.Windows;
using NetworkLogListener.Models;

namespace NetworkLogListener;

public partial class AddTabWindow : Window
{
    public ListenAddress? SelectedAddress { get; private set; }

    public AddTabWindow(List<ListenAddress> addresses)
    {
        InitializeComponent();
        CmbAddresses.ItemsSource = addresses;
        if (addresses.Count > 0)
        {
            CmbAddresses.SelectedIndex = 0;
        }
    }

    private void BtnCreate_Click(object sender, RoutedEventArgs e)
    {
        if (CmbAddresses.SelectedItem is ListenAddress addr)
        {
            SelectedAddress = addr;
            DialogResult = true;
            Close();
        }
        else
        {
            MessageBox.Show(this, "Пожалуйста, выберите адрес.", "Внимание", MessageBoxButton.OK, MessageBoxImage.Warning);
        }
    }

    private void BtnCancel_Click(object sender, RoutedEventArgs e)
    {
        DialogResult = false;
        Close();
    }
}
