#pragma once
#include "raylib.h"
#include <string>
#include <vector>
#include <functional>

// git2.h forward declarations to avoid including in header
typedef struct git_repository git_repository;

// Command result
struct GitResult {
    bool success;
    std::string output;
    std::string error;
};

// File status for visualization
struct FileStatus {
    std::string path;
    enum Status { UNTRACKED, MODIFIED, STAGED, COMMITTED } status;
    Color color;
};

// Commit node for visualization
struct CommitNode {
    std::string hash;
    std::string message;
    std::string author;
    int x, y;  // Position for visualization
    std::vector<std::string> parents;
    std::vector<std::string> branches;  // Branches pointing to this commit
    
    std::string shortHash() const { return hash.substr(0, 7); }
};

// Git wrapper class
class GitWrapper {
public:
    GitWrapper();
    ~GitWrapper();
    
    // Repository management
    bool InitRepo(const std::string& path);
    bool OpenRepo(const std::string& path);
    void CloseRepo();
    bool IsRepoOpen() const { return repo != nullptr; }
    std::string GetRepoPath() const { return repoPath; }
    
    // Basic commands (Level 1)
    GitResult Init(const std::string& path);
    GitResult Add(const std::string& filePattern);
    GitResult Commit(const std::string& message);
    GitResult Status();
    
    // Branch commands (Level 2)
    GitResult CreateBranch(const std::string& branchName);
    GitResult Checkout(const std::string& branchName);
    GitResult Merge(const std::string& branchName);
    std::vector<std::string> GetBranches();
    
    // For simulation (Level 1 tutorial)
    bool CreateFile(const std::string& filename, const std::string& content);
    bool ModifyFile(const std::string& filename, const std::string& newContent);
    
    // Random content generation for testing internal structure visualization
    std::string GenerateRandomContent(int minLines = 5, int maxLines = 20);
    std::string GenerateRandomFilename();
    std::string GenerateRandomDirname();
    bool CreateRandomFile(const std::string& subdir = "");
    bool CreateRandomDirectory(const std::string& parentDir = "");
    bool AppendRandomContent(const std::string& filename);
    
    // Visualization data
    std::vector<FileStatus> GetWorkingDirectoryStatus();
    std::vector<CommitNode> GetCommitGraph(int maxCommits = 50);
    std::string GetHEAD() const;
    
    // Callbacks for real-time updates
    void SetStatusCallback(std::function<void(const std::vector<FileStatus>&)> cb);
    
    // Internal structure access
    struct GitObjectData {
        std::string hash;
        std::string type;  // "commit", "tree", "blob"
        std::string content;
        std::vector<std::string> children;
    };
    std::vector<GitObjectData> GetCommitObjects(const std::string& commitHash);
    
private:
    git_repository* repo;
    std::string repoPath;
    std::string headHash;
    std::function<void(const std::vector<FileStatus>&)> statusCallback;
    
    void NotifyStatusChange();
    void UpdateHEAD();
};
