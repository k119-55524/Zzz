using System;
using System.Runtime.InteropServices;

class Program {
    [DllImport("ole32.dll")]
    private static extern int GetRunningObjectTable(int reserved, out System.Runtime.InteropServices.ComTypes.IRunningObjectTable prot);
    [DllImport("ole32.dll")]
    private static extern int CreateBindCtx(int reserved, out System.Runtime.InteropServices.ComTypes.IBindCtx ppbc);

    static void Main() {
        GetRunningObjectTable(0, out var rot);
        rot.EnumRunning(out var enumMoniker);
        var monikers = new System.Runtime.InteropServices.ComTypes.IMoniker[1];
        while (enumMoniker.Next(1, monikers, IntPtr.Zero) == 0) {
            CreateBindCtx(0, out var bindCtx);
            monikers[0].GetDisplayName(bindCtx, null, out string name);
            if (name.StartsWith("!VisualStudio.DTE")) {
                rot.GetObject(monikers[0], out object obj);
                dynamic dte = obj;
                Console.WriteLine("DTE: " + dte.Solution.FullName);
                try {
                    var transport = dte.Debugger.Transports.Item("Default");
                    foreach (dynamic engine in transport.Engines) {
                        Console.WriteLine("  Engine: " + engine.Name + " / " + engine.ID);
                    }
                } catch (Exception ex) { Console.WriteLine(ex.Message); }
                break;
            }
        }
    }
}
