# gitcopy

**gitcopy** is a lightweight C++ command-line tool designed to backup files from a Git repository to a destination folder. It creates a timestamped subdirectory for each backup and respects `.gitignore` rules.

## Features
- **Smart Copy**: Copies only tracked and untracked files (ignores files listed in `.gitignore`).
- **Timestamped Backups**: Creates destination folders in the format `YYYY MM DD HH MM SS <RepoName>`.
- **Changes Only Mode**: Optional flag to copy only modified and newly added files (perfect for differential backups).
- **Subdirectory Aware**: When run from a subdirectory, it copies only files within that scope but maintains the correct directory structure relative to the repository root.

## Usage

```powershell
gitcopy <source_directory> <destination_directory> [-c | -changes]
```

- `<source_directory>`: Path to the git repository or subdirectory you want to copy (use `.` for current directory).
- `<destination_directory>`: Root path where the backup folder will be created.
- `-c` or `-changes`: (Optional) Copy only modified and untracked files.

## Examples

### 1. Full Backup of Current Repository
Copy everything from the current directory to `G:\backups`.
```powershell
C:\myrepo> gitcopy . G:\backups
```
*Result:* Creates `G:\backups\2026 01 17 10 30 00 myrepo\` containing all files.

### 2. Backup Only Changes
Copy only files that have been modified or are untracked.
```powershell
C:\myrepo> gitcopy . G:\backups -c
```
*Result:* Creates a backup folder containing only the changed files and a `gitcopy_readme.txt` report listing them.

### 3. Backup from a Subdirectory
If you are working deep in a project structure, you can run it from there.
```powershell
C:\myrepo\src\components> gitcopy . G:\backups
```
*Result:* Copies only files under `src\components`, but places them in `G:\backups\...\src\components`, preserving the full path structure.

## FAQ

**Q: Does gitcopy copy ignored files (like `node_modules` or `obj` folders)?**
A: No. It uses `git` commands internally to identify files, so it strictly respects your `.gitignore` rules.

**Q: What happens if a file is deleted in my working copy?**
A: If a file is deleted (but not yet committed), `gitcopy` will simply skip it without error. In `-c` mode, if a file is marked as deleted in git status, it will effectively be skipped.

**Q: Does it overwrite existing backups?**
A: No. Since every backup creates a new folder based on the current second (timestamp), it will always create a new directory unless you run it multiple times within the exact same second.

**Q: What is `gitcopy_readme.txt`?**
A: This file is generated only in `-c` (changes) mode. It contains a list of all files that were identified as modified or untracked at the time of the backup.

**Q: How do I build this tool?**
A: If you have the source code:
1. Open the project in VS Code.
2. Run the `Build gitcopy` task (Ctrl+Shift+B).
3. Alternatively, run `build.bat` from the command line (requires Visual Studio compiler environment).
