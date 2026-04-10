# Raylib Tutorial Guide

This guide explains how to move through the repository without getting lost in the number of examples.

## 1. Start Here

If this is your first pass through the repo:

1. Configure a chapters-only build from [docs/BUILD.md](/Users/kevin/sandbox/gt/raylib_tutorial/crew/dave/docs/BUILD.md).
2. Read and run `chapters/01-cpp-basics` through `chapters/08-raygui-advanced` in order.
3. Pick one complete game and trace how it applies the chapter concepts.

## 2. Chapter Sequence

| Chapter | Topic | Why it matters |
|---------|-------|----------------|
| `01-cpp-basics` | C++ basics | Language foundation for the rest of the repo |
| `02-cmake-intro` | CMake | How targets and builds are organized |
| `03-raylib-basics` | Raylib basics | Window setup, input, drawing |
| `04-game-loop` | Game loop | The core update/render structure |
| `05-collision` | Collision | Interaction rules and physics basics |
| `06-game-states` | Game states | Menus, pause screens, end conditions |
| `07-raygui-basics` | Raygui basics | Immediate-mode UI controls |
| `08-raygui-advanced` | Raygui advanced | More complex layout and styling |

## 3. Game Map

| Game | Best for | Notes |
|------|----------|-------|
| `brick-breaker` | first complete 2D game read-through | small, readable, chapter-aligned |
| `snake` | grid logic and state machines | pairs well with the Snake roadmap |
| `tetris` | array manipulation and rotation | stronger algorithm focus |
| `tank-battle` | multiple actors and projectiles | more moving parts |
| `tower-defense` | wave/path systems | broader gameplay systems |
| `fps` | simple 3D concepts | camera and shooting basics |
| `git-fighter` | advanced optional example | adds `libgit2` and visualization code |

## 4. Snake Progression

`games/snake/` is the base version. The progressive versions live under `games/snake/phases/`.

Recommended order:

1. `games/snake/main.cpp`
2. `games/snake/README.md`
3. `games/snake/ROADMAP.md`
4. `games/snake/phases/README.md`
5. `games/snake/phases/v1-items` through `v4-multi`

This is the best place to study how a small game grows without immediately jumping to a large architecture.

## 5. Janet Workflow

Ignore `janet/` unless you want:

- REPL-driven rendering experiments
- NetREPL-driven live updates
- scripting and prototyping outside the main C++ game flow

If you do want that workflow, read [docs/JANET.md](/Users/kevin/sandbox/gt/raylib_tutorial/crew/dave/docs/JANET.md) after you already understand the normal C++ build.

## 6. Font Setup For Raygui Chapters

The Raygui chapters can render Chinese text when a supported font exists under `data/fonts/`.

See [data/fonts/README.md](/Users/kevin/sandbox/gt/raylib_tutorial/crew/dave/data/fonts/README.md).

## 7. When To Use Which Build Profile

- Want the safest first compile: chapters only
- Want all games and examples: full build with `libgit2`
- Want only Snake progression: Snake phases only
- Want REPL experiments: Janet only

All concrete commands live in [docs/BUILD.md](/Users/kevin/sandbox/gt/raylib_tutorial/crew/dave/docs/BUILD.md).
