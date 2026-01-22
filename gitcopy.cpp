// gitcopy tool by Veenus Adiyodi, 1074 Vectors
// 2026 Jan 
// Version 1.0.0

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <algorithm>
#include <array>
#include <fstream>

namespace fs = std::filesystem;

// Function to execute a shell command and capture its output
std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd, "r"), _pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

// Function to trim whitespace from both ends of a string
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (std::string::npos == first) {
        return str;
    }
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

// Function to get current time string yyyy mm dd HH MM SS
std::string getCurrentTimeStr() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm;
    localtime_s(&now_tm, &now_c);
    
    // Get also seconds part if needed, the prompt asks for it.
    // User example: 17 Jan 2026 6:45 pm 10 seconds -> 2026 01 17 18 45 10
    
    std::stringstream ss;
    ss << std::put_time(&now_tm, "%Y %m %d %H %M %S");
    return ss.str();
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "gitcopy - Version 1.0.0" << std::endl;
        std::cout << "Developer: Veenus Adiyodi, 1074 Vectors" << std::endl;
        std::cout << "Usage: gitcopy <source_dir> <destination_dir> [-changes|-c]" << std::endl;
        return 1;
    }

    std::string sourceArg = argv[1];
    std::string destArg = argv[2];
    bool changesOnly = false;

    if (argc >= 4) {
        std::string flag = argv[3];
        if (flag == "-changes" || flag == "-c") {
            changesOnly = true;
        }
    }

    try {
        fs::path startPath = fs::absolute(sourceArg);
        if (!fs::exists(startPath) || !fs::is_directory(startPath)) {
             std::cerr << "Error: Source directory does not exist: " << startPath << std::endl;
             return 1;
        }

        // 1. Find Git Root
        // We need to change directory to the source path to run git commands in context
        fs::current_path(startPath);
        
        std::string gitRootStr = trim(exec("git rev-parse --show-toplevel"));
        if (gitRootStr.empty()) {
            std::cerr << "Error: Not a git repository." << std::endl;
            return 1;
        }
        
        fs::path gitRoot = fs::path(gitRootStr);
        
        // 2. Determine repo name and subdirectory prefix
        std::string repoName = gitRoot.filename().string();
        
        // Calculate relative path from git root to startPath
        fs::path relativeFromRoot = fs::relative(startPath, gitRoot);
        std::string prefix = relativeFromRoot.string();
        if (prefix == ".") prefix = "";
        else {
             std::replace(prefix.begin(), prefix.end(), '\\', '/');
             if (!prefix.empty() && prefix.back() != '/') prefix += '/';
        }

        // 3. Prepare Destination
        std::string timeStr = getCurrentTimeStr();
        std::string destFolderName = timeStr + " " + repoName;
        fs::path destRoot = fs::absolute(destArg) / destFolderName;
        
        std::cout << "Copying to: " << destRoot << std::endl;
        
        fs::create_directories(destRoot);

        // 4. Get list of files
        std::vector<std::string> filesToCopy;
        
        if (changesOnly) {
             // Get modified and untracked files
             // Get branch name
             std::string branchName = trim(exec("git rev-parse --abbrev-ref HEAD"));

             std::string statusOutput = exec("git status --porcelain --untracked-files=all");
             std::stringstream ss(statusOutput);
             std::string line;
             std::ofstream readme(destRoot / "gitcopy_readme.txt");
             readme << "Changes Copy Report" << std::endl;
             readme << "Source: " << startPath << std::endl;
             readme << "Branch: " << branchName << std::endl;
             readme << "Date: " << timeStr << std::endl << std::endl;
             
             while (std::getline(ss, line)) {
                 if (line.length() > 3) {
                     std::string filePath = line.substr(3);
                     // Remove quotes if present
                     if (filePath.front() == '"' && filePath.back() == '"') {
                         filePath = filePath.substr(1, filePath.length() - 2);
                     }
                     
                     // Filter by prefix
                     if (prefix.empty() || filePath.rfind(prefix, 0) == 0) {
                         filesToCopy.push_back(filePath);
                         readme << line << std::endl;
                     }
                 }
             }
             readme.close();
             std::cout << "Report created at: " << (destRoot / "gitcopy_readme.txt") << std::endl;
        } else {
            // All files (tracked + untracked - ignored)
            // git ls-files --cached --others --exclude-standard
             std::string lsOutput = exec("git ls-files --cached --others --exclude-standard --full-name");
             std::stringstream ss(lsOutput);
             std::string filePath;
             while (std::getline(ss, filePath)) {
                 // Remove quotes if present
                 if (!filePath.empty() && filePath.front() == '"' && filePath.back() == '"') {
                     filePath = filePath.substr(1, filePath.length() - 2);
                 }
                 
                 // Filter by prefix
                 if (prefix.empty() || filePath.rfind(prefix, 0) == 0) {
                      filesToCopy.push_back(filePath);
                 }
             }
        }

        // 5. Copy Files
        std::cout << "Found " << filesToCopy.size() << " files to consider." << std::endl;
        int copiedCount = 0;
        int skippedCount = 0;
        for (const auto& relPathStr : filesToCopy) {
            fs::path relPath(relPathStr);
            fs::path srcFile = gitRoot / relPath;
            
            // Check if source file actually exists (handle deleted files case)
            if (!fs::exists(srcFile)) {
                skippedCount++;
                continue;
            }

            fs::path destFile = destRoot / relPath;
            
            try {
                fs::create_directories(destFile.parent_path());
                fs::copy_file(srcFile, destFile, fs::copy_options::overwrite_existing);
                copiedCount++;
            } catch (const std::exception& e) {
                // If it's a critical error we might want to know, but user said avoid confusion
                // For now, let's print generic error only if really needed, but silent is preferred for minor issues
                std::cerr << "Failed to copy " << relPathStr << ": " << e.what() << std::endl;
            }
        }
        
        std::cout << "Successfully copied " << copiedCount << " files.";
        if (skippedCount > 0) {
            std::cout << " (Skipped " << skippedCount << " deleted/missing files)";
        }
        std::cout << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
