#include "git_wrapper.h"
#include <git2.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <filesystem>

GitWrapper::GitWrapper() : repo(nullptr) {
    git_libgit2_init();
}

GitWrapper::~GitWrapper() {
    CloseRepo();
    git_libgit2_shutdown();
}

bool GitWrapper::InitRepo(const std::string& path) {
    git_repository_init_options opts = GIT_REPOSITORY_INIT_OPTIONS_INIT;
    opts.flags |= GIT_REPOSITORY_INIT_MKPATH;
    
    int error = git_repository_init_ext(&repo, path.c_str(), &opts);
    if (error != 0) {
        const git_error* e = git_error_last();
        std::cerr << "Failed to init repo: " << (e ? e->message : "unknown error") << std::endl;
        return false;
    }
    
    repoPath = path;
    return true;
}

bool GitWrapper::OpenRepo(const std::string& path) {
    CloseRepo();
    
    int error = git_repository_open(&repo, path.c_str());
    if (error != 0) {
        return false;
    }
    
    repoPath = path;
    UpdateHEAD();
    return true;
}

void GitWrapper::CloseRepo() {
    if (repo) {
        git_repository_free(repo);
        repo = nullptr;
    }
}

GitResult GitWrapper::Init(const std::string& path) {
    if (InitRepo(path)) {
        return {true, "Initialized empty Git repository in " + path + "/.git/", ""};
    }
    return {false, "", "Failed to initialize repository"};
}

GitResult GitWrapper::Add(const std::string& filePattern) {
    if (!repo) return {false, "", "Not a git repository"};
    
    git_index* index = nullptr;
    git_repository_index(&index, repo);
    
    if (filePattern == ".") {
        git_strarray paths = {nullptr, 0};
        git_index_add_all(index, &paths, 0, nullptr, nullptr);
    } else {
        git_index_add_bypath(index, filePattern.c_str());
    }
    
    git_index_write(index);
    git_index_free(index);
    
    NotifyStatusChange();
    return {true, "Added " + filePattern + " to staging area", ""};
}

GitResult GitWrapper::Commit(const std::string& message) {
    if (!repo) return {false, "", "Not a git repository"};
    
    // Get index
    git_index* index = nullptr;
    if (git_repository_index(&index, repo) != 0) {
        return {false, "", "Failed to get index"};
    }
    
    // Write index to tree
    git_oid tree_oid;
    if (git_index_write_tree(&tree_oid, index) != 0) {
        git_index_free(index);
        return {false, "", "Failed to write tree"};
    }
    
    git_tree* tree = nullptr;
    if (git_tree_lookup(&tree, repo, &tree_oid) != 0) {
        git_index_free(index);
        return {false, "", "Failed to lookup tree"};
    }
    
    // Create signature
    git_signature* sig = nullptr;
    git_signature_now(&sig, "Player", "player@gitfighter.com");
    
    // Get parent commit (if exists)
    git_oid parent_oid;
    int have_parent = 0;
    if (git_reference_name_to_id(&parent_oid, repo, "HEAD") == 0) {
        have_parent = 1;
    }
    
    // Create commit
    git_oid commit_oid;
    int error;
    
    if (have_parent) {
        git_commit* parent = nullptr;
        git_commit_lookup(&parent, repo, &parent_oid);
        const git_commit* parents[] = {parent};
        error = git_commit_create(&commit_oid, repo, "HEAD", sig, sig, 
                                   NULL, message.c_str(), tree, 1, parents);
        git_commit_free(parent);
    } else {
        // First commit, no parents
        error = git_commit_create(&commit_oid, repo, "HEAD", sig, sig,
                                   NULL, message.c_str(), tree, 0, NULL);
    }
    
    git_signature_free(sig);
    git_tree_free(tree);
    git_index_free(index);
    
    if (error != 0) {
        const git_error* e = git_error_last();
        return {false, "", e ? e->message : "Failed to create commit"};
    }
    
    UpdateHEAD();
    NotifyStatusChange();
    
    char oid_str[GIT_OID_HEXSZ + 1];
    git_oid_tostr(oid_str, sizeof(oid_str), &commit_oid);
    return {true, "[main " + std::string(oid_str).substr(0, 7) + "] " + message, ""};
}

