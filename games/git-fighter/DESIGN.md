# Git Fighter - 救火架构师 游戏设计文档

## 1. 游戏概述

### 1.1 游戏名称
**Git Fighter: 救火架构师** (The Firefighter Architect)

### 1.2 核心理念
通过沉浸式游戏化体验，让玩家在解决"代码危机"的过程中，直观理解 Git 的内部工作原理。游戏将抽象的 Git 概念（blob、tree、commit、branch）具象化为可视化的游戏元素。

### 1.3 目标受众
- 编程初学者，希望可视化理解 Git
- 有一定经验但想深入理解 Git 内部机制的开发者
- 需要快速上手 Git 的团队新人

### 1.4 平台
- Desktop (macOS/Windows/Linux)
- 使用 raylib 5.0 + libgit2 开发

---

## 2. 游戏世界观

### 2.1 背景设定
玩家扮演一位刚刚入职"福报科技"的架构师。公司正面临严重的代码管理危机，CTO（首席技术官）是你的导师，带你通过解决各种 Git 相关的紧急任务来拯救项目。

### 2.2 角色设定
- **玩家**：救火架构师，刚入职的新人
- **CTO**：资深导师，通过对话给出任务和指导
- **项目**：命运多舛的"电商平台"项目

### 2.3 关卡主题
每个关卡都对应一个真实的开发场景：
1. 周末加班 - 项目初始化
2. 分支探险 - 功能开发
3. 合并危机 - 解决冲突
4. 远程协作 - 团队协作

---

## 3. 核心玩法

### 3.1 双视图系统
游戏界面采用左右分栏设计：

```
┌─────────────────────────────────────────────────┐
│  Level Info    │   Commit History (提交图)       │
│  + Commands    │   [可视化提交历史]              │
│                ├─────────────────────────────────┤
│                │   Internal Structure (内部结构) │
│                │   [blob/tree/commit 对象]       │
└─────────────────────────────────────────────────┘
```

