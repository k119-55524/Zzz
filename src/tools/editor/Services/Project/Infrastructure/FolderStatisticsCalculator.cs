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
        public static FolderStatistics Calculate(ProjectNode node, IEnumerable<ProjectNode> roots, string projectRoot)
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
                        size += GetFileSize(relPath, projectRoot);
                    }
                }
            }

            stats.SubfoldersCount = subfolders;
            stats.FilesCount = files;
            stats.TotalSize = size;

            return stats;
        }

        public static string? FindRelativePathFromRoot(IEnumerable<ProjectNode> roots, ProjectNode target)
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

        private static ProjectNode? FindNodeInTree(ProjectNode current, ProjectNode target)
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

        private static long GetFileSize(string relativePath, string projectRoot)
        {
            try
            {
                string path = Path.Combine(projectRoot, relativePath);
                if (File.Exists(path))
                {
                    return new FileInfo(path).Length;
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
