#pragma once
#include "level_manager.h"
#include "../src/git_visualization.h"
#include "../src/git_wrapper.h"
#include <memory>
#include <string>

// Level 2: 分支探险 - 学习 git branch 和 git checkout
class Level02_Branch : public Level {
public:
    Level02_Branch();
    ~Level02_Branch();
    
    void Initialize() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Shutdown() override;
    
    bool IsComplete() const override;
    
private:
    enum class Stage {
        INTRO,              // 介绍分支概念
        CREATE_BRANCH,      // 等待创建分支
        SWITCH_BRANCH,      // 等待切换分支
        MAKE_CHANGES,       // 在新分支上修改
        COMMIT_FEATURE,     // 提交新功能
        MERGE_BRANCH,       // 合并分支
        COMPLETE            // 完成
    };
    
    Stage currentStage;
    float timer;
    bool stageComplete;
    
    // Git 相关
    std::unique_ptr<GitWrapper> git;
    std::string repoPath;
    std::string lastCommitHash;
    
    // 可视化
    std::unique_ptr<GitVis::SplitGitView> splitView;
    
    // 分支状态
    std::string currentBranch;
    std::string featureBranchName;
    bool featureBranchCreated;
    bool changesCommitted;
    bool merged;
    
    // 方法
    void CreateInitialRepo();
    void SyncGraphWithRepo();
    void ProcessGitCommand(const std::string& cmd);
    void DrawCommandInput();
    void DrawStatusPanel();
    void DrawDialogueIfNeeded();
    void CheckGitStatus();
};
