#pragma once
#include "level_manager.h"
#include "../src/git_visualization.h"
#include "../src/git_wrapper.h"
#include <memory>
#include <string>

// Level 3: 合并危机 - 学习处理合并冲突
class Level03_Merge : public Level {
public:
    Level03_Merge();
    ~Level03_Merge();
    
    void Initialize() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Shutdown() override;
    
    bool IsComplete() const override;
    
private:
    enum class Stage {
        INTRO,              // 介绍合并冲突
        CREATE_CONFLICT,    // 在两个分支上修改同一文件
        ATTEMPT_MERGE,      // 尝试合并
        RESOLVE_CONFLICT,   // 解决冲突
        COMMIT_RESOLUTION,  // 提交解决方案
        COMPLETE            // 完成
    };
    
    Stage currentStage;
    float timer;
    bool stageComplete;
    
    // Git 相关
    std::unique_ptr<GitWrapper> git;
    std::string repoPath;
    
    // 可视化
    std::unique_ptr<GitVis::SplitGitView> splitView;
    
    // 合并状态
    bool conflictCreated;
    bool mergeAttempted;
    bool conflictResolved;
    
    // 方法
    void CreateRepoWithConflict();
    void SyncGraphWithRepo();
    void ProcessGitCommand(const std::string& cmd);
    void DrawStatusPanel();
    void DrawDialogueIfNeeded();
};
