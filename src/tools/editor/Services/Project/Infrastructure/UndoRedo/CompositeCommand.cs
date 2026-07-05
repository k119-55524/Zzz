using System;
using System.Collections.Generic;

namespace editor.Services.Project.Infrastructure.UndoRedo
{
    /// <summary>
    /// Объединяет несколько команд в одну атомарную операцию для Undo/Redo
    /// (например, переименование триады hpp+cpp+meta одним действием Ctrl+Z).
    /// </summary>
    public class CompositeCommand : IAssetsTreeCommand, IDisposable
    {
        private readonly List<ICommand> _commands;

        public CompositeCommand(List<ICommand> commands)
        {
            _commands = commands;
        }

        public void Execute()
        {
            foreach (var command in _commands)
            {
                command.Execute();
            }
        }

        public void Undo()
        {
            for (int i = _commands.Count - 1; i >= 0; i--)
            {
                _commands[i].Undo();
            }
        }

        public void Dispose()
        {
            foreach (var command in _commands)
            {
                if (command is IDisposable disposable)
                {
                    disposable.Dispose();
                }
            }
        }
    }
}
