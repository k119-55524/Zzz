using System;

namespace editor.Services
{
    public class SelectionService
    {
        private object? _selectedItem;

        public object? SelectedItem
        {
            get => _selectedItem;
            set
            {
                if (_selectedItem != value)
                {
                    _selectedItem = value;
                    SelectedItemChanged?.Invoke(value);
                }
            }
        }

        public event Action<object?>? SelectedItemChanged;
    }
}
