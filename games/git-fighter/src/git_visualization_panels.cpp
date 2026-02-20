#include "git_visualization.h"
#include "git_wrapper.h"
#include <algorithm>
#include <iostream>
#include "rlgl.h"

namespace GitVis {

// InternalStructurePanel implementation
InternalStructurePanel::InternalStructurePanel()
    : levelHeight(80), siblingSpacing(120), git(nullptr) {}

InternalStructurePanel::~InternalStructurePanel() = default;

void InternalStructurePanel::Initialize(int x, int y, int width, int height) {
    bounds = {(float)x, (float)y, (float)width, (float)height};
    viewport.SetViewSize(width, height);
    viewport.SetBounds(-200, -200, 1500, 800);
}

void InternalStructurePanel::LoadCommitObjects(const std::string& commitHash) {
    Clear();
    rootCommit = commitHash;
    
    std::cout << "LoadCommitObjects: Loading for commit " << commitHash.substr(0, 7) << std::endl;
    
    if (git) {
        // Load real objects from git
        std::cout << "LoadCommitObjects: Using real git data" << std::endl;
        auto gitObjects = git->GetCommitObjects(commitHash);
        std::cout << "LoadCommitObjects: Got " << gitObjects.size() << " objects" << std::endl;
        
        for (const auto& objData : gitObjects) {
            GitObject obj;
            obj.hash = objData.hash;
            obj.shortHash = objData.hash.substr(0, 7);
            obj.content = objData.content;
            obj.position = {100, 100};
            obj.targetPos = {100, 100};
            obj.scale = 1;
            obj.alpha = 1;
            obj.expanded = true;
            
            // Map type string to enum
            if (objData.type == "commit") obj.type = GitObjectType::COMMIT;
            else if (objData.type == "tree") obj.type = GitObjectType::TREE;
            else obj.type = GitObjectType::BLOB;
            
            // Add children
            for (const auto& childHash : objData.children) {
                obj.children.push_back(childHash);
            }
            
            objects[objData.hash] = obj;
        }
        
        // Set up parent-child relationships
        for (auto& pair : objects) {
            auto& obj = pair.second;
            for (const auto& childHash : obj.children) {
                if (objects.count(childHash)) {
                    objects[childHash].parents.push_back(obj.hash);
                }
            }
        }
    } else {
        // Fallback to simulated data if no git wrapper
        GitObject commit;
        commit.hash = commitHash;
        commit.shortHash = commitHash.substr(0, 7);
        commit.type = GitObjectType::COMMIT;
        commit.content = "tree abc123\nauthor...\nInitial commit";
        commit.position = {100, 100};
        commit.targetPos = {100, 100};
        commit.scale = 1;
        commit.alpha = 1;
        commit.expanded = true;
        objects[commitHash] = commit;
    }
    
    LayoutObjects();
}

void InternalStructurePanel::Clear() {
    objects.clear();
    rootCommit.clear();
    selectedObject.clear();
}

void InternalStructurePanel::LoadWorkingDirectory() {
    Clear();
    
    std::cout << "LoadWorkingDirectory: Loading working directory" << std::endl;
    
    // Create virtual "working directory" root
    GitObject workDir;
    workDir.hash = "WORKING_DIR";
    workDir.shortHash = "work";
    workDir.type = GitObjectType::TREE;
    workDir.content = "Working Directory\n(untracked & modified files)";
    workDir.position = {100, 100};
    workDir.targetPos = {100, 100};
    workDir.scale = 1;
    workDir.alpha = 1;
    workDir.expanded = true;
    
    // Add some sample files for visualization
    // In real implementation, scan the repo directory
    struct FileInfo {
        const char* name;
        const char* content;
        GitObjectType type;
    };
    
    FileInfo files[] = {
        {"README.md", "# 福报科技核心项目\n\n这是一个改变命运的项目。", GitObjectType::BLOB},
        {"main.cpp", "#include <iostream>\n\nint main() {\n    return 0;\n}", GitObjectType::BLOB},
    };
    
    for (const auto& file : files) {
        GitObject blob;
        blob.hash = std::string("blob_") + file.name;
        blob.shortHash = file.name;
        blob.type = file.type;
        blob.content = file.content;
        blob.position = {100, 100};
        blob.targetPos = {100, 100};
        blob.scale = 1;
        blob.alpha = 1;
        blob.parents.push_back("WORKING_DIR");
        
        workDir.children.push_back(blob.hash);
        objects[blob.hash] = blob;
    }
    
    objects["WORKING_DIR"] = workDir;
    rootCommit = "WORKING_DIR";
    
    std::cout << "LoadWorkingDirectory: Loaded " << objects.size() << " objects" << std::endl;
    
    LayoutObjects();
}

void InternalStructurePanel::LayoutObjects() {
    // Tree layout algorithm
    std::map<int, std::vector<std::string>> levels;
    
    // Group by level (BFS)
    std::vector<std::string> currentLevel = {rootCommit};
    int level = 0;
    
    while (!currentLevel.empty()) {
        levels[level] = currentLevel;
        
        std::vector<std::string> nextLevel;
        for (const auto& hash : currentLevel) {
            if (!objects.count(hash)) continue;
            const auto& obj = objects[hash];
            if (obj.expanded) {
                for (const auto& child : obj.children) {
                    nextLevel.push_back(child);
                }
            }
        }
        currentLevel = nextLevel;
        level++;
    }
    
    // Position objects
    for (const auto& levelPair : levels) {
        int lvl = levelPair.first;
        const auto& items = levelPair.second;
        
        float totalWidth = items.size() * siblingSpacing;
        float startX = 100;
        float y = 100 + lvl * levelHeight;
        
        for (size_t i = 0; i < items.size(); i++) {
            if (objects.count(items[i])) {
                auto& obj = objects[items[i]];
                obj.targetPos = {startX + i * siblingSpacing, y};
                // Initialize position to target to avoid animation from (0,0)
                if (obj.position.x == 100 && obj.position.y == 100) {
                    obj.position = obj.targetPos;
                }
            }
        }
    }
}

void InternalStructurePanel::ToggleNode(const std::string& hash) {
    if (!objects.count(hash)) return;
    
    auto& obj = objects[hash];
    obj.expanded = !obj.expanded;
    LayoutObjects();
}

GitObject* InternalStructurePanel::GetObjectAt(Vector2 screenPos) {
    Vector2 worldPos = viewport.ScreenToWorld({screenPos.x - bounds.x, screenPos.y - bounds.y});
    
    for (auto& pair : objects) {
        auto& obj = pair.second;
        float dx = worldPos.x - obj.position.x;
        float dy = worldPos.y - obj.position.y;
        if (dx*dx + dy*dy < 900) {  // 30px radius
            return &obj;
        }
    }
    return nullptr;
}

void InternalStructurePanel::SelectObject(const std::string& hash) {
    selectedObject = hash;
}

void InternalStructurePanel::Update(float deltaTime) {
    viewport.Update(deltaTime);
    
    // Animate positions
    for (auto& pair : objects) {
        auto& obj = pair.second;
        obj.position.x += (obj.targetPos.x - obj.position.x) * 5 * deltaTime;
        obj.position.y += (obj.targetPos.y - obj.position.y) * 5 * deltaTime;
    }
    
    // Handle input
    Vector2 mousePos = GetMousePosition();
    
    if (CheckCollisionPointRec(mousePos, bounds)) {
        Vector2 localMouse = {mousePos.x - bounds.x, mousePos.y - bounds.y};
        
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            auto* obj = GetObjectAt(mousePos);
            if (obj) {
                if (obj->type == GitObjectType::TREE) {
                    ToggleNode(obj->hash);
                }
                SelectObject(obj->hash);
            } else {
                viewport.OnDragStart(localMouse);
            }
        }
        
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            viewport.OnDrag(localMouse);
        }
        
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            viewport.OnDragEnd();
        }
        
        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            viewport.OnZoom(1 + wheel * 0.1f, localMouse);
        }
    }
}

