#pragma once
#include "raylib.h"
#include <vector>
#include <string>
#include <map>
#include <functional>
#include <memory>

// Forward declaration
class GitWrapper;

namespace GitVis {

// Animation types
enum class AnimType {
    NONE,
    COMMIT_APPEAR,      // New commit grows from parent
    BRANCH_EXTEND,      // Branch line extends
    MERGE_FLOW,         // Merge lines converge
    NODE_SELECT,        // Node selection pulse
    HOVER_GLOW          // Hover glow effect
};

// Physics-based spring animation
struct SpringAnim {
    float position;
    float velocity;
    float target;
    float stiffness;
    float damping;
    
    void Update(float deltaTime);
    bool IsSettled(float threshold = 0.1f) const;
};

// Git object types for internal structure view
enum class GitObjectType {
    COMMIT,
    TREE,
    BLOB,
    TAG
};

struct GitObject {
    std::string hash;
    std::string shortHash;
    GitObjectType type;
    std::string content;
    std::vector<std::string> children;  // For tree: blobs and subtrees
    std::vector<std::string> parents;   // For commit
    Vector2 position;
    Vector2 targetPos;
    float scale;
    float alpha;
    bool expanded;
    
    // Visual
    Color GetColor() const;
    const char* GetIcon() const;
    const char* GetLabel() const;
};

// Draggable viewport with spring physics
class DraggableView {
public:
    DraggableView();
    
    void Update(float deltaTime);
    void Draw() const;
    
    // Viewport control
    void SetBounds(float minX, float minY, float maxX, float maxY);
    void SetViewSize(float width, float height);
    
    // Interaction
    void OnDragStart(Vector2 pos);
    void OnDrag(Vector2 pos);
    void OnDragEnd();
    void OnZoom(float factor, Vector2 center);
    
    // Coordinate transform
    Vector2 WorldToScreen(Vector2 worldPos) const;
    Vector2 ScreenToWorld(Vector2 screenPos) const;
    
    // Accessors
    float GetZoom() const { return zoom; }
    float GetViewWidth() const { return viewWidth; }
    float GetViewHeight() const { return viewHeight; }
    
    // Spring physics for bounce-back
    void ApplySpringForce();
    
private:
    Vector2 offset;
    Vector2 velocity;
    float zoom;
    
    // Bounds
    float boundsMinX, boundsMinY;
    float boundsMaxX, boundsMaxY;
    float viewWidth, viewHeight;
    
    // Spring params
    float springStiffness;
    float springDamping;
    
    // Drag state
    bool isDragging;
    Vector2 dragStartPos;
    Vector2 dragStartOffset;
};

// Enhanced commit node
struct CommitNode {
    std::string hash;
    std::string shortHash;
    std::string message;
    std::string author;
    std::string email;
    std::string date;
    int64_t timestamp;
    
    // Position
    Vector2 position;
    Vector2 targetPos;
    float lane;  // Horizontal lane (can be fractional for smooth animation)
    
    // Visual state
    float radius;
    float targetRadius;
    float scale;
    float alpha;
    float glowIntensity;
    
    // Selection
    bool selected;
    bool hovered;
    
    // Connections
    std::vector<std::string> parents;
    std::vector<std::string> children;
    
    // Branches & tags
    std::vector<std::string> branches;
    std::vector<std::string> tags;
    
    // Animation
    SpringAnim springX;
    SpringAnim springY;
    float animProgress;
    AnimType currentAnim;
    
    void Update(float deltaTime);
    void StartAnimation(AnimType type);
    Rectangle GetBounds() const;
};

// Connection between commits
struct CommitEdge {
    std::string from;
    std::string to;
    Color color;
    float thickness;
    float progress;  // For animation
    std::vector<Vector2> waypoints;  // Curved path
    
    void CalculateCurve(const Vector2& fromPos, const Vector2& toPos, 
                       float fromLane, float toLane);
    void Draw() const;
};

// Main visualization panel (upper half)
class CommitGraphPanel {
public:
    CommitGraphPanel();
    ~CommitGraphPanel();
    
