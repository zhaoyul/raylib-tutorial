# 构建指南 / Build Guide

## 系统要求 / System Requirements

### Windows
- Visual Studio 2019+ 或 MinGW-w64
- CMake 3.15+
- Git

### Linux (Ubuntu/Debian)
```bash
# 安装必要的开发工具
sudo apt update
sudo apt install build-essential git cmake

# 安装图形库依赖
sudo apt install libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev
```

### Linux (Fedora/RHEL)
```bash
sudo dnf groupinstall "Development Tools"
sudo dnf install cmake git
sudo dnf install libX11-devel libXrandr-devel libXi-devel mesa-libGL-devel
```

### macOS
```bash
# 使用 Homebrew 安装
brew install cmake git
```

## 构建步骤 / Build Steps

### 1. 克隆仓库 / Clone Repository

```bash
git clone https://github.com/zhaoyul/raylib-tutorial.git
cd raylib-tutorial
```

### 2. 创建构建目录 / Create Build Directory

```bash
mkdir build
cd build
```

### 3. 配置项目 / Configure Project

```bash
# 默认配置（构建所有内容）
cmake ..

# 或者选择性构建
cmake -DBUILD_CHAPTERS=ON -DBUILD_GAMES=OFF ..  # 只构建教程章节
cmake -DBUILD_CHAPTERS=OFF -DBUILD_GAMES=ON ..  # 只构建游戏
```

### 4. 编译 / Build

```bash
# 使用 CMake 构建
cmake --build .

# 或者使用 make（Linux/macOS）
make

# 或者使用 MSBuild（Windows with Visual Studio）
cmake --build . --config Release
```

### 5. 运行程序 / Run Programs

编译成功后，可执行文件位于 `build/bin/` 目录：

```bash
# 运行教程章节示例
./bin/chapters/cpp-basics
./bin/chapters/cmake-intro
./bin/chapters/raylib-basics
./bin/chapters/game-loop
./bin/chapters/collision
./bin/chapters/game-states

# 运行游戏
./bin/games/brick-breaker
./bin/games/snake
./bin/games/tetris
./bin/games/tank-battle
./bin/games/tower-defense
./bin/games/fps
```

## Janet + Raylib 互操作模块（可选）

### 依赖安装

Janet 需要单独安装（系统包或源码构建），确保 `janet` 和 `pkg-config` 可用：

```bash
# Ubuntu/Debian
sudo apt install janet pkg-config

# macOS (Homebrew)
brew install janet pkg-config
```

### 构建 Janet 模块

```bash
# 在构建目录启用 Janet 模块
cmake -DBUILD_JANET=ON ..
cmake --build . --target raylib_janet
```

### REPL 使用（示例）

```bash
export JANET_PATH=/path/to/raylib-tutorial/janet
export JANET_MODULE_PATH=/path/to/raylib-tutorial/build/janet
janet
```

更多 REPL 工作流请参考 [docs/JANET.md](JANET.md)。

## 常见问题 / Troubleshooting

### 问题：找不到 X11 库（Linux）

**解决方案：**
```bash
sudo apt install libx11-dev libxrandr-dev libxi-dev
```

### 问题：CMake 版本太旧

**解决方案：**
```bash
# 升级 CMake
pip install --upgrade cmake
# 或从官网下载最新版本
```

### 问题：找不到编译器

**Windows 解决方案：**
- 安装 Visual Studio 2019+ 
- 或安装 MinGW-w64

**Linux 解决方案：**
```bash
sudo apt install build-essential
```

### 问题：Raylib 下载失败

**解决方案：**
手动下载 Raylib：
```bash
cd external
git clone https://github.com/raysan5/raylib.git
cd raylib
git checkout 5.0
```

然后在 CMakeLists.txt 中指定 Raylib 路径。

## 开发环境配置 / Development Environment

### Visual Studio Code

推荐安装的扩展：
- C/C++ (Microsoft)
- CMake Tools
- CMake Language Support

### CLion

CLion 原生支持 CMake 项目，直接打开即可。

### Visual Studio

使用 "打开文件夹" 功能打开项目根目录，Visual Studio 会自动识别 CMake 项目。

## 清理构建 / Clean Build

```bash
# 删除构建目录
rm -rf build

# 重新构建
mkdir build && cd build
cmake ..
cmake --build .
```

## 交叉编译 / Cross Compilation

### 为 Web 构建（使用 Emscripten）

```bash
# 安装 Emscripten
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh

# 在仓库根目录构建 Web 版本（示例：Snake）
cd /path/to/repository
emcmake cmake -S . -B build-web \
  -DBUILD_CHAPTERS=OFF \
  -DBUILD_GAMES=ON \
  -DBUILD_SNAKE_PHASES=OFF \
  -DBUILD_JANET=OFF
cmake --build build-web --target snake

# 生成产物位于 build-web/bin/games/snake.html + snake.js + snake.wasm
# 使用本地静态服务器运行（浏览器直接打开 file:// 往往会被跨域限制）
python3 -m http.server 8080 --directory build-web/bin/games
# 打开 http://127.0.0.1:8080/snake.html
```

### 为 Android 构建

需要 Android NDK，具体步骤请参考 Raylib 官方文档。

## 性能优化构建 / Optimized Build

```bash
# Release 构建（优化性能）
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .

# Debug 构建（包含调试信息）
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
```

## 需要帮助？ / Need Help?

- 查看 [Raylib 官方文档](https://www.raylib.com/)
- 提交 [Issue](https://github.com/zhaoyul/raylib-tutorial/issues)
- 参考 `docs/GUIDE.md` 详细教程
