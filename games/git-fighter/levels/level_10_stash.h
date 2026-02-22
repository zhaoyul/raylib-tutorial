#pragma once
#include "level_manager.h"
#include "../src/git_visualization.h"
#include "../src/git_wrapper.h"
#include <memory>
#include <string>
#include <vector>

// Level 10: Stash 战场 - 学习 stash 暂存工作区
class Level10_Stash : public Level {
public:
    Level10_Stash();
    ~Level10_Stash();
    
    void Initialize() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Shutdown() override;
    
    bool IsComplete() const override;
    
private:
    enum class Stage {
        INTRO,              // 介绍场景
        DEVELOPING,         // 正在开发新功能
        EMERGENCY_CALL,     // 紧急任务插入
        STASH_CHANGES,      // 暂存当前工作
        SWITCH_BRANCH,      // 切换到紧急分支
        FIX_EMERGENCY,      // 修复紧急问题
        POP_STASH,          // 恢复之前的工作
        COMPLETE            // 完成
    };
    
    Stage currentStage;
    float timer;
    bool stageComplete;
    
    std::unique_ptr<GitWrapper> git;
    std::string repoPath;
    std::unique_ptr<GitVis::SplitGitView> splitView;
    
    // Stash 状态
    bool hasStash;
    std::string stashMessage;
    int stashCount;
    
    // 文件状态
    bool hasUncommittedChanges;
    bool hasFeatureFile;
    bool hasEmergencyFix;
    
    void SetupRepo();
    void SyncGraphWithRepo();
    void ProcessGitCommand(const std::string& cmd);
    void DrawStatusPanel();
    void DrawDialogueIfNeeded();
    void DrawStashPanel();
    
    void CreateStash();
    void PopStash();
    void SwitchToEmergency();
    void ApplyEmergencyFix();
};
