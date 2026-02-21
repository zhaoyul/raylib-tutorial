#include "git_visualization.h"
#include <cmath>
#include <algorithm>

namespace GitVis {

// Color palette
static const Color COLOR_COMMIT = {100, 180, 255, 255};
static const Color COLOR_MERGE = {255, 150, 100, 255};
static const Color COLOR_HEAD = {255, 100, 100, 255};
static const Color COLOR_BRANCH = {150, 255, 150, 255};
static const Color COLOR_TAG = {255, 220, 100, 255};
static const Color COLOR_TREE = {180, 150, 255, 255};
static const Color COLOR_BLOB = {200, 200, 200, 255};

static const Color BRANCH_COLORS[] = {
    {100, 200, 255, 255},
    {255, 150, 100, 255},
    {150, 255, 100, 255},
    {255, 100, 200, 255},
    {255, 220, 100, 255},
    {180, 100, 255, 255},
};

// SpringAnim implementation
void SpringAnim::Update(float deltaTime) {
    float force = (target - position) * stiffness;
    velocity += force * deltaTime;
    velocity *= damping;
    position += velocity * deltaTime;
}

bool SpringAnim::IsSettled(float threshold) const {
    return std::abs(target - position) < threshold && std::abs(velocity) < threshold;
}

// GitObject implementation
Color GitObject::GetColor() const {
    switch (type) {
        case GitObjectType::COMMIT: return COLOR_COMMIT;
        case GitObjectType::TREE: return COLOR_TREE;
        case GitObjectType::BLOB: return COLOR_BLOB;
        case GitObjectType::TAG: return COLOR_TAG;
        case GitObjectType::BRANCH: return COLOR_BRANCH;
    }
    return WHITE;
}

const char* GitObject::GetIcon() const {
    switch (type) {
        case GitObjectType::COMMIT: return "📦";
        case GitObjectType::TREE: return "🌳";
        case GitObjectType::BLOB: return "📄";
        case GitObjectType::TAG: return "🏷️";
        case GitObjectType::BRANCH: return "🌿";
    }
    return "?";
}

const char* GitObject::GetLabel() const {
    switch (type) {
        case GitObjectType::COMMIT: return "commit";
        case GitObjectType::TREE: return "tree";
        case GitObjectType::BLOB: return "blob";
        case GitObjectType::TAG: return "tag";
        case GitObjectType::BRANCH: return "branch";
    }
    return "unknown";
}

// DraggableView implementation
DraggableView::DraggableView()
    : offset{0, 0}, zoom(1.0f)
    , boundsMinX(-1000), boundsMinY(-1000), boundsMaxX(2000), boundsMaxY(2000)
    , viewWidth(800), viewHeight(600)
    , isDragging(false) {}

void DraggableView::SetBounds(float minX, float minY, float maxX, float maxY) {
    boundsMinX = minX;
    boundsMinY = minY;
    boundsMaxX = maxX;
    boundsMaxY = maxY;
}

void DraggableView::SetViewSize(float width, float height) {
    viewWidth = width;
    viewHeight = height;
}

void DraggableView::OnDragStart(Vector2 pos) {
    isDragging = true;
    dragStartPos = pos;
    dragStartOffset = offset;
}

void DraggableView::OnDrag(Vector2 pos) {
    if (!isDragging) return;
    Vector2 delta = {pos.x - dragStartPos.x, pos.y - dragStartPos.y};
    offset.x = dragStartOffset.x - delta.x / zoom;
    offset.y = dragStartOffset.y - delta.y / zoom;
}

void DraggableView::OnDragEnd() {
    isDragging = false;
    // No spring physics - position stays where dragged
}

void DraggableView::OnZoom(float factor, Vector2 center) {
    float oldZoom = zoom;
    zoom *= factor;
    zoom = std::clamp(zoom, 0.2f, 3.0f);
    
    // Zoom toward center
    float zoomDelta = zoom - oldZoom;
    offset.x -= (center.x - viewWidth/2) * zoomDelta / (oldZoom * zoom);
    offset.y -= (center.y - viewHeight/2) * zoomDelta / (oldZoom * zoom);
}

Vector2 DraggableView::WorldToScreen(Vector2 worldPos) const {
    return {
        (worldPos.x - offset.x) * zoom + viewWidth / 2,
        (worldPos.y - offset.y) * zoom + viewHeight / 2
    };
}

Vector2 DraggableView::ScreenToWorld(Vector2 screenPos) const {
    return {
        (screenPos.x - viewWidth / 2) / zoom + offset.x,
        (screenPos.y - viewHeight / 2) / zoom + offset.y
    };
}

// ApplySpringForce removed - direct positioning only

void DraggableView::Update(float deltaTime) {
    // No spring physics - direct positioning only
    // offset is set directly in OnDrag
}

void DraggableView::Draw() const {
    // Draw grid
    int gridSize = 50;
    Color gridColor = {50, 50, 60, 100};
    
    float startX = fmod(-offset.x * zoom + viewWidth/2, gridSize * zoom);
    float startY = fmod(-offset.y * zoom + viewHeight/2, gridSize * zoom);
    
    for (float x = startX; x < viewWidth; x += gridSize * zoom) {
        DrawLineV({x, 0}, {x, viewHeight}, gridColor);
    }
    for (float y = startY; y < viewHeight; y += gridSize * zoom) {
        DrawLineV({0, y}, {viewWidth, y}, gridColor);
    }
}

// CommitNode implementation
void CommitNode::Update(float deltaTime) {
    // Direct positioning - no spring physics
    // Scale animation (keep hover effect)
    float targetScale = hovered ? 1.2f : 1.0f;
    scale += (targetScale - scale) * 10 * deltaTime;
    
    // Alpha fade in
    if (alpha < 1.0f) alpha += deltaTime * 2;
    if (alpha > 1.0f) alpha = 1.0f;
    
    // Glow pulse
    if (selected) {
        glowIntensity += deltaTime * 3;
        if (glowIntensity > 1.0f) glowIntensity = 0.0f;
    }
    
    // Animation progress
    if (animProgress < 1.0f) {
        animProgress += deltaTime;
        if (animProgress > 1.0f) animProgress = 1.0f;
    }
}

void CommitNode::StartAnimation(AnimType type) {
    currentAnim = type;
    animProgress = 0;
    
    switch (type) {
        case AnimType::COMMIT_APPEAR:
            scale = 0;
            alpha = 0;
            break;
        default:
            break;
    }
}

Rectangle CommitNode::GetBounds() const {
    float r = radius * scale;
    return {position.x - r, position.y - r, r * 2, r * 2};
}

// CommitEdge implementation
void CommitEdge::CalculateCurve(const Vector2& fromPos, const Vector2& toPos,
                                float fromLane, float toLane) {
    waypoints.clear();
    
    // Create smooth bezier curve
    float midY = (fromPos.y + toPos.y) / 2;
    
    if (std::abs(fromLane - toLane) < 0.5f) {
        // Same lane - straight line
        waypoints.push_back(fromPos);
        waypoints.push_back(toPos);
    } else {
        // Different lanes - curved
        Vector2 cp1 = {fromPos.x, midY};
        Vector2 cp2 = {toPos.x, midY};
        
        // Generate curve points
        for (int i = 0; i <= 20; i++) {
            float t = i / 20.0f;
            float mt = 1 - t;
            
            // Cubic bezier
            Vector2 p = {
                mt*mt*mt*fromPos.x + 3*mt*mt*t*cp1.x + 3*mt*t*t*cp2.x + t*t*t*toPos.x,
                mt*mt*mt*fromPos.y + 3*mt*mt*t*cp1.y + 3*mt*t*t*cp2.y + t*t*t*toPos.y
            };
            waypoints.push_back(p);
        }
    }
}

void CommitEdge::Draw() const {
    if (waypoints.size() < 2) return;
    
    Color c = color;
    c.a = (unsigned char)(255 * progress);
    
    for (size_t i = 1; i < waypoints.size(); i++) {
        DrawLineV(waypoints[i-1], waypoints[i], c);
    }
}

// CommitGraphPanel implementation
CommitGraphPanel::CommitGraphPanel() : time(0) {}
CommitGraphPanel::~CommitGraphPanel() = default;

void CommitGraphPanel::Initialize(int x, int y, int width, int height) {
    bounds = {(float)x, (float)y, (float)width, (float)height};
    viewport.SetViewSize(width, height);
    viewport.SetBounds(-500, -500, 2000, 1500);
}

void CommitGraphPanel::AddCommit(const CommitNode& commit) {
    nodes[commit.hash] = commit;
    
    // Create edges to parents
    for (const auto& parent : commit.parents) {
        CommitEdge edge;
        edge.from = commit.hash;
        edge.to = parent;
        edge.color = {150, 150, 150, 255};
        edge.thickness = 2;
        edge.progress = 0;
        edges.push_back(edge);
    }
}

void CommitGraphPanel::AddBranch(const std::string& name, const std::string& head, Color color) {
    branchColors[name] = color;
    if (nodes.count(head)) {
        nodes[head].branches.push_back(name);
    }
}

void CommitGraphPanel::SetHEAD(const std::string& hash) {
    headHash = hash;
}

void CommitGraphPanel::Clear() {
    nodes.clear();
    edges.clear();
    branchColors.clear();
    headHash.clear();
    selectedHash.clear();
}

void CommitGraphPanel::RecalculateLayout() {
    // Calculate lanes
    std::map<std::string, int> commitLanes;
    int nextLane = 0;
    
    // Sort by timestamp (newest first, at top)
    std::vector<std::string> sortedHashes;
    for (const auto& pair : nodes) {
        sortedHashes.push_back(pair.first);
    }
    std::sort(sortedHashes.begin(), sortedHashes.end(),
        [this](const std::string& a, const std::string& b) {
            return nodes[a].timestamp > nodes[b].timestamp;
        });
    
    // Assign lanes
    float y = 100;
    for (const auto& hash : sortedHashes) {
        auto& node = nodes[hash];
        
        // Try to use parent's lane
        int lane = -1;
        for (const auto& parent : node.parents) {
            if (commitLanes.count(parent)) {
                int parentLane = commitLanes[parent];
                bool available = true;
                for (const auto& other : sortedHashes) {
                    if (other == hash) continue;
                    if (commitLanes.count(other) && commitLanes[other] == parentLane) {
                        if (std::abs(nodes[other].targetPos.y - y) < 80) {
                            available = false;
                            break;
                        }
                    }
                }
                if (available && lane == -1) {
                    lane = parentLane;
                }
            }
        }
        
        if (lane == -1) lane = nextLane++;
        
        commitLanes[hash] = lane;
        node.lane = lane;
        node.targetPos = {200.0f + lane * 150.0f, y};
        
        // Direct positioning - no spring animation
        node.position = node.targetPos;
        y += 100;
    }
    
    // Update edge curves
    for (auto& edge : edges) {
        if (nodes.count(edge.from) && nodes.count(edge.to)) {
            edge.CalculateCurve(
                nodes[edge.from].position,
                nodes[edge.to].position,
                nodes[edge.from].lane,
                nodes[edge.to].lane
            );
        }
    }
}

void CommitGraphPanel::AnimateToLayout() {
    // Direct positioning - no animation
    for (auto& pair : nodes) {
        pair.second.position = pair.second.targetPos;
    }
}

CommitNode* CommitGraphPanel::GetNodeAt(Vector2 screenPos) {
    Vector2 worldPos = viewport.ScreenToWorld({screenPos.x - bounds.x, screenPos.y - bounds.y});
    
    for (auto& pair : nodes) {
        auto& node = pair.second;
        float dx = worldPos.x - node.position.x;
        float dy = worldPos.y - node.position.y;
        if (dx*dx + dy*dy < node.radius * node.radius * node.scale * node.scale) {
            return &node;
        }
    }
    return nullptr;
}

void CommitGraphPanel::SelectNode(const std::string& hash) {
    if (!selectedHash.empty()) {
        nodes[selectedHash].selected = false;
    }
    selectedHash = hash;
    if (nodes.count(hash)) {
        nodes[hash].selected = true;
        if (onNodeSelected) {
            onNodeSelected(nodes[hash]);
        }
    }
}

void CommitGraphPanel::CenterOnNode(const std::string& hash) {
    if (!nodes.count(hash)) return;
    
    const auto& node = nodes[hash];
    viewport.OnDragEnd();  // Reset velocity
}

void CommitGraphPanel::Update(float deltaTime) {
    time += deltaTime;
    
    viewport.Update(deltaTime);
    
    for (auto& pair : nodes) {
        pair.second.Update(deltaTime);
    }
    
    // Update edge curves as nodes move
    for (auto& edge : edges) {
        if (nodes.count(edge.from) && nodes.count(edge.to)) {
            edge.CalculateCurve(
                nodes[edge.from].position,
                nodes[edge.to].position,
                nodes[edge.from].lane,
                nodes[edge.to].lane
            );
        }
    }
    
    // Handle input
    Vector2 mousePos = GetMousePosition();
    Vector2 localMouse = {mousePos.x - bounds.x, mousePos.y - bounds.y};
    
    // Track if we started dragging within this panel
    static bool startedDragInPanel = false;
    
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (CheckCollisionPointRec(mousePos, bounds)) {
            auto* node = GetNodeAt(mousePos);
            if (node) {
                SelectNode(node->hash);
                startedDragInPanel = false;
            } else {
                viewport.OnDragStart(localMouse);
                startedDragInPanel = true;
            }
        } else {
            startedDragInPanel = false;
        }
    }
    
