using System;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using editor.ViewModels;

namespace editor.Views.Widgets
{
    public partial class ConsoleWidget : UserControl
    {
        private ScrollViewer? _scrollViewer;
        private bool _autoScroll = true;

        public ConsoleWidget()
        {
            InitializeComponent();
        }

        private void UserControl_Loaded(object sender, RoutedEventArgs e)
        {
            _scrollViewer = FindVisualChild<ScrollViewer>(LogsListView);
            if (_scrollViewer != null)
            {
                _scrollViewer.ScrollChanged += ScrollViewer_ScrollChanged;
            }

            if (LogsListView.ItemsSource is INotifyCollectionChanged observable)
            {
                observable.CollectionChanged += Items_CollectionChanged;
            }
            else if (LogsListView.ItemsSource is ICollectionView collectionView && collectionView.SourceCollection is INotifyCollectionChanged viewObservable)
            {
                viewObservable.CollectionChanged += Items_CollectionChanged;
            }
        }

        private void ScrollViewer_ScrollChanged(object sender, ScrollChangedEventArgs e)
        {
            if (_scrollViewer == null) return;

            // Если прокрутка изменена по вертикали
            if (e.ExtentHeightChange == 0)
            {
                // Пользователь прокрутил вручную. 
                // Если он находится у самого низа (с погрешностью в 2 пикселя) - автопрокрутка активна.
                double bottomOffset = _scrollViewer.ScrollableHeight;
                _autoScroll = _scrollViewer.VerticalOffset >= bottomOffset - 2;
            }
        }

        private void Items_CollectionChanged(object? sender, NotifyCollectionChangedEventArgs e)
        {
            if (_autoScroll && _scrollViewer != null && e.Action == NotifyCollectionChangedAction.Add)
            {
                LogsListView.Dispatcher.BeginInvoke(new Action(() =>
                {
                    _scrollViewer.ScrollToEnd();
                }), System.Windows.Threading.DispatcherPriority.Background);
            }
        }

        private void LogItem_Click(object sender, MouseButtonEventArgs e)
        {
            if (sender is FrameworkElement element && element.DataContext is LogEntryViewModel log)
            {
                log.IsExpanded = !log.IsExpanded;
                e.Handled = true;
            }
        }

        private void CopyLogItem_Click(object sender, RoutedEventArgs e)
        {
            if (sender is FrameworkElement element && element.DataContext is LogEntryViewModel log)
            {
                try
                {
                    string logLine = $"[{log.TimeFormatted}] [{log.LevelName}] {log.Text}";
                    if (!string.IsNullOrEmpty(log.File))
                    {
                        logLine += $" (in {log.File}:{log.Line})";
                    }
                    System.Windows.Clipboard.SetText(logLine);
                }
                catch (Exception ex)
                {
                    MessageBox.Show($"Не удалось скопировать лог: {ex.Message}", "Ошибка", MessageBoxButton.OK, MessageBoxImage.Error);
                }
            }
        }


        private static T? FindVisualChild<T>(DependencyObject obj) where T : DependencyObject
        {
            for (int i = 0; i < VisualTreeHelper.GetChildrenCount(obj); i++)
            {
                var child = VisualTreeHelper.GetChild(obj, i);
                if (child is T tChild)
                {
                    return tChild;
                }
                var childOfChild = FindVisualChild<T>(child);
                if (childOfChild != null)
                {
                    return childOfChild;
                }
            }
            return null;
        }
    }
}
