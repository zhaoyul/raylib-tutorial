#include "git_wrapper.h"
#include <git2.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <filesystem>
#include <set>
#include <map>

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
    
    // Get current branch name for updating
    std::string update_ref = "HEAD";
    git_reference* head_ref = nullptr;
    if (git_repository_head(&head_ref, repo) == 0) {
        if (git_reference_type(head_ref) == GIT_REFERENCE_SYMBOLIC) {
            // HEAD points to a branch, use the branch name
            update_ref = git_reference_name(head_ref);
        }
        git_reference_free(head_ref);
    }
    
    // Create commit
    git_oid commit_oid;
    int error;
    
    if (have_parent) {
        git_commit* parent = nullptr;
        git_commit_lookup(&parent, repo, &parent_oid);
        const git_commit* parents[] = {parent};
        error = git_commit_create(&commit_oid, repo, update_ref.c_str(), sig, sig, 
                                   NULL, message.c_str(), tree, 1, parents);
        git_commit_free(parent);
    } else {
        // First commit, no parents
        error = git_commit_create(&commit_oid, repo, update_ref.c_str(), sig, sig,
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
    
    // Use system git to perform actual merge
    std::string cmd = "cd " + repoPath + " && git merge " + branchName + " 2>&1";
    
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        return {false, "", "Failed to execute merge command"};
    }
    
    std::string output;
    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        output += buffer;
    }
    
    int exitCode = pclose(pipe);
    
    // Update HEAD after merge
    UpdateHEAD();
    
    // Return appropriate result based on exit code
    if (exitCode == 0) {
        // Fast-forward or clean merge
        return {true, output.empty() ? "Merged branch '" + branchName + "'" : output, ""};
    } else if (output.find("conflict") != std::string::npos || 
               output.find("CONFLICT") != std::string::npos) {
        // Merge with conflicts - this is expected in some scenarios
        return {true, output, ""};
    } else {
        // Other error
        return {false, "", output.empty() ? "Merge failed" : output};
    }
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
    
    std::set<std::string> seenHashes;
    std::map<std::string, std::vector<std::string>> commitBranches;
    
    // First, collect all branch references
    git_reference_iterator* iter = nullptr;
    git_reference_iterator_new(&iter, repo);
    
    const char* refname = nullptr;
    git_reference* ref = nullptr;
    
    std::vector<std::string> allRefNames;
    while (git_reference_next_name(&refname, iter) == 0) {
        allRefNames.push_back(refname);
    }
    git_reference_iterator_free(iter);
    
    // Walk from all branches
    git_revwalk* walker = nullptr;
    git_revwalk_new(&walker, repo);
    git_revwalk_sorting(walker, GIT_SORT_TIME);
    
    // Push all branch heads
    for (const auto& name : allRefNames) {
        if (git_reference_lookup(&ref, repo, name.c_str()) == 0) {
            if (git_reference_type(ref) == GIT_REFERENCE_DIRECT) {
                const git_oid* oid = git_reference_target(ref);
                git_revwalk_push(walker, oid);
                
                // Record which branch points to which commit
                char oid_str[GIT_OID_HEXSZ + 1];
                git_oid_tostr(oid_str, sizeof(oid_str), oid);
                
                std::string branchName = name;
                // Strip refs/heads/ or refs/tags/ prefix
                if (branchName.find("refs/heads/") == 0) {
                    branchName = branchName.substr(11);
                } else if (branchName.find("refs/tags/") == 0) {
                    branchName = branchName.substr(10);
                }
                
                commitBranches[oid_str].push_back(branchName);
            }
            git_reference_free(ref);
        }
    }
    
    // Also push HEAD
    git_revwalk_push_head(walker);
    
    git_oid oid;
    int count = 0;
    while (git_revwalk_next(&oid, walker) == 0 && count < maxCommits) {
        char oid_str[GIT_OID_HEXSZ + 1];
        git_oid_tostr(oid_str, sizeof(oid_str), &oid);
        
        if (seenHashes.count(oid_str)) continue;
        seenHashes.insert(oid_str);
        
        git_commit* commit = nullptr;
        if (git_commit_lookup(&commit, repo, &oid) != 0) {
            continue;
        }
        
        CommitNode node;
        node.hash = oid_str;
        node.message = git_commit_message(commit) ? git_commit_message(commit) : "";
        
        const git_signature* author = git_commit_author(commit);
        if (author) {
            node.author = author->name ? author->name : "";
            node.timestamp = author->when.time;
        } else {
            node.timestamp = 0;
        }
        
        // Add branches pointing to this commit
        if (commitBranches.count(oid_str)) {
            node.branches = commitBranches[oid_str];
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

std::string GitWrapper::GetCurrentBranch() const {
    if (!repo) return "";
    
    git_reference* head_ref = nullptr;
    if (git_repository_head(&head_ref, repo) != 0) {
        return "";
    }
    
    const char* branch_name = git_reference_shorthand(head_ref);
    std::string result = branch_name ? branch_name : "";
    git_reference_free(head_ref);
    return result;
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
    
    // Set to track already processed objects (avoid duplicates)
    std::set<std::string> processed;
    
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
            
            // Recursively process tree
            ProcessTree(tree_oid_str, tree, "", result, processed);
            
            git_tree_free(tree);
        }
        
        result.push_back(commitObj);
    }
    
    git_commit_free(commit);
    return result;
}

