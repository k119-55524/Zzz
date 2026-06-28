using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using editor.Services.Project;

namespace editor.Services.Project.Infrastructure
{
    public class FolderStatistics
    {
        public int SubfoldersCount { get; set; }
        public int FilesCount { get; set; }
        public long TotalSize { get; set; }
    }

    public static class FolderStatisticsCalculator
    {
        public static FolderStatistics Calculate(VirtualNode node, IEnumerable<VirtualNode> roots, string projectRoot, bool isSystemMode, List<string> disabledFilters)
        {
            var stats = new FolderStatistics();
            if (!node.IsFolder)
            {
                return stats;
            }

            int subfolders = 0;
            int files = 0;
            long size = 0;

            foreach (var child in node.Children)
            {
                if (child.IsFolder)
                {
                    subfolders++;
                }
                else
                {
                    files++;
                    string? relPath = FindRelativePathFromRoot(roots, child);
                    if (relPath != null)
                    {
                        size += GetFileSize(relPath, projectRoot, isSystemMode, disabledFilters);
                    }
                }
            }

            stats.SubfoldersCount = subfolders;
            stats.FilesCount = files;
            stats.TotalSize = size;

            return stats;
        }

        public static string? FindRelativePathFromRoot(IEnumerable<VirtualNode> roots, VirtualNode target)
        {
            foreach (var root in roots)
            {
                if (root == target)
                {
                    return root.RelativePath;
                }
                var found = FindNodeInTree(root, target);
                if (found != null)
                {
                    return found.RelativePath;
                }
            }
            return null;
        }

        private static VirtualNode? FindNodeInTree(VirtualNode current, VirtualNode target)
        {
            foreach (var child in current.Children)
            {
                if (child == target)
                {
                    return child;
                }
                var found = FindNodeInTree(child, target);
                if (found != null)
                {
                    return found;
                }
            }
            return null;
        }

        private static long GetFileSize(string relativePath, string projectRoot, bool isSystemMode, List<string> disabledFilters)
        {
            try
            {
                if (isSystemMode)
                {
                    string path = Path.Combine(projectRoot, relativePath);
                    if (File.Exists(path))
                    {
                        return new FileInfo(path).Length;
                    }
                }
                else
                {
                    // Assets mode
                    foreach (var resourceFolder in ProjectStructure.AssetDirectories)
                    {
                        string resTypeName = Path.GetFileName(resourceFolder.RelativePath);
                        if (disabledFilters.Contains(resTypeName))
                            continue;

                        string resDir = Path.Combine(projectRoot, resourceFolder.RelativePath);
                        string path = Path.Combine(resDir, relativePath);
                        if (File.Exists(path))
                        {
                            return new FileInfo(path).Length;
                        }
                    }
                }
            }
            catch
            {
                // ignore
            }
            return 0;
        }
    }
}
