#pragma once
#include "level_manager.h"
#include "../src/git_visualization.h"
#include "../src/git_wrapper.h"
#include <memory>
#include <string>

// Level 5: Rebase 变基危机 - 学习 rebase 和解决 rebase 冲突
class Level05_Rebase : public Level {
public:
    Level05_Rebase();
    ~Level05_Rebase();
    
    void Initialize() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Shutdown() override;
    
    bool IsComplete() const override;
    
    GitWrapper* GetGitWrapper() override { return git.get(); }
    std::string ProcessLevelCommand(const std::string& cmd) override;
    
private:
    enum class Stage {
        INTRO,              // 介绍场景：feature 分支落后 main
        SHOW_BRANCHES,      // 展示分叉的历史
        START_REBASE,       // 开始 rebase
        REBASE_CONFLICT,    // rebase 遇到冲突
        RESOLVE_CONFLICT,   // 解决冲突
        CONTINUE_REBASE,    // 继续 rebase
        REBASE_COMPLETE,    // rebase 完成
        COMPLETE            // 关卡完成
    };
    
    Stage currentStage;
    float timer;
    bool stageComplete;
    
    // Git
    std::unique_ptr<GitWrapper> git;
    std::string repoPath;
    
    // 可视化
    std::unique_ptr<GitVis::SplitGitView> splitView;
    
    // Rebase 状态
    bool rebaseStarted;
    bool conflictOccurred;
    bool conflictResolved;
    int conflictCount;          // 遇到的冲突数量
    int resolvedCount;          // 已解决的冲突数量
    
    // 动画
    float rebaseProgress;
    
    void CreateRebaseScenario();
    void StartRebase();
    void ResolveConflict();
    void ContinueRebase();
    void SyncGraphWithRepo();

    void DrawStatusPanel();
    void DrawDialogueIfNeeded();
};