GitResult GitWrapper::Status() {
    if (!repo) return {false, "", "Not a git repository"};
    return {true, "On branch main\nNothing to commit, working tree clean", ""};
}

// Branch commands
GitResult GitWrapper::CreateBranch(const std::string& branchName) {
    if (!repo) return {false, "", "Not a git repository"};
    
    git_commit* commit = nullptr;
    git_oid commit_oid;
    
    // Get HEAD commit
    if (git_reference_name_to_id(&commit_oid, repo, "HEAD") != 0) {
        return {false, "", "No HEAD to create branch from"};
    }
    
    if (git_commit_lookup(&commit, repo, &commit_oid) != 0) {
        return {false, "", "Failed to lookup commit"};
    }
    
    git_reference* branch_ref = nullptr;
    int error = git_branch_create(&branch_ref, repo, branchName.c_str(), commit, 0);
    git_commit_free(commit);
    
    if (error != 0) {
        const git_error* e = git_error_last();
        return {false, "", e ? e->message : "Failed to create branch"};
    }
    
    git_reference_free(branch_ref);
    NotifyStatusChange();
    return {true, "Created branch '" + branchName + "'", ""};
}

GitResult GitWrapper::Checkout(const std::string& branchName) {
    if (!repo) return {false, "", "Not a git repository"};
    
    git_object* treeish = nullptr;
    int error = git_revparse_single(&treeish, repo, branchName.c_str());
    
    if (error != 0) {
        return {false, "", "Branch not found"};
    }
    
    git_checkout_options opts = GIT_CHECKOUT_OPTIONS_INIT;
    opts.checkout_strategy = GIT_CHECKOUT_SAFE;
    
    // Checkout the tree
    error = git_checkout_tree(repo, treeish, &opts);
    
    if (error != 0) {
        git_object_free(treeish);
        const git_error* e = git_error_last();
        return {false, "", e ? e->message : "Checkout failed"};
    }
    
    // Update HEAD to point to the branch
    git_reference* ref = nullptr;
    std::string ref_name = "refs/heads/" + branchName;
    error = git_reference_lookup(&ref, repo, ref_name.c_str());
    
    if (error == 0) {
        git_repository_set_head(repo, ref_name.c_str());
        git_reference_free(ref);
    }
    
    git_object_free(treeish);
    NotifyStatusChange();
    return {true, "Switched to branch '" + branchName + "'", ""};
}

GitResult GitWrapper::Merge(const std::string& branchName) {
    if (!repo) return {false, "", "Not a git repository"};
    // Simplified - just mark as success for now
    return {true, "Merged branch '" + branchName + "'", ""};
}

std::vector<std::string> GitWrapper::GetBranches() {
    std::vector<std::string> result;
    if (!repo) return result;
    
    git_branch_iterator* iter = nullptr;
    git_branch_iterator_new(&iter, repo, GIT_BRANCH_LOCAL);
    
    git_reference* ref = nullptr;
    git_branch_t branch_type;
    
    while (git_branch_next(&ref, &branch_type, iter) == 0) {
        const char* name = nullptr;
        git_branch_name(&name, ref);
        if (name) {
            result.push_back(name);
        }
        git_reference_free(ref);
    }
    
    git_branch_iterator_free(iter);
    return result;
}

bool GitWrapper::CreateFile(const std::string& filename, const std::string& content) {
    if (repoPath.empty()) return false;
    
    std::string filepath = repoPath + "/" + filename;
    std::ofstream file(filepath);
    if (!file.is_open()) return false;
    
    file << content;
    file.close();
    
    NotifyStatusChange();
    return true;
}

