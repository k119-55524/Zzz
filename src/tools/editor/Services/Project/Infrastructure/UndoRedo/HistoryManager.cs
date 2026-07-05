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

        public HistoryManager(int maxCapacity = 100)
        {
            _maxCapacity = maxCapacity;
        }

        public bool CanUndo => _undoStack.Count > 0;
        public bool CanRedo => _redoStack.Count > 0;

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
        /// Отменить последнее действие (Undo). Возвращает отменённую команду (или null, если
        /// стек пуст) - вызывающий код использует её тип, чтобы решить, нужно ли перестраивать
        /// дерево ассетов (см. IAssetsTreeCommand).
        /// </summary>
        public ICommand? Undo()
        {
            if (_undoStack.Count == 0) return null;

            ICommand command = _undoStack.Pop();
            command.Undo();
            _redoStack.Push(command);
            return command;
        }

        /// <summary>
        /// Повторить последнее отмененное действие (Redo). Возвращает выполненную команду (см. Undo).
        /// </summary>
        public ICommand? Redo()
        {
            if (_redoStack.Count == 0) return null;

            ICommand command = _redoStack.Pop();
            command.Execute();
            _undoStack.Push(command);
            return command;
        }

        public void Clear()
        {
            _undoStack.Clear();
            ClearRedoStack();
        }

        private void ClearRedoStack()
        {
            while (_redoStack.Count > 0)
            {
                var command = _redoStack.Pop();

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
