#include "level.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <sys/stat.h>

// ============================================================
// LevelData 实现
// ============================================================
std::string LevelData::toJson() const {
    std::stringstream ss;
    ss << "{";
    ss << "\"name\":\"" << name << "\",";
    ss << "\"author\":\"" << author << "\",";
    ss << "\"width\":" << width << ",";
    ss << "\"height\":" << height << ",";
    ss << "\"targetScore\":" << targetScore << ",";
    
    // 墙壁
    ss << "\"walls\":[";
    for (size_t i = 0; i < walls.size(); i++) {
        if (i > 0) ss << ",";
        ss << "{\"x\":" << walls[i].x << ",\"y\":" << walls[i].y << "}";
    }
    ss << "],";
    
    // 出生点
    ss << "\"spawnPoints\":[";
    for (size_t i = 0; i < spawnPoints.size(); i++) {
        if (i > 0) ss << ",";
        ss << "{\"x\":" << spawnPoints[i].x << ",\"y\":" << spawnPoints[i].y << "}";
    }
    ss << "]";
    
    ss << "}";
    return ss.str();
}

LevelData LevelData::fromJson(const std::string& json) {
    LevelData level;
    
    // 简单解析
    auto parseString = [&json](const std::string& key) -> std::string {
        size_t pos = json.find("\"" + key + "\":");
        if (pos != std::string::npos) {
            pos += key.length() + 4;
            size_t end = json.find("\"", pos);
            return json.substr(pos, end - pos);
        }
        return "";
    };
    
    auto parseInt = [&json](const std::string& key) -> int {
        size_t pos = json.find("\"" + key + "\":");
        if (pos != std::string::npos) {
            pos += key.length() + 3;
            size_t end = json.find(",", pos);
            if (end == std::string::npos) end = json.find("}", pos);
            return std::stoi(json.substr(pos, end - pos));
        }
        return 0;
    };
    
    level.name = parseString("name");
    level.author = parseString("author");
    level.width = parseInt("width");
    level.height = parseInt("height");
    level.targetScore = parseInt("targetScore");
    
    // 解析墙壁
    size_t wallsPos = json.find("\"walls\":");
    if (wallsPos != std::string::npos) {
        wallsPos += 9;
        size_t wallsEnd = json.find("]", wallsPos);
        std::string wallsStr = json.substr(wallsPos, wallsEnd - wallsPos);
        
        size_t pos = 0;
        while ((pos = wallsStr.find("{", pos)) != std::string::npos) {
            size_t end = wallsStr.find("}", pos);
            std::string wall = wallsStr.substr(pos, end - pos + 1);
            
            size_t xPos = wall.find("\"x\":");
            size_t yPos = wall.find("\"y\":");
            if (xPos != std::string::npos && yPos != std::string::npos) {
                int x = std::stoi(wall.substr(xPos + 4));
                int y = std::stoi(wall.substr(yPos + 4));
                level.walls.push_back({(float)x, (float)y});
            }
            pos = end + 1;
        }
    }
    
    // 解析出生点
    size_t spawnPos = json.find("\"spawnPoints\":");
    if (spawnPos != std::string::npos) {
        spawnPos += 15;
        size_t spawnEnd = json.find("]", spawnPos);
        std::string spawnStr = json.substr(spawnPos, spawnEnd - spawnPos);
        
        size_t pos = 0;
        while ((pos = spawnStr.find("{", pos)) != std::string::npos) {
            size_t end = spawnStr.find("}", pos);
            std::string spawn = spawnStr.substr(pos, end - pos + 1);
            
            size_t xPos = spawn.find("\"x\":");
            size_t yPos = spawn.find("\"y\":");
            if (xPos != std::string::npos && yPos != std::string::npos) {
                int x = std::stoi(spawn.substr(xPos + 4));
                int y = std::stoi(spawn.substr(yPos + 4));
                level.spawnPoints.push_back({(float)x, (float)y});
            }
            pos = end + 1;
        }
    }
    
    return level;
}

bool LevelData::isValid() const {
    return width > 0 && height > 0 && 
           spawnPoints.size() >= 1;
}

// ============================================================
// LevelManager 实现
// ============================================================
LevelManager::LevelManager(const std::string& dir)
    : levelsDir(dir), currentLevel(0) {
    loadAllLevels();
}

bool LevelManager::loadLevel(int index) {
    if (index >= 0 && index < static_cast<int>(levels.size())) {
        currentLevel = index;
        return true;
    }
    return false;
}

bool LevelManager::saveLevel(const LevelData& level, const std::string& filename) {
    ensureDirectory();
    
    std::string fullPath = getFullPath(filename);
    std::ofstream file(fullPath);
    if (!file.is_open()) {
        return false;
    }
    
    file << level.toJson();
    return true;
}