    // Continue dragging even if mouse leaves panel (more responsive)
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && startedDragInPanel) {
        viewport.OnDrag(localMouse);
    }
    
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        if (startedDragInPanel) {
            viewport.OnDragEnd();
        }
        startedDragInPanel = false;
    }
    
    // Only zoom and hover when mouse is over panel
    if (CheckCollisionPointRec(mousePos, bounds)) {
        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            viewport.OnZoom(1 + wheel * 0.1f, localMouse);
        }
        
        // Hover detection
        for (auto& pair : nodes) {
            pair.second.hovered = false;
        }
        auto* hovered = GetNodeAt(mousePos);
        if (hovered) {
            hovered->hovered = true;
            if (onNodeHovered) {
                onNodeHovered(*hovered);
            }
        }
    }
}

void CommitGraphPanel::Draw() {
    BeginScissorMode((int)bounds.x, (int)bounds.y, (int)bounds.width, (int)bounds.height);
    
    // Background
    DrawRectangleRec(bounds, {30, 30, 40, 255});
    
    // Grid
    viewport.Draw();
    
    // Edges
    DrawEdges();
    
    // Nodes
    DrawNodes();
    
    // Labels
    DrawBranchLabels();
    DrawHEADIndicator();
    
    EndScissorMode();
    
    // Border
    DrawRectangleLines((int)bounds.x, (int)bounds.y, (int)bounds.width, (int)bounds.height, 
                       {100, 150, 200, 255});
    
    // Title
    DrawRectangle((int)bounds.x, (int)bounds.y, (int)bounds.width, 30, {40, 44, 52, 200});
    DrawText("Commit History", (int)bounds.x + 10, (int)bounds.y + 8, 18, WHITE);
}