    void Initialize(int x, int y, int width, int height);
    void Update(float deltaTime);
    void Draw();
    
    // Data
    void AddCommit(const CommitNode& commit);
    void AddBranch(const std::string& name, const std::string& head, Color color);
    void SetHEAD(const std::string& hash);
    void Clear();
    
    // Layout
    void RecalculateLayout();
    void AnimateToLayout();
    
    // Interaction
    CommitNode* GetNodeAt(Vector2 screenPos);
    void SelectNode(const std::string& hash);
    void CenterOnNode(const std::string& hash);
    
    // Callbacks
    std::function<void(const CommitNode&)> onNodeSelected;
    std::function<void(const CommitNode&)> onNodeHovered;
    
private:
    void DrawGrid();
    void DrawEdges();
    void DrawNodes();
    void DrawBranchLabels();
    void DrawHEADIndicator();
    void DrawSelectionEffects();
    
    std::map<std::string, CommitNode> nodes;
    std::vector<CommitEdge> edges;
    std::map<std::string, Color> branchColors;
    std::string headHash;
    std::string selectedHash;
    
    DraggableView viewport;
    Rectangle bounds;
    
    // Animation
    float time;
};

// Internal structure view (lower half) - shows blobs, trees
class InternalStructurePanel {
public:
    InternalStructurePanel();
    ~InternalStructurePanel();
    
    void Initialize(int x, int y, int width, int height);
    void Update(float deltaTime);
    void Draw();
    
    // Set git wrapper for real data
    void SetGitWrapper(GitWrapper* gitWrapper) { git = gitWrapper; }
    
    // Load objects from a commit
    void LoadCommitObjects(const std::string& commitHash);
    
    // Load working directory (untracked/staged files)
    void LoadWorkingDirectory();
    
    void Clear();
    
    // Expand/collapse tree nodes
    void ToggleNode(const std::string& hash);
    
    // Interaction
    GitObject* GetObjectAt(Vector2 screenPos);
    void SelectObject(const std::string& hash);
    
private:
    void LayoutObjects();
    void DrawConnections();
    void DrawObject(const GitObject& obj);
    void DrawObjectDetails(const GitObject& obj);
    
    std::map<std::string, GitObject> objects;
    std::string rootCommit;
    std::string selectedObject;
    
    DraggableView viewport;
    Rectangle bounds;
    
    // Layout params
    float levelHeight;
    float siblingSpacing;
    
    // Git wrapper for real data
    GitWrapper* git;
};

// Split view container
class SplitGitView {
public:
    SplitGitView();
    ~SplitGitView();
    
    void Initialize(int x, int y, int width, int height);
    void Update(float deltaTime);
    void Draw();
    
    // Access panels
    CommitGraphPanel* GetCommitPanel() { return &commitPanel; }
    InternalStructurePanel* GetStructurePanel() { return &structurePanel; }
    
    // Set git wrapper for real data
    void SetGitWrapper(GitWrapper* gitWrapper) { 
        structurePanel.SetGitWrapper(gitWrapper); 
    }
    
    // Sync selection
    void OnCommitSelected(const std::string& hash);
    
    // Resize
    void SetSplitRatio(float ratio);  // 0.0 - 1.0, where to split
    
private:
    CommitGraphPanel commitPanel;
    InternalStructurePanel structurePanel;
    
    Rectangle bounds;
    float splitRatio;
    int splitY;
    
    // Drag divider
    bool draggingDivider;
};

// Detail popup for commit/object info
class DetailPopup {
public:
    DetailPopup();
    
    void Show(const CommitNode& commit);
    void Show(const GitObject& obj);
    void Hide();
    
    void Update(float deltaTime);
    void Draw();
    
    bool IsVisible() const { return visible; }
    
private:
    bool visible;
    float animProgress;
    Vector2 position;
    
    std::string title;
    std::vector<std::pair<std::string, std::string>> fields;
};

} // namespace GitVis
