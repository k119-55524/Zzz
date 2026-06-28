using System;
using System.Collections.Generic;

namespace editor.Services.Project.Infrastructure.UndoRedo
{
    /// <summary>
    /// Менеджер истории изменений для Undo/Redo с поддержкой отслеживания состояния изменений (IsDirty).
    /// </summary>
    public class HistoryManager
    {
        private readonly Stack<ICommand> _undoStack = new();
        private readonly Stack<ICommand> _redoStack = new();
        private readonly int _maxCapacity;

        // Переменные для отслеживания точки сохранения
        private ICommand? _savePointCommand;
        private bool _isSavePointAtEmpty = true;
        private bool _hasSavedYet = false;

        public HistoryManager(int maxCapacity = 100)
        {
            _maxCapacity = maxCapacity;
        }

        public bool CanUndo => _undoStack.Count > 0;
        public bool CanRedo => _redoStack.Count > 0;

        /// <summary>
        /// Возвращает true, если текущая позиция в истории отличается от точки последнего сохранения.
        /// </summary>
        public bool IsDirty
        {
            get
            {
                if (!_hasSavedYet)
                {
                    return _undoStack.Count > 0;
                }

                bool atEmpty = _undoStack.Count == 0;
                if (atEmpty != _isSavePointAtEmpty) return true;

                ICommand? currentTop = _undoStack.Count > 0 ? _undoStack.Peek() : null;
                return currentTop != _savePointCommand;
            }
        }

        /// <summary>
        /// Отмечает текущую позицию в истории как точку сохранения (проект сохранен).
        /// </summary>
        public void MarkSavePoint()
        {
            _savePointCommand = _undoStack.Count > 0 ? _undoStack.Peek() : null;
            _isSavePointAtEmpty = _undoStack.Count == 0;
            _hasSavedYet = true;
        }

        /// <summary>
        /// Выполнить команду и записать её в историю.
        /// </summary>
        public void Execute(ICommand command)
        {
            command.Execute();
            _undoStack.Push(command);
            
            // Новое действие сбрасывает стек Redo.
            ClearRedoStack();

            // Ограничиваем глубину стека Undo.
            if (_undoStack.Count > _maxCapacity)
            {
                RemoveOldestUndoItem();
            }
        }

        /// <summary>
        /// Отменить последнее действие (Undo).
        /// </summary>
        public void Undo()
        {
            if (_undoStack.Count == 0) return;

            ICommand command = _undoStack.Pop();
            command.Undo();
            _redoStack.Push(command);
        }

        /// <summary>
        /// Повторить последнее отмененное действие (Redo).
        /// </summary>
        public void Redo()
        {
            if (_redoStack.Count == 0) return;

            ICommand command = _redoStack.Pop();
            command.Execute();
            _undoStack.Push(command);
        }

        /// <summary>
        /// Полностью очистить всю историю.
        /// </summary>
        public void Clear()
        {
            _undoStack.Clear();
            ClearRedoStack();
            _savePointCommand = null;
            _isSavePointAtEmpty = true;
            _hasSavedYet = false;
        }

        private void ClearRedoStack()
        {
            while (_redoStack.Count > 0)
            {
                var command = _redoStack.Pop();

                // Если точка сохранения была на команде из стека Redo, сбрасываем её
                if (command == _savePointCommand)
                {
                    _savePointCommand = null; // Точка сохранения больше недостижима
                }

                if (command is IDisposable disposable)
                {
                    disposable.Dispose();
                }
            }
        }

        private void RemoveOldestUndoItem()
        {
            var list = _undoStack.ToArray();
            Array.Reverse(list);

            _undoStack.Clear();

            // Самый старый элемент имеет индекс 0, мы его отбрасываем
            var oldest = list[0];
            if (oldest == _savePointCommand)
            {
                _savePointCommand = null; // Точка сохранения больше недостижима
            }

            if (oldest is IDisposable disposable)
            {
                disposable.Dispose();
            }

            for (int i = 1; i < list.Length; i++)
            {
                _undoStack.Push(list[i]);
            }
        }
    }
}
