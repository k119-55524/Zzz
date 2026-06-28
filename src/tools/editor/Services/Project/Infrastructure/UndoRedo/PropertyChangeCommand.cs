using System;

namespace editor.Services.Project.Infrastructure.UndoRedo
{
    /// <summary>
    /// Универсальная команда изменения значения свойства.
    /// </summary>
    public class PropertyChangeCommand<T> : ICommand
    {
        private readonly Action<T> _setValueDirectly;
        private readonly T _oldValue;
        private readonly T _newValue;
        private readonly Action _onChanged;

        public PropertyChangeCommand(Action<T> setValueDirectly, T oldValue, T newValue, Action onChanged)
        {
            _setValueDirectly = setValueDirectly;
            _oldValue = oldValue;
            _newValue = newValue;
            _onChanged = onChanged;
        }

        public void Execute()
        {
            _setValueDirectly(_newValue);
            _onChanged?.Invoke();
        }

        public void Undo()
        {
            _setValueDirectly(_oldValue);
            _onChanged?.Invoke();
        }
    }
}
