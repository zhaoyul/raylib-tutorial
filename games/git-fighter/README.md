# Git Fighter

`git-fighter` 是一个基于 Raylib 的 Git 教学/可视化项目，使用 `libgit2` 读取和操作仓库状态，并提供额外的图形演示程序。

`git-fighter` is a Raylib-based Git teaching game and visualization playground. It is the one game in this repository that adds an extra native dependency chain beyond plain Raylib.

## 需要什么 / Prerequisites

在构建 `git-fighter` 之前，确保以下工具可用：

- `pkg-config`
- `libgit2` development files
- the normal repo prerequisites from [docs/BUILD.md](/Users/kevin/sandbox/gt/raylib_tutorial/crew/dave/docs/BUILD.md)

Quick check:

```bash
pkg-config --modversion libgit2
```

If that command fails, the top-level configure step will also fail while `BUILD_GAMES=ON`.

## 构建 / Build

从仓库根目录运行：

```bash
cmake -S . -B build -DCMAKE_POLICY_VERSION_MINIMUM=3.5
cmake --build build --target git-fighter demo-graph demo-split-view -j
```

生成的程序位于：

- `build/bin/git-fighter`
- `build/bin/demo-graph`
- `build/bin/demo-split-view`

## 运行 / Run

```bash
./build/bin/git-fighter
./build/bin/demo-graph
./build/bin/demo-split-view
```

## 仓库内结构 / Local Structure

- `src/`: main app, UI, visualization, git wrapper
- `levels/`: scripted learning levels such as branch, merge, rebase, stash
- `DESIGN.md`: higher-level design notes

## 关卡主题 / Level Themes

当前关卡覆盖：

- real Git basics
- branching
- merging
- remotes
- rebasing
- cherry-picking
- bisect
- reflog
- interactive workflows
- stash

## 注意事项 / Notes

- This target is included automatically when `BUILD_GAMES=ON`.
- If you only want the numbered tutorial chapters, configure with `-DBUILD_GAMES=OFF`.
- On systems with CMake 4, keep `-DCMAKE_POLICY_VERSION_MINIMUM=3.5` in the configure command to avoid the fetched Raylib compatibility error.

## 相关文档 / Related Docs

- [docs/BUILD.md](/Users/kevin/sandbox/gt/raylib_tutorial/crew/dave/docs/BUILD.md)
- [games/git-fighter/DESIGN.md](/Users/kevin/sandbox/gt/raylib_tutorial/crew/dave/games/git-fighter/DESIGN.md)
