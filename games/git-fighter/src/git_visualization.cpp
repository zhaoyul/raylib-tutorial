#include "git_visualization.h"
#include <cmath>
#include <algorithm>
#include <set>
#include <map>
#include <iostream>

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
    
    // Horizontal timeline layout with parallel branch lines
    // from = child commit (newer)
    // to = parent commit (older)
    
    if (std::abs(fromLane - toLane) < 0.5f) {
        // Same lane - straight horizontal line
        waypoints.push_back(fromPos);
        waypoints.push_back(toPos);
    } else {
        // Different lanes - branch/merge connection
        // Create a path that goes horizontally then curves to connect
        float dx = fromPos.x - toPos.x;
        float dy = fromPos.y - toPos.y;
        
        // Branch point: where the branch diverges from parent
        // Go left from child, then curve to parent's Y at parent's X
        
        // First go horizontally from child
        float midX = toPos.x + dx * 0.3f;  // Curve starts near the parent
        
        // Add intermediate points for smooth curve
        waypoints.push_back(fromPos);
        
        // Curve control point - smooth transition between lanes
        Vector2 cp1 = {midX, fromPos.y};  // Stay on child's lane
        Vector2 cp2 = {midX, toPos.y};    // Move to parent's lane
        
        // Generate curve segment
        for (int i = 1; i <= 20; i++) {
            float t = i / 20.0f;
            float mt = 1 - t;
            
            // Cubic bezier from midX point to parent
            Vector2 p = {
                mt*mt*mt*fromPos.x + 3*mt*mt*t*cp1.x + 3*mt*t*t*cp2.x + t*t*t*toPos.x,
                mt*mt*mt*fromPos.y + 3*mt*mt*t*cp1.y + 3*mt*t*t*cp2.y + t*t*t*toPos.y
            };
            waypoints.push_back(p);
        }
        
        waypoints.push_back(toPos);
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
    // Center the view on the typical node area (x=400, y=250)
    viewport.SetOffset({400, 250});
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

void CommitGraphPanel::SetCurrentBranch(const std::string& branch) {
    currentBranch = branch;
}

void CommitGraphPanel::Clear() {
    nodes.clear();
    edges.clear();
    branchColors.clear();
    headHash.clear();
    selectedHash.clear();
}

void CommitGraphPanel::RecalculateLayout() {
    if (nodes.empty()) return;
    
    // Build parent-child relationships
    for (auto& pair : nodes) {
        auto& node = pair.second;
        node.children.clear();
    }
    for (auto& pair : nodes) {
        auto& node = pair.second;
        for (const auto& parentHash : node.parents) {
            if (nodes.count(parentHash)) {
                nodes[parentHash].children.push_back(pair.first);
            }
        }
    }
    
    // Topological sort (pre-order traversal)
    // Start from root commits (no parents) and traverse children first
    std::vector<std::string> sortedHashes;
    std::set<std::string> visited;
    
    // Find root commits (those with no parents in the graph)
    std::vector<std::string> roots;
    for (const auto& pair : nodes) {
        bool hasParentInGraph = false;
        for (const auto& parent : pair.second.parents) {
            if (nodes.count(parent)) {
                hasParentInGraph = true;
                break;
            }
        }
        if (!hasParentInGraph) {
            roots.push_back(pair.first);
        }
    }
    
    // DFS to get pre-order traversal
    std::function<void(const std::string&)> dfs = [&](const std::string& hash) {
        if (visited.count(hash)) return;
        visited.insert(hash);
        sortedHashes.push_back(hash);
        
        // Visit children (follow chronological order for stability)
        auto children = nodes[hash].children;
        std::sort(children.begin(), children.end(),
            [this](const std::string& a, const std::string& b) {
                return nodes[a].timestamp < nodes[b].timestamp;
            });
        
        for (const auto& child : children) {
            dfs(child);
        }
    };
    
    // Sort roots by timestamp for consistent ordering
    std::sort(roots.begin(), roots.end(),
        [this](const std::string& a, const std::string& b) {
            return nodes[a].timestamp < nodes[b].timestamp;
        });
    
    for (const auto& root : roots) {
        dfs(root);
    }
    
    // Group commits by branch
    // First, identify the main branch (longest chain from root to HEAD)
    std::set<std::string> mainBranchCommits;
    {
        std::string current = headHash.empty() ? sortedHashes.back() : headHash;
        while (!current.empty() && nodes.count(current)) {
            mainBranchCommits.insert(current);
            const auto& node = nodes[current];
            current = node.parents.empty() ? "" : node.parents[0];
        }
    }
    
    // Assign each commit to a branch
    std::map<std::string, std::string> commitToBranch;  // commit -> branch name
    std::map<std::string, std::vector<std::string>> branchCommits;  // branch -> ordered commits
    
    // Main branch first
    std::vector<std::string> mainCommits;
    for (const auto& hash : sortedHashes) {
        if (mainBranchCommits.count(hash)) {
            mainCommits.push_back(hash);
            commitToBranch[hash] = "main";
        }
    }
    branchCommits["main"] = mainCommits;
    
    // Find feature branches (commits not on main that have branch labels)
    int branchId = 1;
    for (const auto& hash : sortedHashes) {
        if (commitToBranch.count(hash)) continue;  // Already assigned
        
        const auto& node = nodes[hash];
        
        // Check if this commit has branch labels
        std::string branchName;
        for (const auto& b : node.branches) {
            if (b != "main" && b.find("origin/") != 0) {
                branchName = b;
                break;
            }
        }
        
        if (!branchName.empty()) {
            // This is the tip of a feature branch
            // Walk back to find all commits in this branch
            std::vector<std::string> branchCommitsList;
            std::string current = hash;
            while (!current.empty() && nodes.count(current) && !commitToBranch.count(current)) {
                branchCommitsList.push_back(current);
                commitToBranch[current] = branchName;
                const auto& n = nodes[current];
                current = n.parents.empty() ? "" : n.parents[0];
            }
            // Reverse to get oldest first
            std::reverse(branchCommitsList.begin(), branchCommitsList.end());
            branchCommits[branchName] = branchCommitsList;
        }
    }
    
    // Handle remaining unassigned commits (orphans) - assign to nearby branch
    for (const auto& hash : sortedHashes) {
        if (commitToBranch.count(hash)) continue;
        
        // Find which branch this commit connects to
        std::string bestBranch = "main";
        for (const auto& parent : nodes[hash].parents) {
            if (commitToBranch.count(parent)) {
                bestBranch = commitToBranch[parent];
                break;
            }
        }
        commitToBranch[hash] = bestBranch;
        branchCommits[bestBranch].push_back(hash);
    }
    
    // Assign lanes to branches
    // Sort branches alphabetically for consistent ordering
    // b1 -> lane 1 (below main), b2 -> lane 2, b3 -> lane 3, etc.
    std::vector<std::string> branchNames;
    for (const auto& pair : branchCommits) {
        if (pair.first == "main") continue;
        branchNames.push_back(pair.first);
    }
    
    // Sort alphabetically (b1, b2, b3...)
    std::sort(branchNames.begin(), branchNames.end());
    
    std::map<std::string, int> branchLane;
    branchLane["main"] = 0;
    
    // Assign lanes in alphabetical order
    // First alphabetically (b1) -> lane 1 (closest to main)
    // Last alphabetically (b3) -> lane N (farthest from main)
    int nextLane = 1;
    for (const auto& branchName : branchNames) {
        branchLane[branchName] = nextLane++;
    }
    
    // Assign lanes to commits
    for (const auto& hash : sortedHashes) {
        const std::string& branch = commitToBranch[hash];
        nodes[hash].lane = branchLane[branch];
    }
    
    // Calculate positions
    // X: Each branch starts from its divergence point
    // Y: Branch-based rows with larger spacing
    const float X_SPACING = 120.0f;
    const float Y_CENTER = 300.0f;    // Main branch position
    const float Y_SPACING = 150.0f;   // Increased row height
    
    // Step 1: Assign X to main branch commits first
    std::map<std::string, float> commitX;
    float mainX = 150.0f;
    for (const auto& hash : sortedHashes) {
        if (commitToBranch[hash] == "main") {
            commitX[hash] = mainX;
            mainX += X_SPACING;
        }
    }
    
    // Step 2: Assign X to feature branches (start from divergence point)
    for (const auto& pair : branchCommits) {
        const std::string& branchName = pair.first;
        if (branchName == "main" || pair.second.empty()) continue;
        
        // Find divergence point
        const std::string& firstCommit = pair.second[0];
        const auto& node = nodes[firstCommit];
        
        float startX = 150.0f;
        if (!node.parents.empty()) {
            const std::string& parentHash = node.parents[0];
            if (commitX.count(parentHash)) {
                startX = commitX[parentHash] + X_SPACING;
            }
        }
        
        // Assign X to this branch's commits
        float x = startX;
        for (const auto& hash : pair.second) {
            commitX[hash] = x;
            x += X_SPACING;
        }
    }
    
    // Apply positions
    for (const auto& hash : sortedHashes) {
        auto& node = nodes[hash];
        int lane = node.lane;
        node.targetPos = {commitX[hash], Y_CENTER + lane * Y_SPACING};
        node.position = node.targetPos;
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
    Vector2 localPos = {screenPos.x - bounds.x, screenPos.y - bounds.y};
    Vector2 worldPos = viewport.ScreenToWorld(localPos);
    
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

void CommitGraphPanel::DeselectNode() {
    if (!selectedHash.empty()) {
        if (nodes.count(selectedHash)) {
            nodes[selectedHash].selected = false;
        }
        selectedHash.clear();
        // Notify with empty callback or special signal
        if (onNodeSelected) {
            // Pass a dummy node with empty hash to indicate deselection
            CommitNode dummy;
            dummy.hash = "";
            onNodeSelected(dummy);
        }
    }
}

void CommitGraphPanel::CenterOnNode(const std::string& hash) {
    if (!nodes.count(hash)) return;
    
    viewport.OnDragEnd();  // Reset velocity
    // TODO: Actually center viewport on the node
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
    
    // Track drag state
    static bool startedDragInPanel = false;
    static Vector2 dragStartPos = {0, 0};
    const float CLICK_THRESHOLD = 5.0f;  // Max movement to be considered a click
    
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (CheckCollisionPointRec(mousePos, bounds)) {
            auto* node = GetNodeAt(mousePos);
            if (node) {
                SelectNode(node->hash);
                startedDragInPanel = false;
            } else {
                viewport.OnDragStart(localMouse);
                startedDragInPanel = true;
                dragStartPos = mousePos;
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
            // Check if this was a click (minimal movement)
            float dx = dragStartPos.x - mousePos.x;
            float dy = dragStartPos.y - mousePos.y;
            float dragDist = sqrtf(dx*dx + dy*dy);
            if (dragDist < CLICK_THRESHOLD) {
                // Click on empty space - deselect
                if (CheckCollisionPointRec(mousePos, bounds)) {
                    auto* node = GetNodeAt(mousePos);
                    if (!node) {
                        DeselectNode();
                    }
                }
            }
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
    
    // Edges (with bounds offset)
    DrawEdges();
    
    // Nodes (with bounds offset)
    DrawNodes();
    
    // Labels (with bounds offset)
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
        
        Vector2 fromView = viewport.WorldToScreen(from.position);
        Vector2 toView = viewport.WorldToScreen(to.position);
        // Convert to screen coordinates
        Vector2 fromScreen = {fromView.x + bounds.x, fromView.y + bounds.y};
        Vector2 toScreen = {toView.x + bounds.x, toView.y + bounds.y};
        
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
        
        Vector2 viewPos = viewport.WorldToScreen(node.position);
        // Convert to screen coordinates (add panel bounds)
        Vector2 screenPos = {viewPos.x + bounds.x, viewPos.y + bounds.y};
        
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
    // Draw branch labels above commits for horizontal timeline layout
    for (const auto& pair : nodes) {
        const auto& node = pair.second;
        if (node.branches.empty()) continue;
        
        Vector2 viewPos = viewport.WorldToScreen(node.position);
        // Convert to screen coordinates
        Vector2 screenPos = {viewPos.x + bounds.x, viewPos.y + bounds.y};
        float r = node.radius * node.scale;
        
        // Position labels above the node
        float labelX = screenPos.x;
        float labelY = screenPos.y - r - 25;
        
        // Remove duplicate branch names and mark current branch with *
        std::set<std::string> uniqueBranches;
        std::vector<std::string> displayLabels;
        
        for (const auto& branch : node.branches) {
            if (uniqueBranches.insert(branch).second) {
                // New unique branch
                std::string label;
                if (branch == currentBranch) {
                    label = " *" + branch + " ";
                } else {
                    label = " " + branch + " ";
                }
                displayLabels.push_back(label);
            }
        }
        
        if (displayLabels.empty()) continue;
        
        // Calculate total width to center labels
        float totalWidth = 0;
        for (const auto& label : displayLabels) {
            totalWidth += MeasureText(label.c_str(), 12) + 15;
        }
        totalWidth -= 15;  // Remove last spacing
        
        labelX -= totalWidth / 2;  // Center the labels
        
        for (size_t i = 0; i < displayLabels.size(); i++) {
            // Find original branch name for color lookup
            std::string originalBranch;
            for (const auto& b : node.branches) {
                std::string testLabel = (b == currentBranch) ? " *" + b + " " : " " + b + " ";
                if (testLabel == displayLabels[i]) {
                    originalBranch = b;
                    break;
                }
            }
            if (originalBranch.empty()) originalBranch = node.branches[i];
            
            Color branchColor = branchColors.count(originalBranch) ? branchColors[originalBranch] : GRAY;
            
            const std::string& label = displayLabels[i];
            int fontSize = 12;
            int textW = MeasureText(label.c_str(), fontSize);
            int tagHeight = 18;
            
            // Draw tag background with rounded corners
            Rectangle tagRect = {labelX, labelY - tagHeight/2, (float)(textW + 8), (float)tagHeight};
            DrawRectangleRounded(tagRect, 0.3f, 4, branchColor);
            
            // Draw tag border
            DrawRectangleRoundedLines(tagRect, 0.3f, 4, {255, 255, 255, 100});
            
            // Draw text
            DrawText(label.c_str(), (int)(labelX + 4), (int)(labelY - fontSize/2), fontSize, WHITE);
            
            // Draw small triangle pointing down to node
            float triX = labelX + (textW + 8) / 2;
            Vector2 p1 = {triX - 4, labelY + tagHeight/2};
            Vector2 p2 = {triX + 4, labelY + tagHeight/2};
            Vector2 p3 = {triX, labelY + tagHeight/2 + 6};
            DrawTriangle(p1, p2, p3, branchColor);
            
            labelX += textW + 15;
        }
    }
}

void CommitGraphPanel::DrawHEADIndicator() {
    if (headHash.empty() || !nodes.count(headHash)) return;
    
    const auto& head = nodes[headHash];
    Vector2 viewPos = viewport.WorldToScreen(head.position);
    // Convert to screen coordinates
    Vector2 screenPos = {viewPos.x + bounds.x, viewPos.y + bounds.y};
    // Fixed node size
    float r = head.radius * head.scale;
    
    std::string label = " HEAD ";
    int fontSize = 12;
    int textW = MeasureText(label.c_str(), fontSize);
    int tagHeight = 20;
    
    // Position below the node for horizontal layout
    float boxX = screenPos.x - (textW + 12) / 2;
    float boxY = screenPos.y + r + 15;
    
    // Draw HEAD tag with rounded corners (similar to branch labels)
    Rectangle tagRect = {boxX, boxY, (float)(textW + 12), (float)tagHeight};
    DrawRectangleRounded(tagRect, 0.3f, 4, COLOR_HEAD);
    DrawRectangleRoundedLines(tagRect, 0.3f, 4, {255, 255, 255, 100});
    
    DrawText(label.c_str(), (int)(boxX + 6), (int)(boxY + 4), fontSize, WHITE);
    
    // Draw small triangle pointing up to node
    float triX = screenPos.x;
    Vector2 p1 = {triX - 4, boxY};
    Vector2 p2 = {triX + 4, boxY};
    Vector2 p3 = {triX, boxY - 6};
    DrawTriangle(p1, p2, p3, COLOR_HEAD);
}

} // namespace GitVis
