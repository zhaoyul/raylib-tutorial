#pragma once
#include "level_manager.h"
#include "../src/git_visualization.h"
#include "../src/git_wrapper.h"
#include <memory>
#include <string>
#include <vector>

// Level 8: Reflog 时光回溯 - 学习用 reflog 恢复丢失的代码
class Level08_Reflog : public Level {
public:
    Level08_Reflog();
    ~Level08_Reflog();
    
    void Initialize() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Shutdown() override;
    
    bool IsComplete() const override;
    
    GitWrapper* GetGitWrapper() override { return git.get(); }
    std::string ProcessLevelCommand(const std::string& cmd) override;
    
private:
    enum class Stage {
        INTRO,              // 介绍剧情：误操作警告
        SHOW_HISTORY,       // 展示正常开发历史
        ACCIDENT_HAPPENS,   // 执行 reset --hard，代码丢失
        PANIC_MODE,         // 发现代码丢失，进入紧急模式
        SHOW_REFLOG,        // 显示 reflog，找到丢失的 commit
        RECOVERY_DECISION,  // 选择恢复方式：reset 或 cherry-pick
        RECOVER_CODE,       // 执行恢复操作
        VERIFY_RECOVERY,    // 验证代码已恢复
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
    
    // Reflog 相关状态
    std::vector<GitWrapper::ReflogEntry> reflogEntries;
    int selectedReflogIndex;
    std::string lostCommits[3];     // 3个丢失的 commit hash
    std::string recoveryMethod;     // "reset" or "cherry-pick"
    
    // 事故状态
    bool accidentTriggered;
    bool reflogVisible;
    bool codeRecovered;
    
    // 动画
    float panicPulse;
    int currentReflogPage;
    
    // 方法
    void CreateNormalRepo();
    void TriggerAccident();         // 执行 reset --hard
    void LoadReflog();
    void SyncGraphWithRepo();

    void DrawStatusPanel();
    void DrawDialogueIfNeeded();
    void DrawReflogPanel();         // 专门绘制 reflog 界面
    void DrawPanicOverlay();        // 紧急模式视觉效果
};
