#pragma once
#include "raylib.h"
#include <vector>
#include <string>
#include <memory>
#include <map>

namespace GitVis {

// Visual styles
enum class NodeType {
    NORMAL,      // Regular commit
    MERGE,       // Merge commit (multiple parents)
    HEAD,        // Current HEAD
    BRANCH_TIP,  // Branch tip
    TAG          // Tagged commit
};

enum class EdgeType {
    PARENT,      // Standard parent relationship
    MERGE,       // Merge line
    BRANCH       // Branch line
};

// Visual commit node
struct VisualNode {
    std::string hash;
    std::string shortHash;
    std::string message;
    std::string author;
    float timestamp;
    
    // Visual properties
    Vector2 position;
    Vector2 targetPosition;  // For animation
    float radius;
    NodeType type;
    Color color;
    
    // Branch info
    std::vector<std::string> branches;  // Branches pointing to this commit
    std::vector<std::string> tags;      // Tags on this commit
    
    // Graph structure
    std::vector<std::string> parents;
    std::vector<std::string> children;
    int lane;  // Horizontal lane for layout
    
    // Animation
    float alpha;
    float scale;
    bool visible;
};

// Edge between nodes
struct VisualEdge {
    std::string fromHash;
    std::string toHash;
    EdgeType type;
    Color color;
    std::vector<Vector2> controlPoints;  // Bezier control points
    float progress;  // Animation progress (0-1)
};

// Branch visual info
struct BranchInfo {
    std::string name;
    std::string headHash;
    Color color;
    int lane;
    bool isRemote;
    bool isDetached;
};

// Graph layout engine
class GraphLayout {
public:
    GraphLayout();
    
    // Layout configuration
    void SetCanvasSize(int width, int height);
    void SetNodeSpacing(float x, float y);
    void SetMargins(float left, float top, float right, float bottom);
    
    // Layout algorithms
    void CalculateLanes(std::vector<VisualNode>& nodes, 
                       const std::vector<VisualEdge>& edges);
    void CalculatePositions(std::vector<VisualNode>& nodes);
    void OptimizeCrossings(std::vector<VisualNode>& nodes,
                          const std::vector<VisualEdge>& edges);
    
private:
    int canvasWidth, canvasHeight;
    float nodeSpacingX, nodeSpacingY;
    float marginLeft, marginTop, marginRight, marginBottom;
};

// Main graph renderer
class GitGraphRenderer {
public:
    GitGraphRenderer();
    ~GitGraphRenderer();
    
    // Initialization
    void Initialize();
    void Shutdown();
    
    // Data management
    void LoadFromRepo(const std::string& repoPath);
    void AddCommit(const std::string& hash, const std::string& message,
                   const std::vector<std::string>& parents,
                   const std::string& author, float timestamp);
    void AddBranch(const std::string& name, const std::string& headHash,
                   bool isRemote = false);
    void AddTag(const std::string& name, const std::string& commitHash);
    void SetHEAD(const std::string& hash);
    
    // Layout
    void RecalculateLayout();
    void CenterOnCommit(const std::string& hash);
    void ZoomToFit();
    
    // Animation
    void StartCommitAnimation(const std::string& newCommitHash);
    void StartBranchAnimation(const std::string& branchName);
    void StartMergeAnimation(const std::string& fromBranch, 
                            const std::string& toBranch);
    void UpdateAnimations(float deltaTime);
    
    // Rendering
    void Draw(int x, int y, int width, int height);
    void DrawMinimap(int x, int y, int size);
    
    // Interaction
    VisualNode* GetNodeAtPosition(Vector2 pos);
    void SelectNode(const std::string& hash);
    void Pan(Vector2 delta);
    void Zoom(float factor);
    
    // State
    bool IsAnimating() const;
    std::string GetSelectedCommit() const;
    
private:
    void DrawGridBackground(int x, int y, int width, int height);
    void DrawEdges();
    void DrawNodes();
    void DrawBranchLabels();
    void DrawTagLabels();
    void DrawCommitDetails(const VisualNode& node, int x, int y);
    void DrawConnection(const VisualEdge& edge);
    void DrawBezierLine(Vector2 p0, Vector2 p1, Vector2 p2, Vector2 p3,
                       float thick, Color color);
    
    Color GetBranchColor(int lane);
    Vector2 WorldToScreen(Vector2 worldPos);
    Vector2 ScreenToWorld(Vector2 screenPos);
    
    std::map<std::string, VisualNode> nodes;
    std::vector<VisualEdge> edges;
    std::map<std::string, BranchInfo> branches;
    std::map<std::string, std::vector<std::string>> tags;
    
    std::string headHash;
    std::string selectedHash;
    
    // Viewport
    Vector2 cameraPos;
    float zoomLevel;
    int canvasWidth, canvasHeight;
    
    // Layout engine
    GraphLayout layout;
    
    // Animation state
    bool animating;
    float animationTime;
    std::string animatingCommit;
};

// Commit graph builder from libgit2
class CommitGraphBuilder {
public:
    static GitGraphRenderer* BuildFromRepo(const std::string& repoPath);
    static GitGraphRenderer* BuildFromSimulatedData();  // For testing
};

} // namespace GitVis
