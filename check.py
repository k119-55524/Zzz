import os
import re

def find_cmakelists(root):
    res = []
    for dp, dn, fn in os.walk(root):
        if 'CMakeLists.txt' in fn:
            res.append(os.path.join(dp, 'CMakeLists.txt'))
    return res

def get_files_in_dir(root_dir):
    exts = {'.c', '.cpp', '.h', '.hpp', '.mm', '.m', '.cc', '.cxx'}
    files = []
    for dp, dn, fn in os.walk(root_dir):
        for f in fn:
            if any(f.endswith(e) for e in exts):
                rel = os.path.relpath(os.path.join(dp, f), root_dir)
                rel = rel.replace('\\', '/')
                if 'build' in rel or 'out' in rel:
                    continue
                files.append(rel)
    return files

def check_cmake(filepath):
    dir_path = os.path.dirname(filepath)
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()

    listed_files = []
    matches = re.findall(r'\"?([a-zA-Z0-9_/\.\-]+\.(?:cpp|c|h|hpp|mm|m|cc|cxx))\"?', content)
    for m in matches:
        listed_files.append(m)

    actual_files = get_files_in_dir(dir_path)

    listed_set = set(listed_files)
    actual_set = set(actual_files)

    missing_in_cmake = actual_set - listed_set
    missing_in_fs = listed_set - actual_set

    if missing_in_cmake or missing_in_fs:
        print(f"--- {filepath} ---")
        if missing_in_cmake:
            print("Missing in CMakeLists.txt:")
            for m in sorted(missing_in_cmake):
                print(f"  {m}")
        if missing_in_fs:
            print("Missing in file system:")
            for m in sorted(missing_in_fs):
                print(f"  {m}")
        print()

for cm in find_cmakelists(r'C:\Workspaces\ZzzTest'):
    check_cmake(cm)
