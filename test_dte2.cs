using System;
using System.Runtime.InteropServices;
using System.Reflection;

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
                rot.GetObject(monikers[0], out object dte);
                Console.WriteLine("Found DTE");
                try {
                    object debugger = dte.GetType().InvokeMember("Debugger", BindingFlags.GetProperty, null, dte, null);
                    Console.WriteLine("Debugger type: " + debugger.GetType().FullName);
                    // Try to get Transports
                    object transports = debugger.GetType().InvokeMember("Transports", BindingFlags.GetProperty, null, debugger, null);
                    Console.WriteLine("Transports type: " + transports.GetType().FullName);
                    
                    object defaultTransport = transports.GetType().InvokeMember("Item", BindingFlags.InvokeMethod, null, transports, new object[] { "Default" });
                    object engines = defaultTransport.GetType().InvokeMember("Engines", BindingFlags.GetProperty, null, defaultTransport, null);
                    int count = (int)engines.GetType().InvokeMember("Count", BindingFlags.GetProperty, null, engines, null);
                    for (int i = 1; i <= count; i++) {
                        object engine = engines.GetType().InvokeMember("Item", BindingFlags.InvokeMethod, null, engines, new object[] { i });
                        string engName = (string)engine.GetType().InvokeMember("Name", BindingFlags.GetProperty, null, engine, null);
                        Console.WriteLine("  Engine: " + engName);
                    }
                } catch (Exception ex) {
                    Console.WriteLine("Error: " + ex.Message);
                    if (ex.InnerException != null) Console.WriteLine("Inner: " + ex.InnerException.Message);
                }
                break;
            }
        }
    }
}