void GitWrapper::ProcessTree(const std::string& treeHash, git_tree* tree, 
                             const std::string& path,
                             std::vector<GitObjectData>& result,
                             std::set<std::string>& processed) {
    if (processed.count(treeHash)) return;
    processed.insert(treeHash);
    
    GitObjectData treeObj;
    treeObj.hash = treeHash;
    treeObj.type = "tree";
    treeObj.content = "tree " + treeHash.substr(0, 7);
    if (!path.empty()) {
        treeObj.content += "\n" + path + "/";
    }
    
    // Walk tree entries
    size_t entryCount = git_tree_entrycount(tree);
    for (size_t i = 0; i < entryCount; i++) {
        const git_tree_entry* entry = git_tree_entry_byindex(tree, i);
        if (!entry) continue;
        
        const char* entryName = git_tree_entry_name(entry);
        const git_oid* entryOid = git_tree_entry_id(entry);
        git_object_t entryType = git_tree_entry_type(entry);
        
        char entry_oid_str[GIT_OID_HEXSZ + 1];
        git_oid_tostr(entry_oid_str, sizeof(entry_oid_str), entryOid);
        
        std::string entryPath = path.empty() ? entryName : path + "/" + entryName;
        treeObj.content += "\n" + std::string(entry_oid_str).substr(0, 7) + "  " + entryName;
        
        // Add child
        treeObj.children.push_back(entry_oid_str);
        
        if (entryType == GIT_OBJECT_TREE) {
            // Subdirectory - recursively process
            git_tree* subTree = nullptr;
            if (git_tree_lookup(&subTree, repo, entryOid) == 0) {
                ProcessTree(entry_oid_str, subTree, entryPath, result, processed);
                git_tree_free(subTree);
            }
        } else {
            // Blob (file) - avoid duplicates
            if (!processed.count(entry_oid_str)) {
                processed.insert(entry_oid_str);
                
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
                
                blobObj.content = "blob " + std::string(entry_oid_str).substr(0, 7) + "\n" + entryPath + "\n\n" + blobObj.content;
                result.push_back(blobObj);
            }
        }
    }
    
    result.push_back(treeObj);
}

