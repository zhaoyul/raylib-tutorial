#include "git_wrapper.h"
#include <git2.h>
#include <fstream>
#include <iostream>
#include <filesystem>

GitWrapper::GitWrapper() : repo(nullptr) {
    git_libgit2_init();
}

GitWrapper::~GitWrapper() {
    CloseRepo();
    git_libgit2_shutdown();
}

bool GitWrapper::InitRepo(const std::string& path) {
    CloseRepo();
    
    git_repository* newRepo = nullptr;
    int error = git_repository_init(&newRepo, path.c_str(), 0);
    
    if (error < 0) {
        const git_error* e = git_error_last();
        std::cerr << "Git init failed: " << (e ? e->message : "unknown error") << std::endl;
        return false;
    }
    
    repo = newRepo;
    repoPath = path;
    return true;
}

bool GitWrapper::OpenRepo(const std::string& path) {
    CloseRepo();
    
    git_repository* newRepo = nullptr;
    int error = git_repository_open(&newRepo, path.c_str());
    
    if (error < 0) {
        return false;
    }
    
    repo = newRepo;
    repoPath = path;
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
    
    // Simplified status
    return {true, "On branch main\nNothing to commit, working tree clean", ""};
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

std::vector<FileStatus> GitWrapper::GetWorkingDirectoryStatus() {
    std::vector<FileStatus> result;
    if (!repo) return result;
    
    git_status_options opts = GIT_STATUS_OPTIONS_INIT;
    opts.show = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
    opts.flags = GIT_STATUS_OPT_INCLUDE_UNTRACKED;
    
    git_status_list* list = nullptr;
    git_status_list_new(&list, repo, &opts);
    
    size_t count = git_status_list_entrycount(list);
    for (size_t i = 0; i < count; i++) {
        const git_status_entry* entry = git_status_byindex(list, i);
        FileStatus fs;
        
        if (entry->head_to_index) {
            fs.path = entry->head_to_index->old_file.path;
            fs.status = FileStatus::STAGED;
        } else if (entry->index_to_workdir) {
            fs.path = entry->index_to_workdir->old_file.path;
            fs.status = FileStatus::MODIFIED;
        }
        
        result.push_back(fs);
    }
    
    git_status_list_free(list);
    return result;
}

std::vector<CommitNode> GitWrapper::GetCommitGraph(int maxCommits) {
    std::vector<CommitNode> result;
    if (!repo) {
        std::cout << "GetCommitGraph: No repo open" << std::endl;
        return result;
    }
    
    std::string head = GetHEAD();
    std::cout << "GetCommitGraph: HEAD = " << head << std::endl;
    
    if (head.empty()) {
        std::cout << "GetCommitGraph: No HEAD, empty repo" << std::endl;
        return result;
    }
    
    // Walk commit history
    git_revwalk* walker = nullptr;
    if (git_revwalk_new(&walker, repo) != 0) {
        std::cout << "GetCommitGraph: Failed to create walker" << std::endl;
        return result;
    }
    
    if (git_revwalk_push_head(walker) != 0) {
        std::cout << "GetCommitGraph: Failed to push HEAD" << std::endl;
        git_revwalk_free(walker);
        return result;
    }
    
    git_revwalk_sorting(walker, GIT_SORT_TIME);
    
    git_oid oid;
    int count = 0;
    while (git_revwalk_next(&oid, walker) == 0 && count < maxCommits) {
        git_commit* commit = nullptr;
        if (git_commit_lookup(&commit, repo, &oid) != 0) {
            std::cout << "GetCommitGraph: Failed to lookup commit" << std::endl;
            continue;
        }
        
        CommitNode node;
        char oid_str[GIT_OID_HEXSZ + 1];
        git_oid_tostr(oid_str, sizeof(oid_str), &oid);
        node.hash = oid_str;
        
        const char* msg = git_commit_message(commit);
        node.message = msg ? msg : "";
        
        const git_signature* author = git_commit_author(commit);
        node.author = author ? author->name : "Unknown";
        
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
        
        std::cout << "GetCommitGraph: Found commit " << node.shortHash() << " - " << node.message.substr(0, 30) << std::endl;
        result.push_back(node);
        git_commit_free(commit);
        count++;
    }
    
    git_revwalk_free(walker);
    std::cout << "GetCommitGraph: Total " << result.size() << " commits" << std::endl;
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
                           "Author: " + author->name + " <" + author->email + ">\n" +
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
                git_object_t entryType = git_tree_entry_type(entry);
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
