using System.Runtime.InteropServices;

namespace assets_builder_lib;

public class AssetsBuilderEngine
{
    public string Version
    {
        get
        {
            try
            {
                var ptr = NativeMethods.GetBuilderEngineVersion();
                return ptr != IntPtr.Zero ? Marshal.PtrToStringAnsi(ptr) ?? "1.0.0" : "1.0.0";
            }
            catch
            {
                return "1.0.0 (Native DLL pending)";
            }
        }
    }

    public bool BuildProject(string projectJsonPath, string outputBinaryPath)
    {
        return NativeMethods.SerializeProjectManifest(projectJsonPath, outputBinaryPath);
    }
}
