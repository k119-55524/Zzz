$extensions = @('.c', '.cpp', '.h', '.hpp', '.mm', '.m', '.cc', '.cxx')

function Get-RelativePath {
    param(
        [string]$Path,
        [string]$BasePath
    )
    $pathUri = [uri]$Path
    $baseUri = [uri]"$BasePath\"
    $relativePath = $baseUri.MakeRelativeUri($pathUri).ToString()
    return [uri]::UnescapeDataString($relativePath)
}

$cmakelists = Get-ChildItem -Path "C:\Workspaces\ZzzTest" -Recurse -Filter "CMakeLists.txt"
foreach ($cm in $cmakelists) {
    $dirPath = $cm.Directory.FullName
    $content = Get-Content $cm.FullName -Raw -Encoding UTF8
    
    $listedFiles = @()
    $matches = [regex]::Matches($content, '\"?([a-zA-Z0-9_/\.\-]+\.(?:cpp|c|h|hpp|mm|m|cc|cxx))(?!\w)\"?')
    foreach ($m in $matches) {
        $listedFiles += $m.Groups[1].Value
    }
    
    $actualFiles = @()
    $files = Get-ChildItem -Path $dirPath -Recurse -File
    foreach ($f in $files) {
        if ($extensions -contains $f.Extension) {
            $relPath = Get-RelativePath -Path $f.FullName -BasePath $dirPath
            # Exclude build or out directories if necessary, though simple project might not have them
            if ($relPath -notmatch "build/" -and $relPath -notmatch "out/") {
                $actualFiles += $relPath
            }
        }
    }
    
    $listedSet = Set-Item -Path "Env:\dummy" -Value "" -PassThru # Just a dummy to clear out any previous vars, wait this is not set
    # Using arrays to find diffs
    $missingInCmake = @()
    foreach ($a in $actualFiles) {
        if ($listedFiles -notcontains $a) {
            $missingInCmake += $a
        }
    }
    
    $missingInFs = @()
    foreach ($l in $listedFiles) {
        if ($actualFiles -notcontains $l) {
            $missingInFs += $l
        }
    }
    
    if ($missingInCmake.Count -gt 0 -or $missingInFs.Count -gt 0) {
        Write-Host "--- $($cm.FullName) ---"
        if ($missingInCmake.Count -gt 0) {
            Write-Host "Missing in CMakeLists.txt:"
            foreach ($m in ($missingInCmake | Sort-Object)) {
                Write-Host "  $m"
            }
        }
        if ($missingInFs.Count -gt 0) {
            Write-Host "Missing in file system:"
            foreach ($m in ($missingInFs | Sort-Object)) {
                Write-Host "  $m"
            }
        }
        Write-Host ""
    }
}