bool GitWrapper::ModifyFile(const std::string& filename, const std::string& newContent) {
    return CreateFile(filename, newContent);
}

// Random content generation
std::string GitWrapper::GenerateRandomContent(int minLines, int maxLines) {
    static const char* codeSnippets[] = {
        "// TODO: Implement this function",
        "console.log('Debug message');",
        "if (condition) { return true; }",
        "for (int i = 0; i < n; i++) {}",
        "while (running) { process(); }",
        "// FIXME: Memory leak here",
        "import std::vector;",
        "class MyClass { public: void run(); };",
        "// Performance optimization needed",
        "try { riskyOperation(); } catch (...) {}",
        "auto result = calculate(x, y);",
        "std::cout << \"Output: \" << value << std::endl;",
        "// Reviewed by: senior_dev",
        "mutex.lock(); // Critical section",
        "cache.invalidate(key);",
        "/* Multi-line comment\n * describing the logic\n */",
        "const double PI = 3.14159265359;",
        "static int counter = 0;",
        "#pragma once",
        "namespace app { namespace utils { } }"
    };
    
    static const char* loremWords[] = {
        "lorem", "ipsum", "dolor", "sit", "amet", "consectetur",
        "adipiscing", "elit", "sed", "do", "eiusmod", "tempor",
        "incididunt", "ut", "labore", "et", "dolore", "magna",
        "aliqua", "ut", "enim", "ad", "minim", "veniam"
    };
    
    std::string content;
    int numLines = minLines + (rand() % (maxLines - minLines + 1));
    
    for (int i = 0; i < numLines; i++) {
        int type = rand() % 3;
        if (type == 0) {
            // Code snippet
            content += codeSnippets[rand() % 20];
        } else if (type == 1) {
            // Random words
            int wordCount = 3 + (rand() % 8);
            for (int w = 0; w < wordCount; w++) {
                content += loremWords[rand() % 24];
                if (w < wordCount - 1) content += " ";
            }
        } else {
            // Empty line or separator
            content += "// ------------------------";
        }
        content += "\n";
    }
    
    return content;
}

std::string GitWrapper::GenerateRandomFilename() {
    static const char* prefixes[] = {"feature", "fix", "refactor", "update", "add", "remove"};
    static const char* middles[] = {"user", "auth", "data", "api", "ui", "core", "utils"};
    static const char* extensions[] = {".cpp", ".h", ".md", ".txt", ".py", ".js"};
    
    std::string filename = prefixes[rand() % 6];
    filename += "_";
    filename += middles[rand() % 7];
    filename += "_";
    filename += std::to_string(rand() % 100);
    filename += extensions[rand() % 6];
    
    return filename;
}

std::string GitWrapper::GenerateRandomDirname() {
    static const char* dirNames[] = {
        "components", "utils", "services", "models", "views",
        "controllers", "helpers", "middleware", "plugins", "modules",
        "core", "api", "tests", "docs", "assets"
    };
    
    return std::string(dirNames[rand() % 15]) + "_" + std::to_string(rand() % 100);
}

bool GitWrapper::CreateRandomFile(const std::string& subdir) {
    std::string filename = GenerateRandomFilename();
    if (!subdir.empty()) {
        filename = subdir + "/" + filename;
    }
    std::string content = GenerateRandomContent(5, 30);
    return CreateFile(filename, content);
}

bool GitWrapper::CreateRandomDirectory(const std::string& parentDir) {
    std::string dirName = GenerateRandomDirname();
    if (!parentDir.empty()) {
        dirName = parentDir + "/" + dirName;
    }
    
    std::string fullPath = repoPath + "/" + dirName;
    try {
        std::filesystem::create_directories(fullPath);
        
        // Create a few files inside the new directory
        int numFiles = 1 + (rand() % 4);
        for (int i = 0; i < numFiles; i++) {
            CreateRandomFile(dirName);
        }
        
        NotifyStatusChange();
        return true;
    } catch (...) {
        return false;
    }
}

