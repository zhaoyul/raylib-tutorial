#include "level_manager.h"
#include "git_wrapper.h"
#include "../levels/level_01_realgit.h"
#include "../levels/level_02_branch.h"
#include "../levels/level_03_merge.h"
#include "../levels/level_04_remote.h"
#include <iostream>
#include <algorithm>

Level::Level(int id, const std::string& name, const std::string& desc)
    : levelId(id), levelName(name), description(desc) {}

LevelManager::LevelManager() = default;
LevelManager::~LevelManager() = default;

bool LevelManager::Initialize() {
    git = std::make_unique<GitWrapper>();
    
    // Load Chinese font
    font.Load();
    
    // Register all levels - store them in a separate registry
    auto level1 = std::make_unique<Level01_RealGit>();
    levels.push_back(std::move(level1));
    
    auto level2 = std::make_unique<Level02_Branch>();
    levels.push_back(std::move(level2));
    
    auto level3 = std::make_unique<Level03_Merge>();
    levels.push_back(std::move(level3));
    
    auto level4 = std::make_unique<Level04_Remote>();
    levels.push_back(std::move(level4));
    
    // TODO: Register more levels
    
    return true;
}

void LevelManager::RegisterLevel(std::unique_ptr<Level> level) {
    levels.push_back(std::move(level));
}

void LevelManager::LoadLevel(int levelId) {
    // Find level by ID
    auto it = std::find_if(levels.begin(), levels.end(),
        [levelId](const auto& level) { return level->GetId() == levelId; });
    
    if (it != levels.end()) {
        UnloadCurrentLevel();
        // Move ownership to currentLevel
        currentLevel = std::move(*it);
        // Remove from available levels
        levels.erase(it);
        
        // Set font for level
        currentLevel->SetFont(&font);
        
        currentLevel->Initialize();
        std::cout << "Loaded Level " << levelId << ": " << currentLevel->GetName() << std::endl;
    }
}

void LevelManager::UnloadCurrentLevel() {
    if (currentLevel) {
        currentLevel->Shutdown();
        // Return to available levels
        levels.push_back(std::move(currentLevel));
        currentLevel.reset();
    }
}

void LevelManager::Update(float deltaTime) {
    if (currentLevel) {
        currentLevel->Update(deltaTime);
    }
}

void LevelManager::Draw() {
    if (currentLevel) {
        currentLevel->Draw();
    }
}

bool LevelManager::IsCurrentLevelComplete() const {
    return currentLevel && currentLevel->IsComplete();
}