void InternalStructurePanel::Draw() {
    BeginScissorMode((int)bounds.x, (int)bounds.y, (int)bounds.width, (int)bounds.height);
    
    DrawRectangleRec(bounds, {35, 35, 45, 255});
    viewport.Draw();
    
    // Draw connections first (behind objects)
    for (auto& pair : objects) {
        auto& obj = pair.second;
        for (const auto& parentHash : obj.parents) {
            if (!objects.count(parentHash)) continue;
            
            Vector2 childLocal = viewport.WorldToScreen(obj.position);
            Vector2 parentLocal = viewport.WorldToScreen(objects[parentHash].position);
            
            Vector2 childPos = {childLocal.x + bounds.x, childLocal.y + bounds.y};
            Vector2 parentPos = {parentLocal.x + bounds.x, parentLocal.y + bounds.y};
            
            // Draw curved connection
            Vector2 mid = {(parentPos.x + childPos.x)/2, (parentPos.y + childPos.y)/2};
            Vector2 cp1 = {parentPos.x, mid.y};
            Vector2 cp2 = {childPos.x, mid.y};
            
            Vector2 prev = parentPos;
            for (int i = 1; i <= 10; i++) {
                float t = i / 10.0f;
                float mt = 1 - t;
                Vector2 curr = {
                    mt*mt*mt*parentPos.x + 3*mt*mt*t*cp1.x + 3*mt*t*t*cp2.x + t*t*t*childPos.x,
                    mt*mt*mt*parentPos.y + 3*mt*mt*t*cp1.y + 3*mt*t*t*cp2.y + t*t*t*childPos.y
                };
                DrawLineEx(prev, curr, 2 * viewport.GetZoom(), {100, 100, 120, 200});
                prev = curr;
            }
        }
    }
    
    // Draw objects
    for (auto& pair : objects) {
        auto& obj = pair.second;
        Vector2 localPos = viewport.WorldToScreen(obj.position);
        Vector2 screenPos = {localPos.x + bounds.x, localPos.y + bounds.y};
        float r = 25 * viewport.GetZoom();
        
        if (r < 3) continue;
        
        Color c = obj.GetColor();
        
        // Selection glow
        if (obj.hash == selectedObject) {
            DrawCircle(screenPos.x, screenPos.y, r + 8, {255, 255, 255, 80});
        }
        
        // Main shape
        if (obj.type == GitObjectType::TREE) {
            DrawRectangleRounded(
                {screenPos.x - r, screenPos.y - r*0.7f, r*2, r*1.4f},
                0.3f, 8, c
            );
            if (!obj.children.empty()) {
                const char* indicator = obj.expanded ? "-" : "+";
                DrawText(indicator, (int)(screenPos.x - 5), (int)(screenPos.y - 25 - r), 
                        (int)(16 * viewport.GetZoom()), WHITE);
            }
        } else {
            DrawCircle(screenPos.x, screenPos.y, r, c);
        }
        
        DrawCircleLines(screenPos.x, screenPos.y, r, WHITE);
        
        // Labels
        int fontSize = (int)(11 * viewport.GetZoom());
        DrawText(obj.GetLabel(), (int)(screenPos.x - r + 5), (int)(screenPos.y - r + 5), 
                fontSize, WHITE);
        DrawText(obj.shortHash.c_str(), (int)(screenPos.x - r + 5), (int)(screenPos.y + 5),
                fontSize, {200, 200, 200, 255});
    }
    
    EndScissorMode();
    
    DrawRectangleLines((int)bounds.x, (int)bounds.y, (int)bounds.width, (int)bounds.height,
                       {120, 160, 200, 255});
    
    DrawRectangle((int)bounds.x, (int)bounds.y, (int)bounds.width, 30, {45, 49, 57, 200});
    DrawText("Internal Structure (blobs & trees)", (int)bounds.x + 10, (int)bounds.y + 8, 18, WHITE);
}

