using System.Collections.ObjectModel;
using CommunityToolkit.Mvvm.ComponentModel;

namespace BuildConfigurator.ViewModels;

public abstract partial class ViewModelBase : ObservableObject
{
    // Вставляет элемент в ObservableCollection в позицию, сохраняющую алфавитный порядок по ключу
    // (без Sort/Clear+Add — сохраняет позиции/identity остальных элементов и не дёргает выделение в UI).
    protected static void InsertSorted<T>(ObservableCollection<T> collection, T item, Func<T, string> keySelector)
    {
        var key = keySelector(item);
        var index = 0;
        while (index < collection.Count &&
               string.Compare(keySelector(collection[index]), key, StringComparison.OrdinalIgnoreCase) < 0)
        {
            index++;
        }
        collection.Insert(index, item);
    }
}
