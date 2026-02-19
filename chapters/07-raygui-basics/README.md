# 第7章: Raygui 基础 / Chapter 7: Raygui Basics

## 学习目标 / Learning Objectives

- 了解 Raygui 是什么以及它与 Raylib 的关系
- 掌握 Raygui 的基本控件使用
- 学习按钮、滑块、文本框等常用控件
- 理解即时模式 GUI (Immediate Mode GUI) 的概念

## 内容概览 / Content Overview

### 7.1 什么是 Raygui？

Raygui 是 Raylib 的官方配套 GUI 库，提供简单易用的即时模式图形用户界面控件。

**特点：**
- 轻量级，单头文件库（`raygui.h`）
- 与 Raylib 完美集成
- 提供 20+ 种常用控件
- 支持自定义样式和图标

**依赖关系：**
```
Raygui 依赖于 Raylib
你的项目 -> Raygui -> Raylib
```

### 7.2 即时模式 GUI (Immediate Mode GUI)

与传统保留模式 GUI 不同，即时模式 GUI 每帧都重新绘制：

```cpp
// 传统 GUI：创建按钮，等待回调
Button* btn = new Button("Click me");
btn->onClick = []() { /* 处理点击 */ };

// 即时模式 GUI：每帧检查状态
if (GuiButton((Rectangle){10, 10, 100, 30}, "Click me")) {
    // 按钮被点击，立即处理
}
```

**优点：**
- 代码简洁直观
- 无状态管理负担
- 易于集成到游戏循环中

### 7.3 基础控件

#### 按钮 (Button)
```cpp
if (GuiButton((Rectangle){10, 10, 120, 30}, "Start Game")) {
    currentState = PLAYING;
}
```

#### 标签 (Label)
```cpp
GuiLabel((Rectangle){10, 50, 100, 20}, "Player Name:");
```

#### 滑块 (Slider) - Raygui 4.0+ uses pointer parameter
```cpp
float volume = 0.5f;
GuiSlider((Rectangle){10, 90, 200, 20}, 
          "Volume", TextFormat("%.0f%%", volume * 100), 
          &volume, 0.0f, 1.0f);
```

#### 文本框 (TextBox)
```cpp
char playerName[64] = "";
bool editMode = false;
if (GuiTextBox((Rectangle){10, 130, 200, 30}, playerName, 64, editMode)) {
    editMode = !editMode;  // Toggle edit mode on click
}
```

#### 复选框 (CheckBox) - Raygui 4.0+ uses pointer parameter
```cpp
bool fullscreen = false;
GuiCheckBox((Rectangle){10, 170, 20, 20}, 
            "Fullscreen", &fullscreen);
```

#### 下拉框 (DropdownBox)
```cpp
int selectedDifficulty = 0;
bool dropdownEditMode = false;
const char* difficultyOptions = "Easy;Medium;Hard";
if (GuiDropdownBox((Rectangle){10, 210, 120, 30}, 
                   difficultyOptions, &selectedDifficulty, dropdownEditMode)) {
    dropdownEditMode = !dropdownEditMode;
}
```

### 7.4 容器控件

#### 面板 (Panel)
```cpp
GuiPanel((Rectangle){10, 10, 300, 400}, "Settings Panel");
```

#### 窗口框 (WindowBox)
```cpp
bool windowActive = true;
if (GuiWindowBox((Rectangle){100, 100, 400, 300}, "Game Settings")) {
    windowActive = false;  // Clicked close button
}
```

#### 分组框 (GroupBox)
```cpp
GuiGroupBox((Rectangle){10, 250, 280, 100}, "Audio Settings");
```

### 7.5 常用布局模式

#### 垂直布局
```cpp
int y = 10;
if (GuiButton((Rectangle){10, y, 120, 30}, "Button 1")) { }
y += 40;
if (GuiButton((Rectangle){10, y, 120, 30}, "Button 2")) { }
y += 40;
if (GuiButton((Rectangle){10, y, 120, 30}, "Button 3")) { }
```

#### 水平布局
```cpp
int x = 10;
if (GuiButton((Rectangle){x, 10, 100, 30}, "Left")) { }
x += 110;
if (GuiButton((Rectangle){x, 10, 100, 30}, "Center")) { }
x += 110;
if (GuiButton((Rectangle){x, 10, 100, 30}, "Right")) { }
```

## 中文字体支持 / Chinese Font Support

**需要下载中文字体文件才能显示中文界面！**

由于 macOS 系统字体使用 .ttc 格式，raylib 无法正确加载其中的中文字符，你需要下载独立的 `.otf` 字体文件。

### 快速下载（推荐）

```bash
cd data/fonts

# 下载阿里巴巴普惠体（推荐，体积较小）
curl -L -o AlibabaPuHuiTi-Regular.otf \
  "https://cdn.jsdelivr.net/npm/alibaba-puhuiti-2/Alibaba-PuHuiTi-Regular/Alibaba-PuHuiTi-Regular.otf"
```

### 字体检测顺序

1. `data/fonts/AlibabaPuHuiTi-Regular.otf`
2. `data/fonts/NotoSansSC-Regular.otf`
3. `data/fonts/NotoSansCJKsc-Regular.otf`

找到任一字体后自动显示中文界面，否则显示英文。

详细说明请参考 `data/fonts/README.md`

## 实践示例 / Practice Examples

本章包含以下示例：
1. **基础控件演示** - 展示所有基础控件的使用
2. **简单菜单系统** - 使用 Raygui 构建游戏菜单
3. **设置界面** - 音量、难度等游戏设置的 UI

## 下一步 / Next Steps

完成本章后，继续学习第8章：Raygui 高级应用，了解样式定制、复杂布局和高级控件。

## 参考资源 / Resources

- [Raygui GitHub](https://github.com/raysan5/raygui)
- [Raygui 示例](https://github.com/raysan5/raygui/tree/master/examples)
- [Raygui 控件列表](https://github.com/raysan5/raygui#controls-list)
