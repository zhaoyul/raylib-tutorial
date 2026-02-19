# 第8章: Raygui 高级应用 / Chapter 8: Raygui Advanced

## 学习目标 / Learning Objectives

- 掌握 Raygui 的样式系统，自定义控件外观
- 学习使用图标字体增强界面表现力
- 理解复杂布局的实现方法
- 掌握高级控件如列表视图、颜色选择器等
- 学习如何创建可重用的 UI 组件

## 内容概览 / Content Overview

### 8.1 样式系统 (Styles)

Raygui 提供完整的样式系统来自定义控件外观：

```cpp
// 设置全局样式属性
GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
GuiSetStyle(DEFAULT, TEXT_SPACING, 2);

// 设置按钮样式
GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(BLUE));
GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, ColorToInt(WHITE));
GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL, ColorToInt(DARKBLUE));
GuiSetStyle(BUTTON, BORDER_WIDTH, 2);
```

**常用样式属性：**
| 属性                  | 说明             |
|-----------------------|------------------|
| `TEXT_SIZE`           | 字体大小         |
| `TEXT_SPACING`        | 字符间距         |
| `TEXT_COLOR_NORMAL`   | 正常状态文本颜色 |
| `TEXT_COLOR_FOCUSED`  | 聚焦状态文本颜色 |
| `TEXT_COLOR_PRESSED`  | 按下状态文本颜色 |
| `BASE_COLOR_NORMAL`   | 正常状态背景色   |
| `BORDER_COLOR_NORMAL` | 正常状态边框颜色 |
| `BORDER_WIDTH`        | 边框宽度         |

### 8.2 样式加载与保存

```cpp
// 加载样式文件
GuiLoadStyle("styles/style_cyber.rgs");

// 保存当前样式
GuiSaveStyle("styles/my_style.rgs");
```

### 8.3 图标字体 (Icons)

Raygui 支持内嵌的图标字体：

```cpp
// 在控件文本中使用图标
GuiButton((Rectangle){10, 10, 120, 30}, "#141# Settings");

// 常用图标代码
// #141# - Gear (Settings)
// #142# - Trash (Delete)
// #143# - Eye (View)
// #144# - Warning
// #145# - Info
// #146# - Help
// #147# - Play
// #148# - Pause
// #149# - Stop
```

### 8.4 高级控件

#### 列表视图 (ListView) - Raygui 4.0+ uses pointer for active
```cpp
const char* items = "Item1;Item2;Item3;Item4;Item5";
int scrollIndex = 0;
int activeItem = -1;

activeItem = GuiListView((Rectangle){10, 10, 200, 150},
                         items, &scrollIndex, &activeItem);
```

#### 列表视图扩展版 (ListViewEx)
```cpp
const char* listItems[] = {"Sword", "Shield", "Potion", "Key", "Coin"};
int itemCount = 5;
int focusedItem = -1;
int scrollIndex = 0;
int activeItem = -1;

int selected = GuiListViewEx((Rectangle){10, 10, 200, 150},
                             listItems, itemCount,
                             &focusedItem, &scrollIndex, &activeItem);
```

#### 消息框 (MessageBox)
```cpp
int result = GuiMessageBox((Rectangle){screenWidth/2 - 150, screenHeight/2 - 75, 300, 150},
                           "Confirm Delete",
                           "Are you sure you want to delete this save?",
                           "OK;Cancel");
if (result == 1) {  // Clicked OK
    // Perform delete
}
```

#### 颜色选择器 (ColorPicker) - Raygui 4.0+ uses pointer
```cpp
Color selectedColor = RED;
GuiColorPicker((Rectangle){10, 10, 200, 200},
               NULL, &selectedColor);
```

#### 颜色面板 (ColorPanel)
```cpp
Color color = BLUE;
Vector2 pickerPos = {0, 0};
GuiColorPanel((Rectangle){10, 10, 200, 200},
              NULL, &color);
```

#### 数值框 (Spinner) - Raygui 4.0+ uses pointer
```cpp
int value = 50;
bool editMode = false;
value = GuiSpinner((Rectangle){10, 10, 120, 30},
                   NULL, &value, 0, 100, editMode);
```

#### 数值滑块 (ValueBox) - Raygui 4.0+ uses pointer
```cpp
int intValue = 42;
bool editMode = false;
intValue = GuiValueBox((Rectangle){10, 50, 120, 30},
                       NULL, &intValue, 0, 999, editMode);
```

### 8.5 自动布局系统

实现简单的自动布局：