std::vector<GitWrapper::GitObjectData> GitWrapper::GetAllGitObjects(int maxObjects) {
    std::vector<GitObjectData> result;
    if (!repo) return result;
    
    std::set<std::string> processed;
    int count = 0;
    
    // 1. Add all refs (branches, tags, HEAD)
    git_reference_iterator* iter = nullptr;
    git_reference_iterator_new(&iter, repo);
    
    const char* refname = nullptr;
    while (git_reference_next_name(&refname, iter) == 0 && count < maxObjects) {
        git_reference* ref = nullptr;
        if (git_reference_lookup(&ref, repo, refname) != 0) continue;
        
        if (git_reference_type(ref) == GIT_REFERENCE_DIRECT) {
            const git_oid* oid = git_reference_target(ref);
            char oid_str[GIT_OID_HEXSZ + 1];
            git_oid_tostr(oid_str, sizeof(oid_str), oid);
            
            GitObjectData refObj;
            refObj.hash = std::string("ref:") + refname;
            refObj.type = "ref";
            refObj.content = std::string(refname) + "\n-> " + std::string(oid_str).substr(0, 7);
            refObj.children.push_back(oid_str);
            result.push_back(refObj);
            
            // Mark the target for processing
            processed.insert(oid_str);
        }
        git_reference_free(ref);
        count++;
    }
    git_reference_iterator_free(iter);
    
    // 2. Walk all commits from HEAD and all refs
    git_revwalk* walker = nullptr;
    git_revwalk_new(&walker, repo);
    git_revwalk_push_head(walker);
    
    // Also push all refs
    git_reference_iterator_new(&iter, repo);
    while (git_reference_next_name(&refname, iter) == 0) {
        git_reference* ref = nullptr;
        if (git_reference_lookup(&ref, repo, refname) == 0) {
            if (git_reference_type(ref) == GIT_REFERENCE_DIRECT) {
                git_revwalk_push(walker, git_reference_target(ref));
            }
            git_reference_free(ref);
        }
    }
    git_reference_iterator_free(iter);
    
    git_oid oid;
    while (git_revwalk_next(&oid, walker) == 0 && count < maxObjects) {
        char oid_str[GIT_OID_HEXSZ + 1];
        git_oid_tostr(oid_str, sizeof(oid_str), &oid);
        
        if (processed.count(oid_str)) continue;
        processed.insert(oid_str);
        
        git_commit* commit = nullptr;
        if (git_commit_lookup(&commit, repo, &oid) != 0) continue;
        
        GitObjectData commitObj;
        commitObj.hash = oid_str;
        commitObj.type = "commit";
        
        const char* message = git_commit_message(commit);
        const git_signature* author = git_commit_author(commit);
        commitObj.content = std::string("commit ") + oid_str + "\n" +
                           "Author: " + (author ? author->name : "Unknown") + "\n" +
                           "\n" + (message ? message : "");
        
        // Add tree as child
        git_tree* tree = nullptr;
        if (git_commit_tree(&tree, commit) == 0) {
            char tree_oid[GIT_OID_HEXSZ + 1];
            git_oid_tostr(tree_oid, sizeof(tree_oid), git_tree_id(tree));
            commitObj.children.push_back(tree_oid);
            
            // Process tree if not already done
            if (!processed.count(tree_oid) && count < maxObjects) {
                ProcessTree(tree_oid, tree, "", result, processed);
                count = result.size();
            }
            git_tree_free(tree);
        }
        
        // Add parents
        unsigned int parentCount = git_commit_parentcount(commit);
        for (unsigned int i = 0; i < parentCount; i++) {
            const git_oid* parentOid = git_commit_parent_id(commit, i);
            if (parentOid) {
                char parent_oid[GIT_OID_HEXSZ + 1];
                git_oid_tostr(parent_oid, sizeof(parent_oid), parentOid);
                // Parents are not "children" in the tree sense, but we track them
            }
        }
        
        result.push_back(commitObj);
        count++;
    }
    
    git_revwalk_free(walker);
    return result;
}

// ===== Reflog Operations (Level 8) =====

