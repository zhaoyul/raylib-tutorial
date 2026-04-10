# 项目总结 / Project Summary

## 概览 / Overview

这个仓库是一个以 Raylib 为核心的教学型代码库，覆盖：

- 8 个循序渐进的教程章节
- 7 个主要游戏/示例目录
- 1 组 Snake 渐进式扩展版本
- 1 个可选 Janet + Raylib REPL 模块

The repo is organized as a teaching codebase rather than a single monolithic game. Start with the chapters, then move into complete games and optional experimental modules.

## 结构 / Structure

### `chapters/`

按顺序组织的教程章节：

1. `01-cpp-basics`
2. `02-cmake-intro`
3. `03-raylib-basics`
4. `04-game-loop`
5. `05-collision`
6. `06-game-states`
7. `07-raygui-basics`
8. `08-raygui-advanced`

### `games/`

完整游戏和附加示例：

1. `brick-breaker`
2. `snake`
3. `tetris`
4. `tank-battle`
5. `tower-defense`
6. `fps`
7. `git-fighter`

### `games/snake/phases/`

Snake 的渐进式扩展版本：

- `v0-base` 由 `games/snake/` 提供
- `v1-items`
- `v2-fx`
- `v3-audio`
- `v4-multi`

### `janet/`

可选的 Janet + Raylib 互操作模块、REPL 工作流和 NetREPL 示例。

### `docs/`

主要文档入口：

- `README.md`
- `docs/BUILD.md`
- `docs/GUIDE.md`
- `docs/JANET.md`
- `games/snake/ROADMAP.md`
- `games/git-fighter/README.md`

## 构建特点 / Build Characteristics

- Top-level CMake fetches `raylib` and `raygui` automatically if they are not already installed.
- `BUILD_GAMES=ON` also includes `games/git-fighter`, so full builds require `pkg-config` and `libgit2`.
- Output paths are split by content type:
  - chapters: `build/bin/chapters/`
  - main games: `build/bin/games/`
  - Snake phases: `build/bin/snake-phases/`
  - Git Fighter + demos: `build/bin/`
  - Janet module: `build/janet/`

## 推荐阅读顺序 / Recommended Reading Order

1. Read and run the numbered chapter directories in order.
2. Move to `games/brick-breaker` or `games/snake` for a first complete game.
3. Use `games/snake/ROADMAP.md` and `games/snake/phases/README.md` when you want progressive extensions.
4. Enable `janet/` only when you specifically want REPL-driven prototyping.

## 当前文档重点 / Current Documentation Focus

- 清晰的第一次构建路径
- 平台依赖说明
- 教程与示例结构导航
- `git-fighter` 的额外依赖说明
- CMake 4 和字体资源相关的故障排查

## 适合人群 / Target Audience

- C++ beginners
- Developers new to Raylib
- Students working through game-programming exercises
- Teachers who want small, readable examples plus larger projects

## 相关文档 / Related Documents

- [README.md](/Users/kevin/sandbox/gt/raylib_tutorial/crew/dave/README.md)
- [docs/BUILD.md](/Users/kevin/sandbox/gt/raylib_tutorial/crew/dave/docs/BUILD.md)
- [docs/GUIDE.md](/Users/kevin/sandbox/gt/raylib_tutorial/crew/dave/docs/GUIDE.md)
- [docs/JANET.md](/Users/kevin/sandbox/gt/raylib_tutorial/crew/dave/docs/JANET.md)
