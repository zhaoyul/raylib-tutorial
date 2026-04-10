# 构建指南 / Build Guide

本仓库的构建体验分成两类：

- `chapters/` 和大多数示例只需要标准的 C++/CMake/Raylib toolchain。
- 完整构建在 `BUILD_GAMES=ON` 时还会包含 `games/git-fighter`，因此额外需要 `pkg-config` 和 `libgit2`。

This guide focuses on accurate prerequisites, verified configure profiles, and the two most common first-run failures.

## 1. 前置条件 / Prerequisites

### 全平台基础要求 / Base Requirements

- C++17 compiler
- CMake 3.15+
- Git
- Internet access on the first configure, because `raylib` and `raygui` are fetched automatically if not already installed

### 完整构建额外要求 / Extra Requirements For Full Builds

If you keep `BUILD_GAMES=ON`, install:

- `pkg-config`
- `libgit2` development files

### 平台包建议 / Platform Packages

#### macOS

```bash
brew install cmake git

# Needed for full builds with games/git-fighter
brew install pkg-config libgit2
```

#### Ubuntu / Debian

```bash
sudo apt update

# Base toolchain + desktop graphics packages
sudo apt install build-essential cmake git \
  libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev

# Needed for full builds with games/git-fighter
sudo apt install pkg-config libgit2-dev
```

#### Fedora / RHEL

```bash
sudo dnf groupinstall "Development Tools"
sudo dnf install cmake git \
  libX11-devel libXrandr-devel libXi-devel mesa-libGL-devel mesa-libEGL-devel

# Needed for full builds with games/git-fighter
sudo dnf install pkgconf-pkg-config libgit2-devel
```

#### Windows

- Chapters-only builds work with Visual Studio or MinGW as long as CMake can find a C++17 compiler.
- Full builds are easiest in an environment that also provides `pkg-config` and `libgit2` metadata to CMake. If you use plain Visual Studio, make sure `pkg-config` can resolve `libgit2` before configuring `BUILD_GAMES=ON`.

## 2. Clone The Repository

```bash
git clone https://github.com/zhaoyul/raylib-tutorial.git
cd raylib-tutorial
```

## 3. Choose A Configure Profile

### Profile A: Chapters Only

Recommended for a first run, and verified in this workspace.

```bash
cmake -S . -B build \
  -DBUILD_GAMES=OFF \
  -DBUILD_SNAKE_PHASES=OFF \
  -DBUILD_JANET=OFF \
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5
```

Build and run:

```bash
cmake --build build --target cpp-basics chapter07_raygui_basics -j

./build/bin/chapters/cpp-basics
./build/bin/chapters/chapter07_raygui_basics
```

### Profile B: Full Repository

This includes all chapters, main games, Snake phases, and Git Fighter.

```bash
cmake -S . -B build -DCMAKE_POLICY_VERSION_MINIMUM=3.5
cmake --build build -j
```

Common run commands:

```bash
./build/bin/games/brick-breaker
./build/bin/games/snake
./build/bin/games/tetris
./build/bin/games/tank-battle
./build/bin/games/tower-defense
./build/bin/games/fps
./build/bin/git-fighter
./build/bin/demo-graph
./build/bin/demo-split-view
./build/bin/snake-phases/snake-v1-items
```

### Profile C: Snake Phases Only

Useful when you only want the progressive Snake versions.

```bash
cmake -S . -B build \
  -DBUILD_CHAPTERS=OFF \
  -DBUILD_GAMES=OFF \
  -DBUILD_SNAKE_PHASES=ON \
  -DBUILD_JANET=OFF \
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5

cmake --build build --target snake-v1-items snake-v2-fx snake-v3-audio -j
```

Run from:

```bash
./build/bin/snake-phases/snake-v1-items
./build/bin/snake-phases/snake-v2-fx
./build/bin/snake-phases/snake-v3-audio
```

### Profile D: Janet Only

```bash
cmake -S . -B build \
  -DBUILD_CHAPTERS=OFF \
  -DBUILD_GAMES=OFF \
  -DBUILD_SNAKE_PHASES=OFF \
  -DBUILD_JANET=ON \
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5

cmake --build build --target raylib_janet -j
```

The resulting module is written to `build/janet/`.

## 4. Output Layout

| Content | Output path |
|---------|-------------|
| Chapters | `build/bin/chapters/` |
| Main games | `build/bin/games/` |
| Snake phases | `build/bin/snake-phases/` |
| Git Fighter + demos | `build/bin/` |
| Janet module | `build/janet/` |

## 5. Reconfigure After Changing Options

If you switch between profiles, rerun CMake with the new options:

```bash
cmake -S . -B build <new-options>
```

If the cache gets confusing, delete the build directory and start fresh:

```bash
rm -rf build
```

## 6. Troubleshooting

### Error: `Compatibility with CMake < 3.5 has been removed`

This happens with CMake 4 while configuring the fetched Raylib 5.0 source.

Use:

```bash
cmake -S . -B build -DCMAKE_POLICY_VERSION_MINIMUM=3.5
```

You can keep that flag in all configure commands.

### Error: `Package 'libgit2' not found`

This comes from `games/git-fighter/CMakeLists.txt` when `BUILD_GAMES=ON`.

Check it directly:

```bash
pkg-config --modversion libgit2
```

If the command fails, either:

1. Install `pkg-config` and `libgit2` development packages, then rerun CMake.
2. Disable `BUILD_GAMES` for a chapters-only or Janet-only workflow.

### Error: missing X11/OpenGL headers on Linux

Install the graphics development packages for your distro first. On Ubuntu/Debian, the minimum desktop set is:

```bash
sudo apt install libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev
```

### Error: Raygui demos only show English text

The Raygui chapters try to load Chinese fonts from `data/fonts/`. Follow [data/fonts/README.md](/Users/kevin/sandbox/gt/raylib_tutorial/crew/dave/data/fonts/README.md) and keep the downloaded `.otf` file under `data/fonts/`.

### Error: build cannot fetch Raylib or Raygui

The first configure may download dependencies from GitHub. Make sure:

- the machine has network access
- Git can reach GitHub
- the build directory is not half-configured from an earlier failed fetch

When in doubt:

```bash
rm -rf build
cmake -S . -B build -DCMAKE_POLICY_VERSION_MINIMUM=3.5
```

### Error: windowed programs fail in CI or on a server

Compiling requires graphics headers, and running GUI programs also needs a usable display session. In CI or headless environments, use a virtual display or restrict yourself to configure/build validation.

## 7. Related Docs

- [README.md](/Users/kevin/sandbox/gt/raylib_tutorial/crew/dave/README.md)
- [docs/GUIDE.md](/Users/kevin/sandbox/gt/raylib_tutorial/crew/dave/docs/GUIDE.md)
- [docs/JANET.md](/Users/kevin/sandbox/gt/raylib_tutorial/crew/dave/docs/JANET.md)
- [games/git-fighter/README.md](/Users/kevin/sandbox/gt/raylib_tutorial/crew/dave/games/git-fighter/README.md)
