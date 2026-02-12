# 👥 Snake v4-multi - 双人模式与关卡编辑器

## 概述

这是 Snake 游戏的 **v4-multi** 版本，最终版本，添加了：
- 👥 本地双人同屏对战
- 🛠️ 关卡编辑器
- 💾 关卡保存/加载
- 🤖 AI 对手（基础版）

## 🎮 新增特性

### 双人模式
- **玩家1**：WASD 控制（蓝色蛇）
- **玩家2**：方向键控制（红色蛇）
- **对战规则**：
  - 竞争有限的食物
  - 可以包围/阻挡对方
  - 撞对方身体死亡
  - 先到目标分数者获胜

### 关卡编辑器
- **可视化编辑**：鼠标点击放置/删除墙壁
- **工具切换**：
  - `[1]` 墙壁工具
  - `[2]` 橡皮擦
  - `[3]` 出生点设置
- **保存/加载**：JSON 格式关卡文件
- **关卡信息**：名称、作者、尺寸、目标分数

### 关卡格式
```json
{
  "name": "迷宫挑战",
  "author": "Player1",
  "width": 40,
  "height": 30,
  "targetScore": 100,
  "walls": [{"x": 10, "y": 10}, ...],
  "spawnPoints": [{"x": 20, "y": 15}, {"x": 10, "y": 10}]
}
```

## 📁 新增文件

```
v4-multi/
├── level.h/cpp            # 关卡数据和编辑器
├── game.h/cpp             # 更新后的游戏逻辑（支持双人）
└── README.md              # 本文件
```

## 🎓 学习要点

### 1. 多人输入处理
```cpp
// 玩家1 - WASD
if (IsKeyPressed(KEY_W)) player1.setDirection(UP);
if (IsKeyPressed(KEY_A)) player1.setDirection(LEFT);
...

// 玩家2 - 方向键
if (IsKeyPressed(KEY_UP)) player2.setDirection(UP);
if (IsKeyPressed(KEY_LEFT)) player2.setDirection(LEFT);
...
```

### 2. 关卡序列化
```cpp
LevelData level;
level.name = "My Level";
level.walls.push_back({10, 10});
level.spawnPoints.push_back({20, 15});

// 保存
std::string json = level.toJson();
std::ofstream file("levels/my_level.json");
file << json;

// 加载
LevelData loaded = LevelData::fromJson(json);
```

### 3. 简单 AI（BFS 寻路）
```cpp
// 寻找最近的食物
Direction findPathToFood(const Snake& snake, const Item& food) {
    // BFS 算法
    std::queue<Position> queue;
    std::unordered_map<Position, Direction> cameFrom;
    // ...
}
```

### 4. 编辑器模式
```cpp
enum class EditorState {
    IDLE,
    PLACING_WALL,
    ERASING,
    SETTING_SPAWN
};
```

## 🏗️ 构建和运行

```bash
cmake --build build --target snake-v4-multi
./build/bin/snake-phases/snake-v4-multi
```

## 📝 操作说明

### 主菜单
- `↑/↓` - 选择模式
- `ENTER` - 确认

### 双人模式
| 玩家 | 上 | 下 | 左 | 右 |
|------|----|----|----|----|
| P1 (蓝) | W | S | A | D |
| P2 (红) | ↑ | ↓ | ← | → |

### 关卡编辑器
| 按键 | 功能 |
|------|------|
| `1` | 墙壁工具 |
| `2` | 橡皮擦 |
| `3` | 出生点工具 |
| `鼠标左键` | 放置/删除 |
| `Ctrl+S` | 保存关卡 |
| `ESC` | 返回菜单 |

## 🔧 扩展挑战

1. **在线多人**：使用 WebSocket 或 UDP 实现网络对战
2. **更多 AI 难度**：
   - 简单：随机移动
   - 中等：寻找食物
   - 困难：预测对手、设置陷阱
3. **关卡分享**：导出/导入关卡文件
4. **排行榜**：在线高分榜

## 📊 完整版本对比

| 版本 | 核心特性 | 代码行数 |
|------|---------|---------|
| v0-base | 经典贪吃蛇 | ~210 |
| v1-items | 道具系统、继承多态 | ~830 |
| v2-fx | 粒子特效、缓动函数 | ~1100 |
| v3-audio | 音效、JSON存储、设置 | ~1400 |
| **v4-multi** | **双人模式、关卡编辑器** | **~1800** |

## 🎉 完成！

这是 Snake 渐进式开发路线的最后一个版本。你已经学习了：
- C++ 面向对象编程
- 游戏架构设计
- 图形编程基础
- 数据持久化
- 多人游戏概念

---

**全部版本完成！** 🎊
