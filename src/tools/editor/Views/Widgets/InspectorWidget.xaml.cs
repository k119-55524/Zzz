using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using editor.ViewModels;
using editor.Services.Project.Infrastructure;

namespace editor.Views.Widgets
{
    public partial class InspectorWidget : UserControl
    {
        private Point _collectionDragStartPoint;
        private CollectionItemViewModel? _collectionDragItem;

        public InspectorWidget()
        {
            InitializeComponent();
        }

        private void ClearSearch_Click(object sender, RoutedEventArgs e)
        {
            var button = sender as DependencyObject;
            while (button != null && !(button is TextBox))
            {
                button = VisualTreeHelper.GetParent(button);
            }
            if (button is TextBox textBox)
            {
                textBox.Text = string.Empty;
                textBox.Focus();
            }
        }

        // Применяет значение поля по Enter, не дожидаясь потери фокуса (AcceptsReturn=False,
        // так что Enter иначе ничего не делает - поле однострочное).
        private void ValueTextBox_PreviewKeyDown(object sender, System.Windows.Input.KeyEventArgs e)
        {
            if (e.Key == System.Windows.Input.Key.Enter && sender is TextBox textBox)
            {
                textBox.GetBindingExpression(TextBox.TextProperty)?.UpdateSource();
                e.Handled = true;
            }
        }

        private void CollectionListBox_PreviewMouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            // Перетаскивание начинается только с ручки (Tag="CollectionDragHandle"), а не с
            // произвольного клика по строке - иначе для редактируемых дефайнов клик по TextBox
            // (постановка курсора, выделение текста) постоянно путался бы с началом drag'а.
            if (sender is not ListBox { DataContext: TomlPropertyViewModel { IsSortableCollection: true } } ||
                !IsOnDragHandle(e.OriginalSource as DependencyObject))
            {
                _collectionDragItem = null;
                return;
            }

            _collectionDragStartPoint = e.GetPosition(null);
            _collectionDragItem = FindVisualParent<ListBoxItem>(e.OriginalSource as DependencyObject)?.DataContext as CollectionItemViewModel;
        }

        private static bool IsOnDragHandle(DependencyObject? source)
        {
            while (source != null && source is not ListBoxItem)
            {
                if (source is FrameworkElement { Tag: "CollectionDragHandle" })
                {
                    return true;
                }
                source = VisualTreeHelper.GetParent(source);
            }
            return false;
        }

        private void CollectionListBox_PreviewMouseMove(object sender, MouseEventArgs e)
        {
            if (_collectionDragItem == null || e.LeftButton != MouseButtonState.Pressed || sender is not ListBox listBox)
            {
                return;
            }

            Point current = e.GetPosition(null);
            Vector delta = _collectionDragStartPoint - current;
            if (System.Math.Abs(delta.X) < SystemParameters.MinimumHorizontalDragDistance &&
                System.Math.Abs(delta.Y) < SystemParameters.MinimumVerticalDragDistance)
            {
                return;
            }

            DragDrop.DoDragDrop(listBox, _collectionDragItem, DragDropEffects.Move);
            _collectionDragItem = null;
        }

        private void CollectionListBox_Drop(object sender, DragEventArgs e)
        {
            if (sender is not ListBox listBox ||
                listBox.DataContext is not TomlPropertyViewModel property)
            {
                return;
            }

            if (e.Data.GetData(typeof(ProjectNode)) is ProjectNode projectNode)
            {
                property.TryAddProjectNode(projectNode);
                return;
            }

            if (e.Data.GetData(typeof(CollectionItemViewModel)) is CollectionItemViewModel draggedItem)
            {
                var targetItem = FindVisualParent<ListBoxItem>(e.OriginalSource as DependencyObject)?.DataContext as CollectionItemViewModel;
                property.MoveCollectionItem(draggedItem, targetItem);
            }
        }

        private static T? FindVisualParent<T>(DependencyObject? child) where T : DependencyObject
        {
            while (child != null)
            {
                if (child is T parent)
                {
                    return parent;
                }
                child = VisualTreeHelper.GetParent(child);
            }
            return null;
        }
    }
}