std::vector<GitWrapper::ReflogEntry> GitWrapper::GetReflog(const std::string& ref) {
    std::vector<ReflogEntry> result;
    if (!repo) return result;
    
    git_reflog* reflog = nullptr;
    if (git_reflog_read(&reflog, repo, ref.c_str()) != 0) {
        return result;
    }
    
    size_t count = git_reflog_entrycount(reflog);
    for (size_t i = 0; i < count; i++) {
        const git_reflog_entry* entry = git_reflog_entry_byindex(reflog, i);
        if (!entry) continue;
        
        ReflogEntry re;
        re.index = static_cast<int>(i);
        
        // Current hash (after action)
        const git_oid* oid = git_reflog_entry_id_new(entry);
        if (oid) {
            char oid_str[GIT_OID_HEXSZ + 1];
            git_oid_tostr(oid_str, sizeof(oid_str), oid);
            re.hash = oid_str;
        }
        
        // Old hash (before action)
        const git_oid* old_oid = git_reflog_entry_id_old(entry);
        if (old_oid) {
            char old_oid_str[GIT_OID_HEXSZ + 1];
            git_oid_tostr(old_oid_str, sizeof(old_oid_str), old_oid);
            re.oldHash = old_oid_str;
        }
        
        // Action message
        const char* msg = git_reflog_entry_message(entry);
        re.message = msg ? msg : "";
        
        // Parse action from message (e.g., "commit:", "reset:", "checkout:")
        size_t colonPos = re.message.find(':');
        if (colonPos != std::string::npos) {
            re.action = re.message.substr(0, colonPos);
            re.message = re.message.substr(colonPos + 1);
        } else {
            re.action = "unknown";
        }
        
        // Author and timestamp
        const git_signature* sig = git_reflog_entry_committer(entry);
        if (sig) {
            re.author = sig->name ? sig->name : "";
            re.timestamp = sig->when.time;
        }
        
        result.push_back(re);
    }
    
    git_reflog_free(reflog);
    return result;
}

GitResult GitWrapper::ResetHard(const std::string& target) {
    if (!repo) return {false, "", "Not a git repository"};
    
    // Parse target (can be hash, branch name, or reflog entry like HEAD@{1})
    git_object* targetObj = nullptr;
    if (git_revparse_single(&targetObj, repo, target.c_str()) != 0) {
        return {false, "", "Invalid target: " + target};
    }
    
    git_checkout_options opts = GIT_CHECKOUT_OPTIONS_INIT;
    opts.checkout_strategy = GIT_CHECKOUT_FORCE;
    
    int error = git_reset(repo, targetObj, GIT_RESET_HARD, &opts);
    git_object_free(targetObj);
    
    if (error != 0) {
        const git_error* e = git_error_last();
        return {false, "", e ? e->message : "Reset failed"};
    }
    
    UpdateHEAD();
    NotifyStatusChange();
    return {true, "HEAD is now at " + GetHEAD().substr(0, 7), ""};
}

GitResult GitWrapper::CherryPick(const std::string& commitHash) {
    if (!repo) return {false, "", "Not a git repository"};
    
    // For simplicity in this game, we'll use git command
    std::string cmd = "cd " + repoPath + " && git cherry-pick " + commitHash;
    int result = std::system(cmd.c_str());
    
    if (result != 0) {
        // Abort cherry-pick on failure
        std::string abortCmd = "cd " + repoPath + " && git cherry-pick --abort";
        std::system(abortCmd.c_str());
        return {false, "", "Cherry-pick failed (conflicts may exist)"};
    }
    
    UpdateHEAD();
    NotifyStatusChange();
    return {true, "Cherry-picked " + commitHash.substr(0, 7), ""};
}

GitResult GitWrapper::CreateBranchAt(const std::string& branchName, const std::string& target) {
    if (!repo) return {false, "", "Not a git repository"};
    
    // Parse target
    git_object* targetObj = nullptr;
    if (git_revparse_single(&targetObj, repo, target.c_str()) != 0) {
        return {false, "", "Invalid target: " + target};
    }
    
    const git_oid* targetOid = git_object_id(targetObj);
    
    // Create branch
    git_reference* branchRef = nullptr;
    int error = git_branch_create(&branchRef, repo, branchName.c_str(), 
                                   (git_commit*)targetObj, 0);
    git_object_free(targetObj);
    
    if (error != 0) {
        const git_error* e = git_error_last();
        return {false, "", e ? e->message : "Failed to create branch"};
    }
    
    git_reference_free(branchRef);
    return {true, "Created branch '" + branchName + "' at " + target, ""};
}
