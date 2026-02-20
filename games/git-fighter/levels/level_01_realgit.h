#pragma once
#include "level_manager.h"
#include "../src/git_visualization.h"
#include "../src/git_wrapper.h"
#include <memory>
#include <string>

// Level 1: 使用真实 Git 操作的可视化教学
class Level01_RealGit : public Level {
public:
    Level01_RealGit();
    ~Level01_RealGit();
    
    void Initialize() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Shutdown() override;
    
    bool IsComplete() const override;
    
private:
    enum class Stage {
        INTRO,              // CTO 对话
        SHOW_WORKSPACE,     // 显示工作区
        WAIT_INIT,          // 等待 git init
        WAIT_ADD,           // 等待 git add
        WAIT_COMMIT,        // 等待 git commit
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
    
    // 文件创建状态
    bool readmeCreated;
    bool mainCppCreated;
    bool filesAdded;
    
    // 方法
    void CreateSampleFiles();
    void SyncGraphWithRepo();
    void ProcessGitCommand(const std::string& cmd);
    void DrawCommandInput();
    void DrawStatusPanel();
    void DrawDialogueIfNeeded();
    
    // 检查 Git 状态
    void CheckGitStatus();
};
