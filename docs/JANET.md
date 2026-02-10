# Janet + Raylib REPL 工作流 / Janet + Raylib REPL Workflow

## 目标 / Goal

本模块提供 Janet 语言与 Raylib 的最小互操作层，支持在 **REPL** 中直接驱动窗口、绘制和游戏状态更新，用于快速原型、实时调试和热重载探索。

## 1. 构建互操作模块 / Build the Interop Module

> 需要先安装 Janet 运行时（确保 `janet` 与 `pkg-config` 可用）。

```bash
cmake -S . -B build -DBUILD_JANET=ON
cmake --build build --target raylib_janet
```

构建完成后，模块位于 `build/janet/raylib.so`（Windows 为 `.dll`）。

## 2. REPL 启动 / Start the REPL

```bash
export JANET_PATH=/path/to/raylib-tutorial/janet
export JANET_MODULE_PATH=/path/to/raylib-tutorial/build/janet
janet
```

在 REPL 中导入模块：

```clojure
(import raylib)
(import workflow)
```

## 3. 独特的 REPL 驱动工作流 / A Live REPL Workflow

这个工作流通过 **逐帧驱动** + **函数热替换** 实现实时编辑：

1. **初始化窗口**：
   ```clojure
   (workflow/init "Janet Playground" 800 450 :fps 60)
   ```
2. **定义状态与行为（可随时改写）**：
   ```clojure
   (def game-state @{:x 200 :y 200 :vx 160 :vy 120})

   (defn update [dt state]
     (set game-state :x (+ (get game-state :x) (* (get game-state :vx) dt)))
     (set game-state :y (+ (get game-state :y) (* (get game-state :vy) dt))))

   (defn draw [state]
     (raylib/draw-text "Edit me in REPL" 20 20 20 240 240 240 255)
     (raylib/draw-circle (get game-state :x) (get game-state :y) 20 255 196 0 255))

   (set workflow/update update)
   (set workflow/draw draw)
   ```
3. **逐帧推进（REPL 仍可继续输入）**：
   ```clojure
   (workflow/step-frame)   # 只渲染 1 帧
   (workflow/run-frames 120) # 渲染 120 帧
   ```

### 热替换玩法 / Hot Reload Tricks

在窗口运行中，你可以直接改写 `update` 或 `draw` 函数，下一帧立即生效。  
这种 “**一步一帧 + 热替换**” 的流程非常适合用 REPL 进行玩法试验和调试。

## 4. 通过 REPL 创建完整项目 / Create a Full Project from the REPL

在 REPL 中随时生成可运行的项目骨架：

```clojure
(workflow/save-template "my-game.janet")
```

然后将你在 REPL 中的 `update` / `draw` 内容复制到 `my-game.janet` 中，即可：

```bash
janet my-game.janet
```

## 5. 内置 REPL 示例 / Built-in Example

```bash
janet janet/examples/repl-demo.janet
```

该示例会弹出窗口，并提示你在 REPL 中修改 `update` / `draw`。
