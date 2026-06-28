namespace editor.Services.Project.Infrastructure.UndoRedo
{
    /// <summary>
    /// Интерфейс для всех выполняемых и отменяемых команд в редакторе.
    /// </summary>
    public interface ICommand
    {
        /// <summary>
        /// Выполнить команду (или повторить при Redo).
        /// </summary>
        void Execute();

        /// <summary>
        /// Отменить команду (Undo).
        /// </summary>
        void Undo();
    }
}