void InternalStructurePanel::DrawConnections() {
    for (const auto& pair : objects) {
        const auto& obj = pair.second;
        if (obj.parents.empty()) continue;
        
        Vector2 childPos = viewport.WorldToScreen(obj.position);
        
        for (const auto& parentHash : obj.parents) {
            if (!objects.count(parentHash)) continue;
            
            Vector2 parentPos = viewport.WorldToScreen(objects[parentHash].position);
            
            // Draw curved connection
            Vector2 mid = {(parentPos.x + childPos.x)/2, (parentPos.y + childPos.y)/2};
            Vector2 cp1 = {parentPos.x, mid.y};
            Vector2 cp2 = {childPos.x, mid.y};
            
            Vector2 prev = parentPos;
            for (int i = 1; i <= 10; i++) {
                float t = i / 10.0f;
                float mt = 1 - t;
                Vector2 curr = {
                    mt*mt*mt*parentPos.x + 3*mt*mt*t*cp1.x + 3*mt*t*t*cp2.x + t*t*t*childPos.x,
                    mt*mt*mt*parentPos.y + 3*mt*mt*t*cp1.y + 3*mt*t*t*cp2.y + t*t*t*childPos.y
                };
                DrawLineEx(prev, curr, 2 * viewport.GetZoom(), {100, 100, 120, 200});
                prev = curr;
            }
        }
    }
}

