#pragma once
#include "level_manager.h"
#include "../src/git_visualization.h"
#include "../src/git_wrapper.h"
#include <memory>
#include <string>
#include <vector>

// Level 7: Bisect 故障定位 - 用二分查找找到引入 bug 的 commit
class Level07_Bisect : public Level {
public:
    Level07_Bisect();
    ~Level07_Bisect();

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
        INTRO,              // 介绍 bug 报告
        START_BISECT,       // 开始 bisect
        TESTING_COMMIT,     // 测试当前 commit
        MARK_GOOD,          // 标记为好
        MARK_BAD,           // 标记为坏
        FOUND_CULPRIT,      // 找到罪魁祸首
        COMPLETE            // 完成
    };

    Stage currentStage;
    float timer;
    bool stageComplete;

    std::unique_ptr<GitWrapper> git;
    std::unique_ptr<GitVis::SplitGitView> splitView;

    // Bisect 状态
    std::vector<std::string> allCommits;        // 所有 commit hashes
    std::vector<bool> commitStatus;             // true=good, false=bad, undefined=unknown
    int goodIndex;                              // 已知好的 commit 索引
    int badIndex;                               // 已知坏的 commit 索引
    int currentTestIndex;                       // 当前正在测试的索引
    int bugCommitIndex;                         // 真正引入 bug 的 commit 索引
    int stepsTaken;                             // 已用步数
    int maxSteps;                               // 最大步数（二分查找最优）

    // 模拟测试结果
    bool currentTestResult;

    void CreateBuggyHistory();
    void SyncGraphWithRepo();
    void StartBisect();
    void TestCurrentCommit();
    void MarkGood();
    void MarkBad();
    void NextBisectStep();
    void DrawStatusPanel();
    void DrawDialogueIfNeeded();
    void DrawBisectPanel();
    void DrawCommitRange();


    int CalculateNextIndex();
};
