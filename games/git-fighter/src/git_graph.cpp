#include "git_graph.h"
#include <algorithm>
#include <cmath>

namespace GitVis {

// Color palette for branches
static const Color BRANCH_COLORS[] = {
    {100, 200, 255, 255},  // Cyan
    {255, 150, 100, 255},  // Orange
    {150, 255, 100, 255},  // Lime
    {255, 100, 200, 255},  // Pink
    {255, 220, 100, 255},  // Yellow
    {180, 100, 255, 255},  // Purple
    {100, 255, 180, 255},  // Teal
    {255, 100, 100, 255},  // Red
};
static const int NUM_BRANCH_COLORS = 8;

// GraphLayout implementation
GraphLayout::GraphLayout()
    : canvasWidth(1280), canvasHeight(720)
    , nodeSpacingX(120), nodeSpacingY(80)
    , marginLeft(100), marginTop(50), marginRight(50), marginBottom(100) {}

void GraphLayout::SetCanvasSize(int width, int height) {
    canvasWidth = width;
    canvasHeight = height;
}

void GraphLayout::SetNodeSpacing(float x, float y) {
    nodeSpacingX = x;
    nodeSpacingY = y;
}

void GraphLayout::SetMargins(float left, float top, float right, float bottom) {
    marginLeft = left;
    marginTop = top;
    marginRight = right;
    marginBottom = bottom;
}

void GraphLayout::CalculateLanes(std::vector<VisualNode>& nodes,
                                  const std::vector<VisualEdge>& edges) {
    // Simple lane assignment algorithm
    // Assign lanes based on branch topology
    int nextLane = 0;
    std::map<std::string, int> commitLanes;

    // Sort nodes by timestamp (newest first)
    std::sort(nodes.begin(), nodes.end(),
              [](const VisualNode& a, const VisualNode& b) {
                  return a.timestamp > b.timestamp;
              });

    for (auto& node : nodes) {
        // Try to reuse parent's lane
        int assignedLane = -1;
        for (const auto& parentHash : node.parents) {
            if (commitLanes.count(parentHash)) {
                int parentLane = commitLanes[parentHash];
                // Check if this lane is available
                bool available = true;
                for (const auto& other : nodes) {
                    if (other.lane == parentLane &&
                        std::abs(other.position.y - node.position.y) < nodeSpacingY) {
                        available = false;
                        break;
                    }
                }
                if (available && assignedLane == -1) {
                    assignedLane = parentLane;
                }
            }
        }

        // Assign new lane if needed
        if (assignedLane == -1) {
            assignedLane = nextLane++;
        }

        node.lane = assignedLane;
        commitLanes[node.hash] = assignedLane;
    }
}

void GraphLayout::CalculatePositions(std::vector<VisualNode>& nodes) {
    // Assign Y positions based on time (newest at top)
    // Sort by timestamp
    std::sort(nodes.begin(), nodes.end(),
              [](const VisualNode& a, const VisualNode& b) {
                  return a.timestamp > b.timestamp;
              });

    float currentY = marginTop;
    for (auto& node : nodes) {
        node.targetPosition.y = currentY;
        node.targetPosition.x = marginLeft + node.lane * nodeSpacingX;
        currentY += nodeSpacingY;
    }
}

void GraphLayout::OptimizeCrossings(std::vector<VisualNode>& nodes,
                                    const std::vector<VisualEdge>& edges) {
    // Simple crossing minimization (can be improved)
    // For now, just ensure smooth transitions
}

// GitGraphRenderer implementation
GitGraphRenderer::GitGraphRenderer()
    : cameraPos{0, 0}
    , zoomLevel(1.0f)
    , canvasWidth(1280)
    , canvasHeight(720)
    , animating(false)
    , animationTime(0)
    , selectedHash("") {}

GitGraphRenderer::~GitGraphRenderer() = default;

void GitGraphRenderer::Initialize() {
    layout.SetCanvasSize(canvasWidth, canvasHeight);
    layout.SetNodeSpacing(120, 80);
    layout.SetMargins(150, 100, 50, 150);
}

void GitGraphRenderer::Shutdown() {
    nodes.clear();
    edges.clear();
    branches.clear();
    tags.clear();
}

void GitGraphRenderer::AddCommit(const std::string& hash, const std::string& message,
                                  const std::vector<std::string>& parents,
                                  const std::string& author, float timestamp) {
    VisualNode node;
    node.hash = hash;
    node.shortHash = hash.substr(0, 7);
    node.message = message;
    node.author = author;
    node.timestamp = timestamp;
    node.parents = parents;
    node.radius = 20;
    node.type = NodeType::NORMAL;
    node.color = WHITE;
    node.alpha = 0;
    node.scale = 0;
    node.visible = true;
    node.position = {0, 0};
    node.targetPosition = {0, 0};
    node.lane = 0;

    // Determine node type
    if (parents.size() > 1) {
        node.type = NodeType::MERGE;
        node.color = {255, 150, 100, 255};  // Orange for merge
    }

    nodes[hash] = node;

    // Create edges to parents
    for (const auto& parentHash : parents) {
        VisualEdge edge;
        edge.fromHash = hash;
        edge.toHash = parentHash;
        edge.type = EdgeType::PARENT;
        edge.color = {150, 150, 150, 255};
        edge.progress = 0;
        edges.push_back(edge);
    }
}

void GitGraphRenderer::AddBranch(const std::string& name, const std::string& headHash,
                                  bool isRemote) {
    BranchInfo branch;
    branch.name = name;
    branch.headHash = headHash;
    branch.isRemote = isRemote;
    branch.isDetached = false;
    branch.color = GetBranchColor(branches.size() % NUM_BRANCH_COLORS);
    branch.lane = branches.size();

    branches[name] = branch;

    // Mark the commit as a branch tip
    if (nodes.count(headHash)) {
        nodes[headHash].type = NodeType::BRANCH_TIP;
        nodes[headHash].branches.push_back(name);
    }
}

void GitGraphRenderer::AddTag(const std::string& name, const std::string& commitHash) {
    tags[commitHash].push_back(name);

    if (nodes.count(commitHash)) {
        nodes[commitHash].tags.push_back(name);
        nodes[commitHash].type = NodeType::TAG;
    }
}

void GitGraphRenderer::SetHEAD(const std::string& hash) {
    headHash = hash;

    if (nodes.count(hash)) {
        nodes[hash].type = NodeType::HEAD;
    }
}

void GitGraphRenderer::RecalculateLayout() {
    std::vector<VisualNode> nodeList;
    for (auto& pair : nodes) {
        nodeList.push_back(pair.second);
    }

    layout.CalculateLanes(nodeList, edges);
    layout.CalculatePositions(nodeList);
    layout.OptimizeCrossings(nodeList, edges);

    // Update nodes
    for (const auto& node : nodeList) {
        nodes[node.hash].lane = node.lane;
        nodes[node.hash].targetPosition = node.targetPosition;
    }

    // Calculate edge control points for smooth curves
    for (auto& edge : edges) {
        if (nodes.count(edge.fromHash) && nodes.count(edge.toHash)) {
            const auto& from = nodes[edge.fromHash];
            const auto& to = nodes[edge.toHash];

            edge.controlPoints.clear();

            // Simple bezier curve
            Vector2 mid1 = {
                from.targetPosition.x,
                (from.targetPosition.y + to.targetPosition.y) / 2
            };
            Vector2 mid2 = {
                to.targetPosition.x,
                (from.targetPosition.y + to.targetPosition.y) / 2
            };

            edge.controlPoints.push_back(mid1);
            edge.controlPoints.push_back(mid2);
        }
    }
}

Color GitGraphRenderer::GetBranchColor(int lane) {
    return BRANCH_COLORS[lane % NUM_BRANCH_COLORS];
}

void GitGraphRenderer::Draw(int x, int y, int width, int height) {
    // Set scissor mode for clipping
    BeginScissorMode(x, y, width, height);

    // Draw background
    DrawRectangle(x, y, width, height, {30, 30, 40, 255});

    // Draw grid (optional)
    DrawGridBackground(x, y, width, height);

    // Draw edges first (behind nodes)
    DrawEdges();

    // Draw nodes
    DrawNodes();

    // Draw labels
    DrawBranchLabels();
    DrawTagLabels();

    // Draw selected commit details
    if (!selectedHash.empty() && nodes.count(selectedHash)) {
        DrawCommitDetails(nodes[selectedHash], x + 10, y + height - 120);
    }

    EndScissorMode();

    // Draw border
    DrawRectangleLines(x, y, width, height, {100, 150, 200, 255});
}

void GitGraphRenderer::DrawGridBackground(int x, int y, int width, int height) {
    // Draw subtle grid lines
    Color gridColor = {50, 50, 60, 100};
    float gridSize = 50 * zoomLevel;

    float offsetX = fmod(cameraPos.x, gridSize);
    float offsetY = fmod(cameraPos.y, gridSize);

    for (float gx = x - offsetX; gx < x + width; gx += gridSize) {
        DrawLineV({gx, (float)y}, {gx, (float)(y + height)}, gridColor);
    }

    for (float gy = y - offsetY; gy < y + height; gy += gridSize) {
        DrawLineV({(float)x, gy}, {(float)(x + width), gy}, gridColor);
    }
}

void GitGraphRenderer::DrawEdges() {
    for (const auto& edge : edges) {
        if (!nodes.count(edge.fromHash) || !nodes.count(edge.toHash)) continue;

        const auto& from = nodes[edge.fromHash];
        const auto& to = nodes[edge.toHash];

        if (!from.visible || !to.visible) continue;

        Vector2 fromPos = WorldToScreen(from.position);
        Vector2 toPos = WorldToScreen(to.position);

        Color edgeColor = edge.color;
        edgeColor.a = (unsigned char)(255 * from.alpha * to.alpha);

        // Draw curved line
        if (edge.controlPoints.size() >= 2) {
            Vector2 p1 = WorldToScreen(edge.controlPoints[0]);
            Vector2 p2 = WorldToScreen(edge.controlPoints[1]);

            // Draw bezier curve as line segments
            DrawBezierLine(fromPos, p1, p2, toPos, 3, edgeColor);
        } else {
            DrawLineEx(fromPos, toPos, 3, edgeColor);
        }

        // Animated progress for new edges
        if (edge.progress < 1.0f) {
            float animX = fromPos.x + (toPos.x - fromPos.x) * edge.progress;
            float animY = fromPos.y + (toPos.y - fromPos.y) * edge.progress;
            DrawCircle(animX, animY, 5, {255, 255, 100, 255});
        }
    }
}

void GitGraphRenderer::DrawBezierLine(Vector2 p0, Vector2 p1, Vector2 p2, Vector2 p3,
                                       float thick, Color color) {
    const int segments = 20;
    Vector2 prev = p0;

    for (int i = 1; i <= segments; i++) {
        float t = (float)i / segments;
        float mt = 1 - t;

        // Cubic bezier
        Vector2 curr = {
            mt * mt * mt * p0.x + 3 * mt * mt * t * p1.x +
            3 * mt * t * t * p2.x + t * t * t * p3.x,
            mt * mt * mt * p0.y + 3 * mt * mt * t * p1.y +
            3 * mt * t * t * p2.y + t * t * t * p3.y
        };

        DrawLineEx(prev, curr, thick, color);
        prev = curr;
    }
}

void GitGraphRenderer::DrawNodes() {
    for (auto& pair : nodes) {
        auto& node = pair.second;
        if (!node.visible) continue;

        Vector2 pos = WorldToScreen(node.position);
        float r = node.radius * node.scale * zoomLevel;

        Color nodeColor = node.color;
        nodeColor.a = (unsigned char)(255 * node.alpha);

        // Draw glow for HEAD
        if (node.type == NodeType::HEAD || node.hash == headHash) {
            DrawCircle(pos.x, pos.y, r + 8, {255, 200, 100, 100});
            DrawCircle(pos.x, pos.y, r + 4, {255, 200, 100, 150});
        }

        // Draw node circle
        DrawCircle(pos.x, pos.y, r, nodeColor);
        DrawCircleLines(pos.x, pos.y, r, {200, 200, 200, (unsigned char)(255 * node.alpha)});

        // Draw inner circle
        DrawCircle(pos.x, pos.y, r * 0.7f,
                   {40, 44, 52, (unsigned char)(255 * node.alpha)});

        // Draw hash abbreviation
        float fontSize = 12 * zoomLevel;
        int textWidth = MeasureText(node.shortHash.c_str(), fontSize);
        DrawText(node.shortHash.c_str(),
                 pos.x - textWidth / 2,
                 pos.y - fontSize / 2,
                 fontSize,
                 {200, 200, 200, (unsigned char)(255 * node.alpha)});

        // Draw selection ring
        if (node.hash == selectedHash) {
            DrawRing(pos, r + 5, r + 8, 0, 360, 32,
                     {100, 200, 255, (unsigned char)(200 * node.alpha)});
        }
    }
}

void GitGraphRenderer::DrawBranchLabels() {
    for (const auto& pair : branches) {
        const auto& branch = pair.second;
        if (!nodes.count(branch.headHash)) continue;

        const auto& node = nodes[branch.headHash];
        if (!node.visible) continue;

        Vector2 pos = WorldToScreen(node.position);

        // Draw branch label box
        std::string label = branch.isRemote ? "🌐 " + branch.name : "🏠 " + branch.name;
        int textWidth = MeasureText(label.c_str(), 14);
        int boxWidth = textWidth + 16;
        int boxHeight = 24;

        float boxX = pos.x + node.radius * zoomLevel + 10;
        float boxY = pos.y - boxHeight / 2;

        DrawRectangle(boxX, boxY, boxWidth, boxHeight, branch.color);
        DrawRectangleLines(boxX, boxY, boxWidth, boxHeight, WHITE);

        DrawText(label.c_str(), boxX + 8, boxY + 5, 14, WHITE);
    }

    // Draw HEAD label
    if (!headHash.empty() && nodes.count(headHash)) {
        const auto& node = nodes[headHash];
        Vector2 pos = WorldToScreen(node.position);

        std::string headLabel = "➤ HEAD";
        int textWidth = MeasureText(headLabel.c_str(), 14);

        float boxX = pos.x - textWidth / 2 - 8;
        float boxY = pos.y - node.radius * zoomLevel - 35;

        DrawRectangle(boxX, boxY, textWidth + 16, 24, {255, 100, 100, 255});
        DrawText(headLabel.c_str(), boxX + 8, boxY + 5, 14, WHITE);
    }
}

void GitGraphRenderer::DrawTagLabels() {
    for (const auto& pair : tags) {
        if (!nodes.count(pair.first)) continue;

        const auto& node = nodes[pair.first];
        if (!node.visible) continue;

        Vector2 pos = WorldToScreen(node.position);
        float startX = pos.x - node.radius * zoomLevel - 10;
        float y = pos.y;

        for (const auto& tag : pair.second) {
            std::string label = "🏷️ " + tag;
            int textWidth = MeasureText(label.c_str(), 12);
            int boxWidth = textWidth + 12;
            int boxHeight = 20;

            float boxX = startX - boxWidth;

            DrawRectangle(boxX, y - boxHeight / 2, boxWidth, boxHeight,
                          {255, 220, 100, 230});
            DrawText(label.c_str(), boxX + 6, y - boxHeight / 2 + 4, 12,
                     {80, 60, 20, 255});

            startX -= boxWidth + 5;
        }
    }
}

void GitGraphRenderer::DrawCommitDetails(const VisualNode& node, int x, int y) {
    // Draw info panel for selected commit
    int panelWidth = 400;
    int panelHeight = 100;

    DrawRectangle(x, y, panelWidth, panelHeight, {40, 44, 52, 240});
    DrawRectangleLines(x, y, panelWidth, panelHeight, {100, 150, 200, 255});

    DrawText(TextFormat("Commit: %s", node.hash.c_str()), x + 10, y + 10, 16, WHITE);
    DrawText(TextFormat("Author: %s", node.author.c_str()), x + 10, y + 32, 14, LIGHTGRAY);
    DrawText(TextFormat("Message: %s", node.message.c_str()), x + 10, y + 54, 14, LIGHTGRAY);

    if (!node.branches.empty()) {
        std::string branchText = "Branches: ";
        for (const auto& b : node.branches) branchText += b + " ";
        DrawText(branchText.c_str(), x + 10, y + 76, 12, {100, 200, 255, 255});
    }
}

Vector2 GitGraphRenderer::WorldToScreen(Vector2 worldPos) {
    return {
        worldPos.x * zoomLevel - cameraPos.x,
        worldPos.y * zoomLevel - cameraPos.y
    };
}

Vector2 GitGraphRenderer::ScreenToWorld(Vector2 screenPos) {
    return {
        (screenPos.x + cameraPos.x) / zoomLevel,
        (screenPos.y + cameraPos.y) / zoomLevel
    };
}

void GitGraphRenderer::UpdateAnimations(float deltaTime) {
    // Animate nodes to their target positions
    float lerpSpeed = 5.0f * deltaTime;

    for (auto& pair : nodes) {
        auto& node = pair.second;

        // Position lerp
        node.position.x += (node.targetPosition.x - node.position.x) * lerpSpeed;
        node.position.y += (node.targetPosition.y - node.position.y) * lerpSpeed;

        // Fade in
        if (node.alpha < 1.0f) {
            node.alpha += deltaTime * 2;
            if (node.alpha > 1.0f) node.alpha = 1.0f;
        }

        // Scale in
        if (node.scale < 1.0f) {
            node.scale += deltaTime * 3;
            if (node.scale > 1.0f) node.scale = 1.0f;
        }
    }

    // Animate edges
    for (auto& edge : edges) {
        if (edge.progress < 1.0f) {
            edge.progress += deltaTime;
            if (edge.progress > 1.0f) edge.progress = 1.0f;
        }
    }
}

void GitGraphRenderer::StartCommitAnimation(const std::string& newCommitHash) {
    animating = true;
    animationTime = 0;
    animatingCommit = newCommitHash;

    if (nodes.count(newCommitHash)) {
        nodes[newCommitHash].scale = 0;
        nodes[newCommitHash].alpha = 0;
    }
}

void GitGraphRenderer::Pan(Vector2 delta) {
    cameraPos.x -= delta.x;
    cameraPos.y -= delta.y;
}

void GitGraphRenderer::Zoom(float factor) {
    zoomLevel *= factor;
    if (zoomLevel < 0.3f) zoomLevel = 0.3f;
    if (zoomLevel > 3.0f) zoomLevel = 3.0f;
}

void GitGraphRenderer::SelectNode(const std::string& hash) {
    selectedHash = hash;
}

VisualNode* GitGraphRenderer::GetNodeAtPosition(Vector2 screenPos) {
    Vector2 worldPos = ScreenToWorld(screenPos);

    for (auto& pair : nodes) {
        auto& node = pair.second;
        if (!node.visible) continue;

        float dx = worldPos.x - node.position.x;
        float dy = worldPos.y - node.position.y;
        float dist = sqrt(dx * dx + dy * dy);

        if (dist < node.radius) {
            return &node;
        }
    }

    return nullptr;
}

bool GitGraphRenderer::IsAnimating() const {
    return animating;
}

std::string GitGraphRenderer::GetSelectedCommit() const {
    return selectedHash;
}

void GitGraphRenderer::CenterOnCommit(const std::string& hash) {
    if (!nodes.count(hash)) return;

    const auto& node = nodes[hash];
    cameraPos.x = node.position.x * zoomLevel - canvasWidth / 2;
    cameraPos.y = node.position.y * zoomLevel - canvasHeight / 2;
}

void GitGraphRenderer::ZoomToFit() {
    if (nodes.empty()) return;

    float minX = 99999, minY = 99999, maxX = -99999, maxY = -99999;

    for (const auto& pair : nodes) {
        const auto& node = pair.second;
        if (node.position.x < minX) minX = node.position.x;
        if (node.position.y < minY) minY = node.position.y;
        if (node.position.x > maxX) maxX = node.position.x;
        if (node.position.y > maxY) maxY = node.position.y;
    }

    float width = maxX - minX + 300;
    float height = maxY - minY + 200;

    float zoomX = canvasWidth / width;
    float zoomY = canvasHeight / height;

    zoomLevel = (zoomX < zoomY) ? zoomX : zoomY;
    if (zoomLevel > 1.5f) zoomLevel = 1.5f;
    if (zoomLevel < 0.3f) zoomLevel = 0.3f;

    cameraPos.x = minX * zoomLevel - 150;
    cameraPos.y = minY * zoomLevel - 100;
}

// CommitGraphBuilder implementation
GitGraphRenderer* CommitGraphBuilder::BuildFromSimulatedData() {
    GitGraphRenderer* graph = new GitGraphRenderer();
    graph->Initialize();

    // Create simulated commit history
    float time = 1700000000;

    // Initial commit
    graph->AddCommit("a1b2c3d", "Initial commit: 项目初始化", {}, "小王", time);
    time -= 3600;

    // Add feature
    graph->AddCommit("e4f5g6h", "Add main.cpp", {"a1b2c3d"}, "小王", time);
    time -= 3600;

    // Create branch (simulated)
    graph->AddCommit("i7j8k9l", "Add config files", {"e4f5g6h"}, "小王", time);
    time -= 1800;

    // Parallel branch
    graph->AddCommit("m0n1o2p", "Feature: login module", {"e4f5g6h"}, "小李", time);
    time -= 3600;

    // Merge
    graph->AddCommit("q3r4s5t", "Merge branch 'feature/login'",
                     {"i7j8k9l", "m0n1o2p"}, "小王", time);
    time -= 3600;

    // More commits
    graph->AddCommit("u6v7w8x", "Fix: bug in login", {"q3r4s5t"}, "小王", time);
    time -= 1800;

    graph->AddCommit("y9z0a1b", "Update README", {"u6v7w8x"}, "小王", time);

    // Add branches
    graph->AddBranch("main", "y9z0a1b");
    graph->AddBranch("feature/payment", "u6v7w8x");
    graph->AddBranch("origin/main", "y9z0a1b", true);

    // Add tags
    graph->AddTag("v0.1.0", "a1b2c3d");
    graph->AddTag("v0.2.0", "q3r4s5t");

    // Set HEAD
    graph->SetHEAD("y9z0a1b");

    // Calculate layout
    graph->RecalculateLayout();
    graph->ZoomToFit();

    return graph;
}

} // namespace GitVis