void InternalStructurePanel::DrawObject(const GitObject& obj) {
    Vector2 screenPos = viewport.WorldToScreen(obj.position);
    float r = 25 * viewport.GetZoom();
    
    if (r < 3) return;
    
    Color c = obj.GetColor();
    
    // Selection glow
    if (obj.hash == selectedObject) {
        DrawCircle(screenPos.x, screenPos.y, r + 8, {255, 255, 255, 80});
    }
    
    // Main shape - rounded rect for tree, circle for blob
    if (obj.type == GitObjectType::TREE) {
        DrawRectangleRounded(
            {screenPos.x - r, screenPos.y - r*0.7f, r*2, r*1.4f},
            0.3f, 8, c
        );
        
        // Expand indicator
        if (!obj.children.empty()) {
            const char* indicator = obj.expanded ? "-" : "+";
            DrawText(indicator, (int)(screenPos.x - 5), (int)(screenPos.y - 25 - r), 
                    (int)(16 * viewport.GetZoom()), WHITE);
        }
    } else {
        DrawCircle(screenPos.x, screenPos.y, r, c);
    }
    
    DrawCircleLines(screenPos.x, screenPos.y, r, WHITE);
    
    // Icon and text
    int fontSize = (int)(11 * viewport.GetZoom());
    
    // Type label
    DrawText(obj.GetLabel(), (int)(screenPos.x - r + 5), (int)(screenPos.y - r + 5), 
            fontSize, WHITE);
    
    // Hash
    DrawText(obj.shortHash.c_str(), (int)(screenPos.x - r + 5), (int)(screenPos.y + 5),
            fontSize, {200, 200, 200, 255});
}

void InternalStructurePanel::DrawObjectDetails(const GitObject& obj) {
    int panelW = 300;
    int panelH = 150;
    int x = (int)(bounds.x + bounds.width - panelW - 10);
    int y = (int)(bounds.y + bounds.height - panelH - 10);
    
    DrawRectangle(x, y, panelW, panelH, {40, 44, 52, 240});
    DrawRectangleLines(x, y, panelW, panelH, {150, 200, 255, 255});
    
    DrawText(obj.GetLabel(), x + 10, y + 10, 20, obj.GetColor());
    DrawText(obj.hash.c_str(), x + 10, y + 35, 14, LIGHTGRAY);
    
    // Preview content
    std::string preview = obj.content.substr(0, 100);
    if (obj.content.length() > 100) preview += "...";
    
    DrawText("Content:", x + 10, y + 55, 14, GRAY);
    DrawText(preview.c_str(), x + 10, y + 75, 12, WHITE);
    
    if (obj.type == GitObjectType::TREE) {
        DrawText(TextFormat("Children: %d", (int)obj.children.size()), 
                x + 10, y + 130, 14, LIGHTGRAY);
    }
}

// SplitGitView implementation
SplitGitView::SplitGitView() : splitRatio(0.6f), draggingDivider(false) {}

SplitGitView::~SplitGitView() = default;

void SplitGitView::Initialize(int x, int y, int width, int height) {
    bounds = {(float)x, (float)y, (float)width, (float)height};
    splitY = (int)(y + height * splitRatio);
    
    commitPanel.Initialize(x, y, width, splitY - y);
    structurePanel.Initialize(x, splitY, width, y + height - splitY);
}

void SplitGitView::Update(float deltaTime) {
    commitPanel.Update(deltaTime);
    structurePanel.Update(deltaTime);
    
    // Handle divider drag
    Vector2 mousePos = GetMousePosition();
    Rectangle divider = {bounds.x, (float)(splitY - 3), bounds.width, 6};
    
    if (CheckCollisionPointRec(mousePos, divider)) {
        SetMouseCursor(MOUSE_CURSOR_RESIZE_NS);
        
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            draggingDivider = true;
        }
    } else {
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    }
    
    if (draggingDivider) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            splitY = (int)mousePos.y;
            splitY = std::clamp(splitY, (int)(bounds.y + 100), (int)(bounds.y + bounds.height - 100));
            
            // Resize panels
            commitPanel.Initialize((int)bounds.x, (int)bounds.y, (int)bounds.width, splitY - (int)bounds.y);
            structurePanel.Initialize((int)bounds.x, splitY, (int)bounds.width, 
                                     (int)(bounds.y + bounds.height) - splitY);
        } else {
            draggingDivider = false;
        }
    }
}

void SplitGitView::Draw() {
    commitPanel.Draw();
    structurePanel.Draw();
    
    // Draw divider
    DrawRectangle((int)bounds.x, splitY - 2, (int)bounds.width, 4, {80, 90, 110, 255});
    DrawRectangle((int)(bounds.x + bounds.width/2 - 20), splitY - 4, 40, 8, {120, 140, 170, 255});
}

void SplitGitView::OnCommitSelected(const std::string& hash) {
    std::cout << "SplitGitView::OnCommitSelected: hash=" << hash.substr(0, 7) << std::endl;
    structurePanel.LoadCommitObjects(hash);
}

void SplitGitView::SetSplitRatio(float ratio) {
    splitRatio = std::clamp(ratio, 0.3f, 0.8f);
}

} // namespace GitVis
