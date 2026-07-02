namespace editor.Services.Project.Infrastructure.UndoRedo
{
    /// <summary>
    /// Marks commands that create/delete/rename/move files or folders, so that Undo/Redo
    /// know they must rebuild the Assets/System tree afterwards. Commands that only rewrite
    /// the contents of an existing file (e.g. config property edits) don't implement this -
    /// rebuilding the tree for them would replace unrelated ProjectNode instances and drop
    /// the current TreeView selection for no reason.
    /// </summary>
    public interface IAssetsTreeCommand : ICommand
    {
    }
}