bool LevelManager::loadAllLevels() {
    namespace fs = std::filesystem;

    levels.clear();
    ensureDirectory();
    
    // 创建默认关卡
    LevelData defaultLevel;
    defaultLevel.name = "随机生成";
    defaultLevel.author = "System";
    defaultLevel.width = 40;
    defaultLevel.height = 30;
    defaultLevel.spawnPoints.push_back({20, 15});
    defaultLevel.spawnPoints.push_back({10, 10});
    levels.push_back(defaultLevel);

    std::vector<fs::path> levelFiles;
    try {
        for (const auto& entry : fs::directory_iterator(levelsDir)) {
            if (!entry.is_regular_file()) continue;
            if (entry.path().extension() == ".json") {
                levelFiles.push_back(entry.path());
            }
        }
    } catch (...) {
        if (currentLevel >= static_cast<int>(levels.size())) {
            currentLevel = 0;
        }
        return true;
    }

    std::sort(levelFiles.begin(), levelFiles.end());

    for (const auto& filePath : levelFiles) {
        std::ifstream file(filePath);
        if (!file.is_open()) continue;

        std::stringstream buffer;
        buffer << file.rdbuf();
        const std::string json = buffer.str();
        if (json.empty()) continue;

        try {
            LevelData level = LevelData::fromJson(json);
            if (level.name.empty()) {
                level.name = filePath.stem().string();
            }
            if (level.author.empty()) {
                level.author = "Player";
            }
            if (level.targetScore <= 0) {
                level.targetScore = 100;
            }

            if (!level.isValid()) {
                continue;
            }

            if (level.width > 40 || level.height > 30) {
                continue;
            }

            levels.push_back(level);
        } catch (...) {
            continue;
        }
    }

    if (currentLevel >= static_cast<int>(levels.size())) {
        currentLevel = 0;
    }
    
    return true;
}

void LevelManager::nextLevel() {
    currentLevel = (currentLevel + 1) % static_cast<int>(levels.size());
}

void LevelManager::prevLevel() {
    currentLevel = (currentLevel + static_cast<int>(levels.size()) - 1) % 
                   static_cast<int>(levels.size());
}

LevelData LevelManager::createNewLevel(const std::string& name) {
    LevelData level;
    level.name = name;
    level.author = "Player";
    level.width = 40;
    level.height = 30;
    level.targetScore = 100;
    level.spawnPoints.push_back({20, 15});
    return level;
}

bool LevelManager::deleteLevel(int index) {
    if (index > 0 && index < static_cast<int>(levels.size())) {
        levels.erase(levels.begin() + index);
        if (currentLevel >= static_cast<int>(levels.size())) {
            currentLevel = static_cast<int>(levels.size()) - 1;
        }
        return true;
    }
    return false;
}

std::string LevelManager::getFullPath(const std::string& filename) const {
    return levelsDir + filename;
}

void LevelManager::ensureDirectory() const {
    #ifdef _WIN32
    _mkdir(levelsDir.c_str());
    #else
    mkdir(levelsDir.c_str(), 0755);
    #endif
}

// ============================================================
// LevelEditor 实现
// ============================================================
LevelEditor::LevelEditor(int grid)
    : baseGridSize(grid), gridSize(grid), offsetX(0), offsetY(0), 
      isDirty(false), currentTool(Tool::WALL), selectedSpawnPoint(0) {
}

void LevelEditor::newLevel(const std::string& name, int width, int height) {
    editingLevel = LevelData();
    editingLevel.name = name;
    editingLevel.width = width;
    editingLevel.height = height;
    editingLevel.spawnPoints.push_back({static_cast<float>(width / 2), static_cast<float>(height / 2)});
    isDirty = false;
}

void LevelEditor::loadLevel(const LevelData& level) {
    editingLevel = level;
    isDirty = false;
}

void LevelEditor::update() {
    handleInput();
}

void LevelEditor::updateLayout(int screenWidth, int screenHeight) {
    const int topPadding = 40;
    const int bottomPadding = 30;
    const int sidePadding = 10;

    const int availableWidth = std::max(1, screenWidth - sidePadding * 2);
    const int availableHeight = std::max(1, screenHeight - topPadding - bottomPadding);

    const int fitWidth = std::max(1, availableWidth / std::max(1, editingLevel.width));
    const int fitHeight = std::max(1, availableHeight / std::max(1, editingLevel.height));
    gridSize = std::max(8, std::min({baseGridSize, fitWidth, fitHeight}));

    const int pixelWidth = editingLevel.width * gridSize;
    const int pixelHeight = editingLevel.height * gridSize;

    offsetX = (screenWidth - pixelWidth) / 2;
    offsetY = topPadding + (availableHeight - pixelHeight) / 2;
}

void LevelEditor::draw(int screenWidth, int screenHeight, Font font) {
    updateLayout(screenWidth, screenHeight);
    drawGrid();
    drawLevel();
    drawToolbar(screenWidth, screenHeight, font);
}

void LevelEditor::handleInput() {
    handleMouseInput();
    handleKeyboardInput();
}

void LevelEditor::handleMouseInput() {
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePos = GetMousePosition();
        Vector2 gridPos = screenToGrid(static_cast<int>(mousePos.x), static_cast<int>(mousePos.y));
        
        int x = static_cast<int>(gridPos.x);
        int y = static_cast<int>(gridPos.y);
        
        if (isInGrid(x, y)) {
            switch (currentTool) {
                case Tool::WALL:
                    addWall(x, y);
                    break;
                case Tool::ERASE:
                    removeWall(x, y);
                    break;
                case Tool::SPAWN_POINT:
                    setSpawnPoint(x, y);
                    break;
            }
        }
    }
}