void CommitGraphPanel::DrawEdges() {
    for (const auto& edge : edges) {
        if (!nodes.count(edge.from) || !nodes.count(edge.to)) continue;
        
        const auto& from = nodes[edge.from];
        const auto& to = nodes[edge.to];
        
        Vector2 fromScreen = viewport.WorldToScreen(from.position);
        Vector2 toScreen = viewport.WorldToScreen(to.position);
        
        // Get branch color
        Color lineColor = {120, 120, 120, 200};
        if (!from.branches.empty() && branchColors.count(from.branches[0])) {
            lineColor = branchColors[from.branches[0]];
            lineColor.a = 150;
        }
        
        // Draw curved line
        float midY = (fromScreen.y + toScreen.y) / 2;
        Vector2 cp1 = {fromScreen.x, midY};
        Vector2 cp2 = {toScreen.x, midY};
        
        // Draw as segments
        Vector2 prev = fromScreen;
        for (int i = 1; i <= 20; i++) {
            float t = i / 20.0f;
            float mt = 1 - t;
            Vector2 curr = {
                mt*mt*mt*fromScreen.x + 3*mt*mt*t*cp1.x + 3*mt*t*t*cp2.x + t*t*t*toScreen.x,
                mt*mt*mt*fromScreen.y + 3*mt*mt*t*cp1.y + 3*mt*t*t*cp2.y + t*t*t*toScreen.y
            };
            DrawLineEx(prev, curr, 2.0f, lineColor);
            prev = curr;
        }
    }
}

