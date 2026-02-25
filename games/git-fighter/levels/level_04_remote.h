#pragma once
#include "level_manager.h"
#include "../src/git_visualization.h"
#include "../src/git_wrapper.h"
#include <memory>
#include <string>

// Level 4: 远程协作 - 学习 git remote, push, pull
class Level04_Remote : public Level {
public:
    Level04_Remote();
    ~Level04_Remote();

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
        INTRO,              // 介绍远程仓库
        ADD_REMOTE,         // 添加远程仓库
        PUSH_MAIN,          // 推送到远程
        FETCH_REMOTE,       // 获取远程更新
        PULL_CHANGES,       // 拉取并合并
        COMPLETE            // 完成
    };

    Stage currentStage;
    float timer;
    bool stageComplete;

    // Git 相关
    std::unique_ptr<GitWrapper> git;

    // 可视化
    std::unique_ptr<GitVis::SplitGitView> splitView;

    // 远程状态
    bool remoteAdded;
    bool pushed;
    bool fetched;
    bool pulled;
    
    // Remote 仓库路径
    std::string remotePath;
    
    // 远程 commit 模拟（其他人 push 的提交）
    std::vector<std::pair<std::string, std::string>> remoteCommits;  // hash, message
    bool remoteHasNewCommits;  // 标记 remote 是否有新提交

    // 方法
    void CreateLocalRepo();
    void CreateRemoteRepo();  // 创建远程仓库并添加一些提交
    void SyncGraphWithRepo();
    void SyncRemoteBranches();  // 同步远程分支到可视化

    void DrawStatusPanel();
    void DrawDialogueIfNeeded();
};