```cpp
// 布局管理器
class Layout {
public:
    Rectangle bounds;
    int padding = 10;
    int spacing = 5;
    int currentY;

    Layout(Rectangle bounds, int padding = 10)
        : bounds(bounds), padding(padding) {
        currentY = bounds.y + padding;
    }

    Rectangle NextRow(int height) {
        Rectangle rect = {
            bounds.x + padding,
            (float)currentY,
            bounds.width - padding * 2,
            (float)height
        };
        currentY += height + spacing;
        return rect;
    }

    Rectangle NextColumn(int width, int height) {
        // 实现列布局
    }
};

// 使用
Layout layout({10, 10, 300, 400});
GuiButton(layout.NextRow(30), "Button 1");
GuiButton(layout.NextRow(30), "Button 2");
GuiSlider(layout.NextRow(20), ...);
```

### 8.6 滚动面板 (ScrollPanel)

```cpp
Rectangle panelBounds = {10, 10, 300, 400};
Rectangle contentBounds = {0, 0, 600, 800};  // 内容比面板大
Vector2 scrollOffset = {0, 0};

// 绘制滚动面板
GuiScrollPanel(panelBounds, contentBounds, &scrollOffset);

// 在面板内绘制内容（需要考虑滚动偏移）
BeginScissorMode(panelBounds.x, panelBounds.y, panelBounds.width, panelBounds.height);
DrawText("Content", panelBounds.x + scrollOffset.x, panelBounds.y + scrollOffset.y, 20, BLACK);
EndScissorMode();
```

### 8.7 标签页 (TabBar)

```cpp
const char* tabs[] = {"Items", "Equip", "Skills", "Quests"};
int activeTab = 0;

// 使用按钮实现标签页
int tabX = 10;
int tabY = 10;
int tabWidth = 80;
int tabHeight = 30;

for (int i = 0; i < 4; i++) {
    // Set different color based on active state
    if (activeTab == i) {
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(SKYBLUE));
    } else {
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(LIGHTGRAY));
    }

    if (GuiButton((Rectangle){(float)(tabX + i * tabWidth), (float)tabY, (float)tabWidth, (float)tabHeight},
                  tabs[i])) {
        activeTab = i;
    }
}

// Draw tab content
switch (activeTab) {
    case 0: /* Items content */ break;
    case 1: /* Equipment content */ break;
    case 2: /* Skills content */ break;
    case 3: /* Quests content */ break;
}
```

### 8.8 工具提示 (Tooltip)

```cpp
// Simple tooltip implementation
void GuiTooltip(Rectangle bounds, const char* text) {
    Vector2 mousePos = GetMousePosition();
    if (CheckCollisionPointRec(mousePos, bounds)) {
        // Draw tooltip background
        int textWidth = MeasureText(text, 10);
        DrawRectangle(mousePos.x + 10, mousePos.y + 10, textWidth + 10, 20, DARKGRAY);
        DrawText(text, mousePos.x + 15, mousePos.y + 15, 10, WHITE);
    }
}

// Usage
if (GuiButton((Rectangle){10, 10, 100, 30}, "Help")) { }
GuiTooltip((Rectangle){10, 10, 100, 30}, "Click for help");
```

### 8.9 文件对话框

```cpp
// 简单的文件浏览器
void DrawFileBrowser(Rectangle bounds, const char* path, char* selectedFile) {
    // 绘制文件列表
    FilePathList files = LoadDirectoryFiles(path);

    for (int i = 0; i < files.count; i++) {
        Rectangle itemBounds = {bounds.x, bounds.y + i * 25, bounds.width, 20};
        if (GuiButton(itemBounds, files.paths[i])) {
            strcpy(selectedFile, files.paths[i]);
        }
    }

    UnloadDirectoryFiles(files);
}
```

## 中文字体支持 / Chinese Font Support

**需要下载中文字体文件才能显示中文界面！**

macOS 系统字体（.ttc 格式）无法被 raylib 正确加载中文字符，需要下载独立的 `.otf` 字体文件。

### 快速下载

```bash
cd data/fonts
curl -L -o AlibabaPuHuiTi-Regular.otf \
  "https://cdn.jsdelivr.net/npm/alibaba-puhuiti-2/Alibaba-PuHuiTi-Regular/Alibaba-PuHuiTi-Regular.otf"
```

详细说明请参考 `data/fonts/README.md`

## 实践示例 / Practice Examples

本章包含以下示例：
1. **RPG 物品栏系统** - 使用列表视图、颜色选择器构建复杂界面
2. **游戏设置面板** - 综合应用各种高级控件
3. **关卡编辑器界面** - 使用滚动面板、自动布局的实用案例

## 下一步 / Next Steps

完成 Raygui 章节后，你已经掌握了完整的游戏 UI 开发技能。可以：
- 将这些知识应用到实际游戏项目中
- 创建自己的 UI 组件库
- 深入学习自定义绘制和动画效果

## 参考资源 / Resources

- [Raygui 样式示例](https://github.com/raysan5/raygui/tree/master/styles)
- [Raygui 图标列表](https://github.com/raysan5/raygui/blob/master/icons/raygui_icons.txt)
- [样式编辑器工具](https://github.com/raysan5/raygui/tree/master/tools/rGuiStyler)