#### 3.1.1 提交图 (Commit Graph)
- **功能**：可视化提交历史 DAG
- **元素**：
  - 圆圈 = Commit 节点
  - 曲线 = Parent 关系
  - 标签 = 分支名 (main, feature/*)
  - 高亮 = HEAD 位置

#### 3.1.2 内部结构 (Internal Structure)
- **功能**：展示 Git 对象模型
- **元素**：
  - 蓝色矩形 = Tree (目录)
  - 绿色圆形 = Blob (文件)
  - 红色圆形 = Commit
  - 连线 = 对象引用关系
  - +/- 按钮 = 展开/收起目录

### 3.2 交互方式
| 操作 | 功能 |
|-----|------|
| I | git init |
| A | git add . |
| C | git commit |
| 1 | [调试] 创建随机文件 |
| 2 | [调试] 创建随机目录 |
| 3 | [调试] 追加文件内容 |
| 鼠标拖拽 | 平移视图 |
| 滚轮 | 缩放视图 |
| 点击节点 | 查看对象详情 |
| 点击 +/- | 展开/收起目录 |

---

## 4. 关卡设计

### Level 1: 周末加班
**主题**：Git 基础操作

#### 4.1.1 故事背景
周五晚上，CTO 紧急召唤："项目还在用 U 盘传代码！今晚必须搞定 Git！"

#### 4.1.2 学习目标
- 理解 Git 仓库初始化
- 理解工作区、暂存区、版本库
- 掌握 init/add/commit 基本流程
- 认识 blob、tree、commit 对象

#### 4.1.3 游戏流程
1. **WAIT_INIT**: 按 I 初始化仓库
   - 内部结构显示工作目录文件
2. **WAIT_ADD**: 按 A 添加文件到暂存区
   - 观察文件从"未跟踪"变"已暂存"
3. **WAIT_COMMIT**: 按 C 提交
   - 观察 commit 对象生成
   - 内部结构显示 commit → tree → blobs 链条

#### 4.1.4 可视化要点
- init 前：内部结构为空
- init 后：显示工作目录文件（未跟踪状态）
- add 后：文件进入暂存区（颜色变化）
- commit 后：显示完整的对象图

---

### Level 2: 分支探险
**主题**：Branch & Checkout

#### 4.2.1 故事背景
"需要开发登录功能，但不能影响主分支！"

#### 4.2.2 学习目标
- 理解分支的本质（指向 commit 的指针）
- 掌握 branch/checkout 操作
- 理解 HEAD 的作用
- 认识分离头指针状态

#### 4.2.3 游戏流程
1. **CREATE_BRANCH**: 创建 feature/login 分支
   - 提交图显示新分支标签
2. **SWITCH_BRANCH**: 切换到 feature 分支
   - HEAD 移动到 feature
   - 工作目录显示可以修改
3. **MAKE_CHANGES**: 创建 login.cpp
   - 内部结构显示新文件
4. **COMMIT_FEATURE**: 提交修改
   - feature 分支向前推进
   - main 分支保持不变

#### 4.2.4 可视化要点
- 分支创建：新标签指向当前 commit
- checkout：HEAD 标签移动
- commit 后：分支线分叉

---

### Level 3: 合并危机
**主题**：Merge & Conflict Resolution

#### 4.3.1 故事背景
"main 和 feature 都改了 config.txt！合并冲突了！"

#### 4.3.2 学习目标
- 理解合并的本质（找到共同祖先）
- 掌握冲突解决方法
- 理解 fast-forward vs 3-way merge

#### 4.3.3 游戏流程
1. 预设场景：
   - main 分支：config.txt timeout=45
   - feature 分支：config.txt timeout=60
2. **ATTEMPT_MERGE**: 尝试合并
   - 冲突！显示冲突标记
3. **RESOLVE_CONFLICT**: 解决冲突
   - 手动编辑（模拟）
   - add 标记为已解决
4. **COMMIT_RESOLUTION**: 提交合并结果
   - 生成 merge commit（有两个 parent）

#### 4.3.4 可视化要点
- 合并前：两条分支线
- 冲突时：显示冲突文件高亮
- 合并后：commit 有两个父节点

---

### Level 4: 远程协作
**主题**：Remote, Push, Pull

#### 4.4.1 故事背景
"需要团队协作！连接远程仓库！"

#### 4.4.2 学习目标
- 理解远程仓库概念
- 掌握 remote/push/pull/fetch
- 理解本地分支和远程分支的区别

#### 4.4.3 游戏流程
1. **ADD_REMOTE**: 添加 origin
2. **PUSH_MAIN**: 推送本地提交到远程
   - 显示 origin/main 分支
3. **FETCH_REMOTE**: 获取远程更新
   - 模拟远程有新提交
4. **PULL_CHANGES**: 拉取并合并
   - fast-forward 或 merge

#### 4.4.4 可视化要点
- origin/main 用不同颜色显示
- push 后：本地和远程分支对齐
- fetch 后：origin/main 领先
- pull 后：本地分支追上

---

## 5. 技术实现

### 5.1 架构图
```
┌─────────────────────────────────────┐
│           GitGame (Main)            │
├─────────────────────────────────────┤
│  LevelManager  │  UIManager         │
├────────────────┼────────────────────┤
│  Level 1-4     │  SplitGitView      │
│  (关卡逻辑)     │  ├─ CommitGraphPanel
│                │  └─ InternalStructurePanel
├────────────────┴────────────────────┤
│           GitWrapper                │
│  (libgit2 封装)                      │
└─────────────────────────────────────┘
```

### 5.2 核心类

#### GitWrapper
- **职责**：封装 libgit2 操作
- **方法**：
  - `Init()`, `Add()`, `Commit()`
  - `CreateBranch()`, `Checkout()`
  - `GetCommitGraph()`, `GetCommitObjects()`
  - `CreateRandomFile()`, `ScanWorkingDirectory()`

#### SplitGitView
- **职责**：管理双视图布局
- **组成**：
  - `CommitGraphPanel`：提交历史可视化
  - `InternalStructurePanel`：对象结构可视化

#### InternalStructurePanel
- **职责**：树形布局 + 交互
- **算法**：BFS 层级布局
- **交互**：
  - +/- 按钮展开/收起
  - 拖拽平移
  - 滚轮缩放

### 5.3 渲染流程
```
BeginTextureMode(renderTarget)  // 离屏渲染
  ClearBackground()
  
  // 关卡绘制
  level->Draw()
  
  // 双视图绘制
  splitView->Draw()
    ├─ commitPanel.Draw()      // 提交图
    └─ structurePanel.Draw()   // 内部结构
      
  // HUD 绘制
  DrawHUD()
  
EndTextureMode()

// 缩放到窗口
DrawTexturePro(renderTarget, ...)
```

---

## 6. 教育设计

### 6.1 渐进式学习
| 关卡 | 概念 | 复杂度 |
|-----|------|--------|
| 1 | init/add/commit, blob/tree | ⭐⭐ |
| 2 | branch/checkout, HEAD | ⭐⭐⭐ |
| 3 | merge/conflict, 3-way merge | ⭐⭐⭐⭐ |
| 4 | remote/push/pull, distributed | ⭐⭐⭐⭐⭐ |

### 6.2 可视化教学
- **即时反馈**：每个操作后，双视图立即更新
- **对比学习**：工作区 vs 暂存区 vs 版本库
- **历史回放**：可以查看任意 commit 的对象结构

### 6.3 调试工具
- F5/F6/F7（或 1/2/3）：随机生成文件/目录/内容
- 用于测试内部结构面板的展开/收起功能
- 帮助理解 Git 如何追踪文件变化

---

## 7. 美术风格

### 7.1 色彩方案
```
背景：#1E222A (深色编辑器风格)

Commit 节点：
- 普通：#5C8DFF (蓝色)
- HEAD：#FF6B6B (红色)
- 选中：#FFD93D (黄色)

对象类型：
- Commit：#FF6B6B (红)
- Tree：#5C8DFF (蓝)
- Blob：#6BCB77 (绿)

分支：
- main：#5C8DFF
- feature/*：#FFD93D
- origin/*：#FF9F45
```

### 7.2 UI 风格
- 扁平化设计
- 圆角矩形 (圆角半径 0.3)
- 微妙的阴影和发光效果
- 等宽字体显示代码/哈希

---

## 8. 扩展计划

### 8.1 未来关卡
- **Level 5**: 历史回滚 (reset/revert/reflog)
- **Level 6**: 紧急修复 (stash/cherry-pick)
- **Level 7**: 标签管理 (tag)
- **Level 8**: 子模块 (submodule)

### 8.2 多人模式
- 模拟多人协作场景
- 冲突解决对战模式

### 8.3 沙盒模式
- 完全自由的 Git 操作环境
- 可以导入真实项目练习

---

## 9. 开发日志

### v1.0 功能清单
- [x] 4 个完整关卡
- [x] 双视图可视化系统
- [x] 真实的 Git 操作（libgit2）
- [x] 中文界面支持
- [x] 关卡完成手动 Next 按钮
- [x] 内部结构展开/收起
- [x] 随机内容生成（调试用）

### 已知问题
- [ ] 复杂目录结构布局可能需要优化
- [ ] 需要更多 CTO 对话内容

---

## 10. 参考资料

### 10.1 Git 内部原理
- Pro Git Book - Git Internals
- libgit2 文档

### 10.2 游戏设计
- 可视化工具灵感：GitKraken, SourceTree
- 教育理念：Constructivism Learning Theory

---

**Document Version**: 1.0  
**Last Updated**: 2026-02-20  
**Author**: Git Fighter Dev Team