void CommitGraphPanel::DrawNodes() {
    for (const auto& pair : nodes) {
        const auto& node = pair.second;
        
        Vector2 screenPos = viewport.WorldToScreen(node.position);
        // Fixed node size - doesn't scale with zoom
        float r = node.radius * node.scale;
        
        // Skip if node would be off-screen (culling optimization)
        if (screenPos.x < bounds.x - 50 || screenPos.x > bounds.x + bounds.width + 50 ||
            screenPos.y < bounds.y - 50 || screenPos.y > bounds.y + bounds.height + 50) {
            continue;
        }
        
        Color nodeColor = COLOR_COMMIT;
        if (node.branches.size() > 1 || node.parents.size() > 1) {
            nodeColor = COLOR_MERGE;
        }
        
        // Glow effect for selected
        if (node.selected) {
            float glow = (1 + sin(time * 5)) * 0.5f;
            DrawCircle(screenPos.x, screenPos.y, r + 8 + glow * 4, 
                      {100, 200, 255, (unsigned char)(100 * glow)});
        }
        
        // Glow for HEAD
        if (node.hash == headHash) {
            DrawCircle(screenPos.x, screenPos.y, r + 6, {255, 100, 100, 100});
        }
        
        // Main circle
        DrawCircle(screenPos.x, screenPos.y, r, nodeColor);
        DrawCircleLines(screenPos.x, screenPos.y, r, WHITE);
        
        // Inner circle
        DrawCircle(screenPos.x, screenPos.y, r * 0.7f, {40, 44, 52, 255});
        
        // Hash text - fixed font size
        int fontSize = 12;
        int textW = MeasureText(node.shortHash.c_str(), fontSize);
        DrawText(node.shortHash.c_str(), 
                (int)(screenPos.x - textW/2), 
                (int)(screenPos.y - fontSize/2),
                fontSize, WHITE);
    }
}

