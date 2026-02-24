#pragma once
#include "level_manager.h"
#include "../src/git_visualization.h"
#include "../src/git_wrapper.h"
#include <memory>
#include <string>
#include <vector>

// Level 9: Interactive Rebase 历史重写 - 整理提交历史
class Level09_Interactive : public Level {
public:
    Level09_Interactive();
    ~Level09_Interactive();
    
    void Initialize() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Shutdown() override;
    
    bool IsComplete() const override;
    
    GitWrapper* GetGitWrapper() override { return git.get(); }
    std::string ProcessLevelCommand(const std::string& cmd) override;
    void RefreshWorkingDirectory() override;
    
private:
    enum class Action {
        PICK,       // 保留
        REWORD,     // 修改消息
        SQUASH,     // 合并到上一个
        DROP        // 删除
    };
    
    struct CommitPlan {
        std::string hash;
        std::string message;
        Action action;
        bool isCurrent;  // 是否是当前提交（不能 squash 第一个）
    };
    
    enum class Stage {
        INTRO,              // 介绍混乱的历史
        SHOW_HISTORY,       // 展示需要整理的历史
        PLAN_REBASE,        // 规划 rebase 操作
        EXECUTE_REBASE,     // 执行 rebase
        VERIFY_RESULT,      // 验证结果
        COMPLETE            // 完成
    };
    
    Stage currentStage;
    float timer;
    bool stageComplete;
    
    std::unique_ptr<GitWrapper> git;
    std::unique_ptr<GitVis::SplitGitView> splitView;
    
    // 提交计划
    std::vector<CommitPlan> commitPlans;
    int selectedPlanIndex;
    
    // 统计
    int originalCount;
    int finalCount;
    int squashedCount;
    int droppedCount;
    
    void CreateMessyHistory();
    void SyncGraphWithRepo();
    void GenerateCommitPlan();
    void ExecuteRebase();
    void CycleAction(int index);
    void DrawStatusPanel();
    void DrawDialogueIfNeeded();
    void DrawInteractivePanel();

    
    const char* ActionToString(Action a);
    Color ActionToColor(Action a);
};
