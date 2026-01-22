# Test script for gitcopy
$gitcopyExe = "$PSScriptRoot\bin\release\gitcopy.exe"
$testDir = "$PSScriptRoot\temp\test_repo"
$backupDir = "$PSScriptRoot\temp\backups"

# Clean up previous runs
if (Test-Path $testDir) { Remove-Item -Recurse -Force $testDir }
if (Test-Path $backupDir) { Remove-Item -Recurse -Force $backupDir }

# 1. Setup Dummy Repo
Write-Host "Setting up dummy repo..."
New-Item -ItemType Directory -Path $testDir | Out-Null
Set-Location $testDir
git init
"content1" | Out-File "file1.txt"
New-Item -ItemType Directory -Path "subdir" | Out-Null
"content2" | Out-File "subdir\file2.txt"
"ignoreme" | Out-File "ignore.tmp" -Encoding ascii
"*.tmp" | Out-File ".gitignore" -Encoding ascii

git add .
git commit -m "Initial commit"

# TEST: Version Info
Write-Host "Testing Version Info..."
& $gitcopyExe | Out-String | ForEach-Object {
    if ($_ -match "Version 1.0.0") { Write-Host "Version info found - OK" }
    if ($_ -match "Veenus Adiyodi") { Write-Host "Developer info found - OK" }
}

# TEST: Untracked Directory (The Bug Fix)
Write-Host "Testing Untracked Directory..."
New-Item -ItemType Directory -Path "untracked_dir" | Out-Null
"untracked content" | Out-File "untracked_dir\untracked_file.txt"
# Standard git status would show "untracked_dir/" which caused Access Denied

$output = & $gitcopyExe "." $backupDir "-c" 2>&1
$output | Write-Host
if ($output -match "gitcopy_readme.txt") {
    Write-Host "Readme report message confirmed - OK"
    # Verify content
    $readmePath = $output | Where-Object { $_ -match "Report created at: (.*)" } | ForEach-Object { $Matches[1] }
    if ($readmePath) {
        # Remove surrounding quotes if present
        $readmePath = $readmePath.Trim('"')
        Write-Host "Checking readme at: $readmePath"
        
        if (Test-Path $readmePath) {
             $readmeContent = Get-Content $readmePath
             Write-Host "Readme Content:"
             $readmeContent | Write-Host
             if ($readmeContent -match "Branch: master") { Write-Host "Branch name found in readme - OK" } else { Write-Error "Branch name missing in readme!" }
        } else {
             Write-Error "Readme file not found at extracted path: $readmePath"
        }
    }
} else { Write-Error "Readme report message missing!" }

# Verify file existence
$latestBackup = Get-ChildItem $backupDir | Sort-Object CreationTime -Descending | Select-Object -First 1
if (Test-Path "$($latestBackup.FullName)\untracked_dir\untracked_file.txt") { Write-Host "untracked_file.txt copied - OK" } else { Write-Error "untracked_file.txt missing!" }

# TEST: Deleted File Handling
Write-Host "Testing Deleted File Handling..."
Remove-Item "subdir\file2.txt"
Start-Sleep -Seconds 2
$outputDeleted = & $gitcopyExe "." $backupDir "-c" 2>&1
$outputDeleted | Write-Host
if ($outputDeleted -match "Skipped 1 deleted/missing files") { Write-Host "Skipped file reported - OK" }

Write-Host "Cleaning up test artifacts..."
if (Test-Path "$PSScriptRoot\temp") { Remove-Item -Recurse -Force "$PSScriptRoot\temp" }
Write-Host "Cleanup done."

Write-Host "Done."