void CommitGraphPanel::DrawBranchLabels() {
    for (const auto& pair : nodes) {
        const auto& node = pair.second;
        if (node.branches.empty()) continue;
        
        Vector2 screenPos = viewport.WorldToScreen(node.position);
        // Fixed node size
        float r = node.radius * node.scale;
        
        float labelY = screenPos.y - r - 10;
        
        for (const auto& branch : node.branches) {
            Color branchColor = branchColors.count(branch) ? branchColors[branch] : GRAY;
            
            std::string label = " " + branch + " ";
            int fontSize = 12;
            int textW = MeasureText(label.c_str(), fontSize);
            
            DrawRectangle((int)(screenPos.x + r + 5), (int)(labelY - 10), textW + 10, 20, branchColor);
            DrawText(label.c_str(), (int)(screenPos.x + r + 10), (int)(labelY - 8), fontSize, WHITE);
            
            labelY -= 22;
        }
    }
}

void CommitGraphPanel::DrawHEADIndicator() {
    if (headHash.empty() || !nodes.count(headHash)) return;
    
    const auto& head = nodes[headHash];
    Vector2 screenPos = viewport.WorldToScreen(head.position);
    // Fixed node size
    float r = head.radius * head.scale;
    
    std::string label = " HEAD ";
    int fontSize = 14;
    int textW = MeasureText(label.c_str(), fontSize);
    
    float boxX = screenPos.x - textW/2;
    float boxY = screenPos.y - r - 35;
    
    DrawRectangle((int)boxX - 5, (int)boxY, textW + 10, 24, COLOR_HEAD);
    DrawText(label.c_str(), (int)boxX, (int)boxY + 5, fontSize, WHITE);
    
    // Draw arrow
    DrawTriangle(
        {screenPos.x, screenPos.y - r - 5},
        {screenPos.x - 6, screenPos.y - r - 15},
        {screenPos.x + 6, screenPos.y - r - 15},
        COLOR_HEAD
    );
}

} // namespace GitVis
