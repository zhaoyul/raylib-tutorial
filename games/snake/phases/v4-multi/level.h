#pragma once
#include "raylib.h"
#include <string>
#include <vector>

// ============================================================
// 关卡数据
// ============================================================
struct LevelData {
    std::string name;           // 关卡名称
    std::string author;         // 作者
    int width, height;          // 关卡尺寸
    std::vector<Vector2> walls; // 墙壁位置
    std::vector<Vector2> spawnPoints; // 出生点（支持多人）
    int targetScore;            // 目标分数（对战模式）
    
    LevelData() : width(40), height(30), targetScore(100) {}
    
    // 序列化
    std::string toJson() const;
    static LevelData fromJson(const std::string& json);
    
    // 验证关卡是否有效
    bool isValid() const;
};

// ============================================================
// 关卡管理器
// ============================================================
class LevelManager {
private:
    std::vector<LevelData> levels;
    std::string levelsDir;
    int currentLevel;
    
public:
    LevelManager(const std::string& dir = "levels/");
    
    // 加载和保存
    bool loadLevel(int index);
    bool saveLevel(const LevelData& level, const std::string& filename);
    bool loadAllLevels();
    
    // 获取关卡
    LevelData& getCurrentLevel() { return levels[currentLevel]; }
    const LevelData& getLevel(int index) const { return levels[index]; }
    int getLevelCount() const { return static_cast<int>(levels.size()); }
    int getCurrentIndex() const { return currentLevel; }
    
    // 切换关卡
    void setCurrentLevel(int index) { currentLevel = index; }
    void nextLevel();
    void prevLevel();
    
    // 获取关卡列表
    const std::vector<LevelData>& getAllLevels() const { return levels; }
    
    // 创建新关卡
    LevelData createNewLevel(const std::string& name);
    
    // 删除关卡
    bool deleteLevel(int index);
    
private:
    std::string getFullPath(const std::string& filename) const;
    void ensureDirectory() const;
};

// ============================================================
// 关卡编辑器
// ============================================================
class LevelEditor {
private:
    LevelData editingLevel;
    int baseGridSize;
    int gridSize;
    int offsetX, offsetY;
    bool isDirty;  // 是否有未保存的更改
    
    // 工具类型
    enum class Tool {
        WALL,           // 墙壁
        ERASE,          // 橡皮擦
        SPAWN_POINT,    // 出生点
    };
    Tool currentTool;
    int selectedSpawnPoint;
    
public:
    LevelEditor(int gridSize = 20);
    
    // 初始化
    void newLevel(const std::string& name, int width, int height);
    void loadLevel(const LevelData& level);
    
    // 更新和绘制
    void updateLayout(int screenWidth, int screenHeight);
    void update();
    void draw(int screenWidth, int screenHeight, Font font);
    
    // 输入处理
    void handleInput();
    void handleMouseInput();
    void handleKeyboardInput();
    
    // 获取编辑后的关卡
    LevelData getLevel() const { return editingLevel; }
    bool hasUnsavedChanges() const { return isDirty; }
    
    // 工具切换
    void setTool(Tool tool) { currentTool = tool; }
    Tool getTool() const { return currentTool; }
    const char* getToolName() const;
    
    // 保存提示
    void markSaved() { isDirty = false; }
    void markDirty() { isDirty = true; }
    
private:
    // 绘制网格
    void drawGrid();
    // 绘制关卡
    void drawLevel();
    // 绘制工具栏
    void drawToolbar(int screenWidth, int screenHeight, Font font);
    // 网格坐标转换
    Vector2 screenToGrid(int screenX, int screenY) const;
    bool isInGrid(int x, int y) const;
    // 添加/删除墙壁
    void addWall(int x, int y);
    void removeWall(int x, int y);
    // 设置出生点
    void setSpawnPoint(int x, int y);
};