void LevelEditor::handleKeyboardInput() {
    // 工具切换
    if (IsKeyPressed(KEY_ONE)) {
        currentTool = Tool::WALL;
    } else if (IsKeyPressed(KEY_TWO)) {
        currentTool = Tool::ERASE;
    } else if (IsKeyPressed(KEY_THREE)) {
        currentTool = Tool::SPAWN_POINT;
    }
    
    // 清除所有
    if (IsKeyPressed(KEY_C) && (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_LEFT_SUPER))) {
        editingLevel.walls.clear();
        isDirty = true;
    }
}

void LevelEditor::drawGrid() {
    for (int i = 0; i <= editingLevel.width; i++) {
        int x = offsetX + i * gridSize;
        DrawLine(x, offsetY, x, offsetY + editingLevel.height * gridSize, LIGHTGRAY);
    }
    for (int i = 0; i <= editingLevel.height; i++) {
        int y = offsetY + i * gridSize;
        DrawLine(offsetX, y, offsetX + editingLevel.width * gridSize, y, LIGHTGRAY);
    }
}

void LevelEditor::drawLevel() {
    // 绘制墙壁
    for (const auto& wall : editingLevel.walls) {
        int x = offsetX + static_cast<int>(wall.x) * gridSize;
        int y = offsetY + static_cast<int>(wall.y) * gridSize;
        DrawRectangle(x, y, gridSize, gridSize, GRAY);
    }
    
    // 绘制出生点
    for (size_t i = 0; i < editingLevel.spawnPoints.size(); i++) {
        const auto& spawn = editingLevel.spawnPoints[i];
        int x = offsetX + static_cast<int>(spawn.x) * gridSize;
        int y = offsetY + static_cast<int>(spawn.y) * gridSize;
        Color color = (static_cast<int>(i) == selectedSpawnPoint) ? GREEN : BLUE;
        DrawCircle(x + gridSize/2, y + gridSize/2, gridSize/3, color);
    }
}

void LevelEditor::drawToolbar(int /* screenWidth */, int screenHeight, Font font) {
    const char* toolNames[] = {"[1] 墙壁", "[2] 橡皮", "[3] 出生点"};
    const char* currentToolName = toolNames[static_cast<int>(currentTool)];

    DrawTextEx(font, currentToolName, {10.0f, 10.0f}, 20, 1.0f, DARKGRAY);
    
    if (isDirty) {
        DrawTextEx(font, "*未保存", {100.0f, 10.0f}, 20, 1.0f, RED);
    }
    
    // 显示关卡信息
    DrawTextEx(font, TextFormat("关卡: %s", editingLevel.name.c_str()),
               {10.0f, static_cast<float>(screenHeight - 30)}, 16, 1.0f, GRAY);
    DrawTextEx(font, TextFormat("尺寸: %dx%d", editingLevel.width, editingLevel.height),
               {200.0f, static_cast<float>(screenHeight - 30)}, 16, 1.0f, GRAY);
    DrawTextEx(font, "ENTER 返回菜单  |  ESC 退出程序",
               {10.0f, static_cast<float>(screenHeight - 50)}, 16, 1.0f, DARKGRAY);
}

Vector2 LevelEditor::screenToGrid(int screenX, int screenY) const {
    int x = static_cast<int>(std::floor((screenX - offsetX) / static_cast<float>(gridSize)));
    int y = static_cast<int>(std::floor((screenY - offsetY) / static_cast<float>(gridSize)));
    return {(float)x, (float)y};
}

bool LevelEditor::isInGrid(int x, int y) const {
    return x >= 0 && x < editingLevel.width && y >= 0 && y < editingLevel.height;
}

void LevelEditor::addWall(int x, int y) {
    // 检查是否已存在
    for (const auto& wall : editingLevel.walls) {
        if ((int)wall.x == x && (int)wall.y == y) {
            return;
        }
    }
    editingLevel.walls.push_back({(float)x, (float)y});
    isDirty = true;
}

void LevelEditor::removeWall(int x, int y) {
    editingLevel.walls.erase(
        std::remove_if(editingLevel.walls.begin(), editingLevel.walls.end(),
            [x, y](const Vector2& w) { return (int)w.x == x && (int)w.y == y; }),
        editingLevel.walls.end()
    );
    isDirty = true;
}

void LevelEditor::setSpawnPoint(int x, int y) {
    if (selectedSpawnPoint < static_cast<int>(editingLevel.spawnPoints.size())) {
        editingLevel.spawnPoints[selectedSpawnPoint] = {(float)x, (float)y};
        isDirty = true;
    }
}

const char* LevelEditor::getToolName() const {
    switch (currentTool) {
        case Tool::WALL: return "墙壁";
        case Tool::ERASE: return "橡皮擦";
        case Tool::SPAWN_POINT: return "出生点";
        default: return "未知";
    }
}
