using System;
using System.Runtime.CompilerServices;

namespace editor.Services.Project.Infrastructure.UndoRedo
{
    /// <summary>
    /// Базовый класс для объектов данных, поддерживающих автоматический Undo/Redo для своих свойств.
    /// </summary>
    public abstract class UndoableObject
    {
        // Менеджер истории, которому делегируются команды изменений
        protected HistoryManager? History { get; private set; }

        /// <summary>
        /// Событие, срабатывающее при изменении любого свойства объекта.
        /// Используется для автоматического сохранения на диск или уведомления UI.
        /// </summary>
        public event Action? OnChanged;

        protected void RaiseOnChanged()
        {
            OnChanged?.Invoke();
        }

        /// <summary>
        /// Установить менеджер истории. Если null, изменения будут происходить без записи истории.
        /// </summary>
        public void SetHistoryManager(HistoryManager? history)
        {
            History = history;
        }

        /// <summary>
        /// Устанавливает значение свойства с записью в историю Undo/Redo.
        /// </summary>
        /// <typeparam name="T">Тип значения свойства.</typeparam>
        /// <param name="field">Ссылка на приватное поле.</param>
        /// <param name="value">Новое значение.</param>
        /// <param name="setValueDirectly">Действие по прямой установке поля в обход публичного сеттера (во избежание зацикливания при Undo).</param>
        /// <param name="propertyName">Имя свойства.</param>
        /// <returns>True, если значение было изменено.</returns>
        protected bool SetProperty<T>(
            ref T field, 
            T value, 
            Action<T> setValueDirectly, 
            [CallerMemberName] string? propertyName = null)
        {
            if (Equals(field, value)) return false;

            if (History != null)
            {
                var command = new PropertyChangeCommand<T>(
                    setValueDirectly: setValueDirectly,
                    oldValue: field,
                    newValue: value,
                    onChanged: () => OnChanged?.Invoke()
                );

                History.Execute(command);
            }
            else
            {
                setValueDirectly(value);
                OnChanged?.Invoke();
            }

            return true;
        }
    }
}