bool GitWrapper::AppendRandomContent(const std::string& filename) {
    if (repoPath.empty()) return false;
    
    std::string filepath = repoPath + "/" + filename;
    std::ofstream file(filepath, std::ios::app);
    if (!file.is_open()) return false;
    
    file << "\n\n// Random addition at " << (rand() % 1000) << "\n";
    file << GenerateRandomContent(3, 10);
    file.close();
    
    NotifyStatusChange();
    return true;
}

std::vector<FileStatus> GitWrapper::GetWorkingDirectoryStatus() {
    std::vector<FileStatus> result;
    if (!repo) return result;
    
    git_status_list* status_list = nullptr;
    git_status_options opts = GIT_STATUS_OPTIONS_INIT;
    opts.show = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
    
    if (git_status_list_new(&status_list, repo, &opts) != 0) {
        return result;
    }
    
    size_t count = git_status_list_entrycount(status_list);
    for (size_t i = 0; i < count; i++) {
        const git_status_entry* entry = git_status_byindex(status_list, i);
        if (!entry) continue;
        
        FileStatus fs;
        if (entry->head_to_index) {
            fs.path = entry->head_to_index->new_file.path;
        } else if (entry->index_to_workdir) {
            fs.path = entry->index_to_workdir->new_file.path;
        }
        
        if (entry->status & GIT_STATUS_WT_NEW) {
            fs.status = FileStatus::UNTRACKED;
            fs.color = RED;
        } else if (entry->status & GIT_STATUS_INDEX_NEW) {
            fs.status = FileStatus::STAGED;
            fs.color = GREEN;
        } else {
            fs.status = FileStatus::MODIFIED;
            fs.color = YELLOW;
        }
        
        result.push_back(fs);
    }
    
    git_status_list_free(status_list);
    return result;
}

std::vector<CommitNode> GitWrapper::GetCommitGraph(int maxCommits) {
    std::vector<CommitNode> result;
    if (!repo) {
        return result;
    }
    
    std::string head = GetHEAD();
    
    if (head.empty()) {
        return result;
    }
    
    // Walk commit history
    git_revwalk* walker = nullptr;
    git_revwalk_new(&walker, repo);
    git_revwalk_push_head(walker);
    git_revwalk_sorting(walker, GIT_SORT_TIME);
    
    git_oid oid;
    int count = 0;
    while (git_revwalk_next(&oid, walker) == 0 && count < maxCommits) {
        git_commit* commit = nullptr;
        if (git_commit_lookup(&commit, repo, &oid) != 0) {
            continue;
        }
        
        CommitNode node;
        char oid_str[GIT_OID_HEXSZ + 1];
        git_oid_tostr(oid_str, sizeof(oid_str), &oid);
        node.hash = oid_str;
        node.message = git_commit_message(commit) ? git_commit_message(commit) : "";
        
        const git_signature* author = git_commit_author(commit);
        if (author) {
            node.author = author->name ? author->name : "";
        }
        
        // Get parents
        unsigned int parent_count = git_commit_parentcount(commit);
        for (unsigned int i = 0; i < parent_count; i++) {
            const git_oid* parent_oid = git_commit_parent_id(commit, i);
            if (parent_oid) {
                char parent_str[GIT_OID_HEXSZ + 1];
                git_oid_tostr(parent_str, sizeof(parent_str), parent_oid);
                node.parents.push_back(parent_str);
            }
        }
        
        result.push_back(node);
        git_commit_free(commit);
        count++;
    }
    
    git_revwalk_free(walker);
    return result;
}

std::string GitWrapper::GetHEAD() const {
    if (!repo) return "";
    
    git_oid head_oid;
    if (git_reference_name_to_id(&head_oid, repo, "HEAD") != 0) {
        return "";
    }
    
    char oid_str[GIT_OID_HEXSZ + 1];
    git_oid_tostr(oid_str, sizeof(oid_str), &head_oid);
    return std::string(oid_str);
}

