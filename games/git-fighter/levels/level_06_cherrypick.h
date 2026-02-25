#pragma once
#include "level_manager.h"
#include "../src/git_visualization.h"
#include "../src/git_wrapper.h"
#include <memory>
#include <string>
#include <vector>

// Level 6: Cherry-pick 紧急修复 - 挑选特定 commit 到 release 分支
class Level06_CherryPick : public Level {
public:
    Level06_CherryPick();
    ~Level06_CherryPick();

    void Initialize() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Shutdown() override;

    bool IsComplete() const override;

    GitWrapper* GetGitWrapper() override { return git.get(); }
    std::string ProcessLevelCommand(const std::string& cmd) override;
    void RefreshWorkingDirectory() override;

private:
    enum class Stage {
        INTRO,              // 生产环境出 bug 了！
        SHOW_COMMITS,       // 查看各分支的提交
        SELECT_COMMITS,     // 选择要 cherry-pick 的 commit
        PICKING,            // 执行 cherry-pick
        HANDLE_CONFLICT,    // 处理可能的冲突
        VERIFY_FIX,         // 验证修复
        COMPLETE            // 完成
    };

    Stage currentStage;
    float timer;
    bool stageComplete;

    std::unique_ptr<GitWrapper> git;
    std::unique_ptr<GitVis::SplitGitView> splitView;

    // Cherry-pick 状态
    std::vector<std::string> availableFixes;    // 可选的修复 commits
    std::vector<std::string> selectedFixes;     // 已选择的
    std::vector<std::string> pickedFixes;       // 已成功的
    int currentPickIndex;
    bool hasConflict;

    // 可视化
    std::string hoveredCommit;

    void CreateProductionScenario();
    void SyncGraphWithRepo();

    void DrawStatusPanel();
    void DrawDialogueIfNeeded();
    void DrawCommitSelector();
};