void GitWrapper::UpdateHEAD() {
    headHash = GetHEAD();
}

void GitWrapper::SetStatusCallback(std::function<void(const std::vector<FileStatus>&)> cb) {
    statusCallback = cb;
}

void GitWrapper::NotifyStatusChange() {
    if (statusCallback) {
        statusCallback(GetWorkingDirectoryStatus());
    }
}

std::vector<GitWrapper::GitObjectData> GitWrapper::GetCommitObjects(const std::string& commitHash) {
    std::vector<GitObjectData> result;
    if (!repo) return result;
    
    // Parse commit hash
    git_oid commit_oid;
    if (git_oid_fromstr(&commit_oid, commitHash.c_str()) != 0) {
        return result;
    }
    
    // Lookup commit
    git_commit* commit = nullptr;
    if (git_commit_lookup(&commit, repo, &commit_oid) != 0) {
        return result;
    }
    
    // Add commit object
    {
        GitObjectData commitObj;
        commitObj.hash = commitHash;
        commitObj.type = "commit";
        
        // Get commit content
        const char* message = git_commit_message(commit);
        const git_signature* author = git_commit_author(commit);
        commitObj.content = std::string("commit ") + commitHash.substr(0, 7) + "\n" +
                           "Author: " + (author ? author->name : "Unknown") + "\n" +
                           "\n" + (message ? message : "");
        
        // Get tree as child
        git_tree* tree = nullptr;
        if (git_commit_tree(&tree, commit) == 0) {
            char tree_oid_str[GIT_OID_HEXSZ + 1];
            git_oid_tostr(tree_oid_str, sizeof(tree_oid_str), git_tree_id(tree));
            commitObj.children.push_back(tree_oid_str);
            
            // Process tree
            GitObjectData treeObj;
            treeObj.hash = tree_oid_str;
            treeObj.type = "tree";
            treeObj.content = "tree " + std::string(tree_oid_str).substr(0, 7);
            
            // Walk tree entries
            size_t entryCount = git_tree_entrycount(tree);
            for (size_t i = 0; i < entryCount; i++) {
                const git_tree_entry* entry = git_tree_entry_byindex(tree, i);
                if (!entry) continue;
                
                const char* entryName = git_tree_entry_name(entry);
                const git_oid* entryOid = git_tree_entry_id(entry);
                
                char entry_oid_str[GIT_OID_HEXSZ + 1];
                git_oid_tostr(entry_oid_str, sizeof(entry_oid_str), entryOid);
                
                treeObj.content += "\n" + std::string(entry_oid_str).substr(0, 7) + "  " + entryName;
                
                // Add blob child
                treeObj.children.push_back(entry_oid_str);
                
                // Create blob object
                GitObjectData blobObj;
                blobObj.hash = entry_oid_str;
                blobObj.type = "blob";
                
                // Try to read blob content
                git_blob* blob = nullptr;
                if (git_blob_lookup(&blob, repo, entryOid) == 0) {
                    size_t blobSize = git_blob_rawsize(blob);
                    const void* blobData = git_blob_rawcontent(blob);
                    
                    // Store first 200 chars or size info
                    if (blobData && blobSize > 0) {
                        size_t previewSize = blobSize > 200 ? 200 : blobSize;
                        blobObj.content = std::string((const char*)blobData, previewSize);
                        if (blobSize > 200) blobObj.content += "...";
                    } else {
                        blobObj.content = "<empty file>";
                    }
                    
                    git_blob_free(blob);
                } else {
                    blobObj.content = "<binary or unreadable>";
                }
                
                blobObj.content = "blob " + std::string(entry_oid_str).substr(0, 7) + "\n" + entryName + "\n\n" + blobObj.content;
                result.push_back(blobObj);
            }
            
            result.push_back(treeObj);
            git_tree_free(tree);
        }
        
        result.push_back(commitObj);
    }
    
    git_commit_free(commit);
    return result;
}
