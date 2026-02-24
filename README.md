# 8-Ball Pool Game: Refactoring Report (ver1 → ver2)

**Course:** Advanced Programming Lab — 2nd Year CSE  
**Project:** 8-Ball Pool Game (raylib / C)  
**Date:** February 2026

---

## Table of Contents

### Part A — AI Prompts Used
1. [Overview](#overview)
2. [Stage 1: Analysis & Planning Prompts](#stage-1-analysis--planning-prompts)
3. [Stage 2: Coding Convention & Standards Prompts](#stage-2-coding-convention--standards-prompts)
4. [Stage 3: Folder Structure Reorganization Prompts](#stage-3-folder-structure-reorganization-prompts)
5. [Stage 4: Code-Level Improvement Prompts](#stage-4-code-level-improvement-prompts)
6. [Stage 5: Documentation & Guide Prompts](#stage-5-documentation--guide-prompts)
7. [Prompt Design Philosophy](#prompt-design-philosophy)
8. [Summary of Prompt Categories](#summary-of-prompt-categories)

### Part B — Detailed Explanation of Changes
9. [Executive Summary](#9-executive-summary)
10. [Coding & Naming Conventions](#10-coding--naming-conventions)
11. [Design Model](#11-design-model)
12. [Design Patterns Analysis](#12-design-patterns-analysis)
13. [Detailed Change Log (ver1 → ver2)](#13-detailed-change-log-ver1--ver2)
14. [What Was Improved & What Remains](#14-what-was-improved--what-remains)
15. [Lessons Learned](#15-lessons-learned)
16. [Appendix A: File Metrics Comparison](#appendix-a-file-metrics-comparison)
17. [Appendix B: Dependency Direction Verification](#appendix-b-dependency-direction-verification)

---

# PART A — AI PROMPTS USED

---

## Overview

This section records the AI prompts used to transform the 8-Ball Pool game from its original monolithic structure (ver1) into the modular, header-separated version (ver2). The refactoring was carried out in stages, each guided by targeted prompts covering naming conventions, separation of concerns, SOLID principles, build system improvements, and structural reorganization across multiple files.

---

## Stage 1: Analysis & Planning Prompts

### Prompt 1.1 — Initial Codebase Audit

> "Analyze this C raylib 8-ball pool game. The entire game is written in a single file: main.c. Read it fully and for each section, list all functions, global structs, enums, macros, and `#define` constants. Identify naming conventions used, responsibilities bundled into single functions, SOLID principle violations, and architectural weaknesses. Provide a full summary of what this codebase does and how it is currently organized."

**Purpose:** To fully understand the ver1 codebase before touching anything. Every function, every constant, every struct needed to be inventoried before a refactoring plan could be made.

**What we learned:**
- The entire game (~600 lines) lived in a single `main.c` with no separation of concerns
- A single `Game` struct acted as a god object holding physics state, rendering state, input state, and game logic state all together
- Naming was abbreviated and inconsistent: `g` for game pointer, `bls` for balls, `pktd` for pocketed, `BlType` for ball type, `GM_START` for game state
- All constants were raw `#define` macros scattered at the top with no grouping or documentation
- Functions like `UpdateGm`, `DrawGm`, `HndlInput`, `ChkCollisions` handled multiple unrelated responsibilities
- No header files existed — everything was in one translation unit

### Prompt 1.2 — Refactoring Plan & File Split Strategy

> "For this single-file C raylib pool game, design a multi-file refactoring plan. The goal is to split responsibilities into: main.c (entry point only), game.c/.h (game state, input, turns, win logic), physics.c/.h (movement, collision, pockets), graphics.c/.h (all drawing functions), utils.c/.h (math helpers), and config.h (all constants). Show which functions and structs belong in each file, what each header should expose, and what should remain private. Make sure to identify any shared types that will need a common.h."


**Purpose:** To design the exact file architecture of ver2 before writing any code. This produced the blueprint for the 6-header, 5-source-file structure that was implemented.

---

## Stage 2: Coding Convention & Standards Prompts

### Prompt 2.1 — Naming Convention Redesign

> "Create a naming convention upgrade plan for a C raylib pool game. The ver1 code uses heavy abbreviations (BL_CUE, GM_PLAY, pktd, vel, BlType, HndlInput, ChkCollisions). Design a new convention that covers: (1) struct field names — full descriptive words, (2) enum values — MODULE_DESCRIPTOR format, (3) function names — VerbNoun PascalCase, (4) constants — UPPER_SNAKE_CASE in config.h, (5) type names — PascalCase without abbreviation. Show a before/after table using the actual names from this codebase."

**Purpose:** To define and document the naming convention improvements applied in ver2, replacing all abbreviated names with readable, descriptive equivalents.

**Convention decisions made:**

| Element | ver1 (old) | ver2 (new) |
|---------|-----------|-----------|
| Struct fields | `pos`, `vel`, `pktd`, `isStrp` | `position`, `velocity`, `pocketed`, `isStriped` |
| Enum values | `BL_CUE`, `GM_PLAY`, `PLR_NONE` | `BALL_CUE`, `GAME_PLAYING`, `PLAYER_NONE` |
| Function names | `HndlInput`, `ChkCollisions`, `UpdatePhys` | `HandleInput`, `CheckCollisions`, `UpdatePhysics` |
| Constants | `T_W`, `T_H`, `BL_R`, `PKT_R` | `TABLE_WIDTH`, `TABLE_HEIGHT`, `BALL_RADIUS`, `POCKET_RADIUS` |
| Type names | `BlType`, `GmState`, `PlrType` | `BallType`, `GameState`, `PlayerType` |
| Player struct | `blsLeft`, `nm` | `ballsRemaining`, `name` |

### Prompt 2.2 — Config Header Design

> "Take all the `#define` constants from this C pool game's main.c (T_W, T_H, BL_R, PKT_R, RIL_W, FRIC, MIN_VEL, MAX_PWR_PX, MAX_SHT_SPD, MAX_BL_SPD, STICK_LENGTH) and reorganize them into a well-structured config.h file. Group them by category: window settings, table dimensions, ball physics, shot power, and stick behavior. Rename them to be fully descriptive using UPPER_SNAKE_CASE. Add a brief comment explaining the purpose of each constant."

**Purpose:** To centralize all magic numbers into a single, well-documented configuration header, making tuning and modification easy without touching game logic code.

---

## Stage 3: Folder Structure Reorganization Prompts

### Prompt 3.1 — Header Architecture & common.h Design

> "For a refactored C raylib pool game split into game.c, graphics.c, physics.c, and utils.c, design a header architecture that avoids circular includes. Specifically: (1) What shared types (Ball, Game, Player, BallType, GameState) belong in a common.h? (2) How should config.h relate to common.h? (3) What should each module header (game.h, graphics.h, physics.h, utils.h) expose vs. keep private? (4) Should pocket position logic be in a shared inline function — and if so, where does it live?"

**Purpose:** To design the header dependency tree cleanly before writing any headers, preventing the circular include problems that commonly occur when splitting a monolithic C file.

**Decisions made:**
- `config.h` defines only constants, no types, included by `common.h`
- `common.h` defines all shared types (`Ball`, `Player`, `Game`, enums) and the `GetPocketPositions()` inline helper — included by all modules
- Each module header only exposes its public API; internal helpers remain in `.c` files
- `GetPocketPositions()` was made `static inline` in `common.h` to avoid duplication between `graphics.c` and `game.c` both needing pocket coordinates

### Prompt 3.2 — Build System Upgrade

> "The ver1 pool game was compiled with a single gcc command. Design a proper Makefile and build.bat for the ver2 multi-file version. The Makefile should: compile each .c file into a separate .o object, link them together, support `make`, `make run`, and `make clean` targets, and reference raylib from C:/raylib/raylib. The build.bat should do the same in a single gcc command for quick Windows builds. Both should use -std=c99 -O2 -Wall -Wextra flags."

**Purpose:** To upgrade the build system from a manual one-liner to a proper Makefile with separate compilation, which speeds up incremental rebuilds during development and makes the project structure explicit.

---

## Stage 4: Code-Level Improvement Prompts

### Prompt 4.1 — Physics Module Extraction

> "Extract all physics-related code from this monolithic pool game main.c into a standalone physics.c and physics.h. The physics module should own: UpdatePhysics (movement + friction + wall bouncing), CheckCollisions (ball-to-ball overlap resolution), ResolveElasticCollision (normal/tangent component swap), and ClampBallSpeed. It should depend on utils.h for Distance() and ClampBallSpeed(). Show the exact function signatures for physics.h and the full implementation of physics.c with proper includes."

**Purpose:** To cleanly isolate all physics simulation logic so it can be modified, tested, or replaced without touching game logic or rendering code.

### Prompt 4.2 — Graphics Module Extraction

> "Extract all drawing code from this monolithic raylib pool game into graphics.c and graphics.h. The module should expose: DrawGame (master draw function), DrawTable, DrawBalls, DrawCueStick, DrawPowerBar, DrawHUD, DrawOverlays. It should only depend on common.h and raylib.h — no direct dependency on game.h or physics.h. The DrawBalls function must handle stripe overlay rendering. DrawCueStick must handle the recoil animation and aiming line. Show the complete implementation."

**Purpose:** To fully separate rendering from logic, ensuring that any change to how the game looks never risks breaking how the game plays.

### Prompt 4.3 — Duplicate Symbol Bug Fix

> "In this refactored C pool game, CheckPockets is declared in both physics.h and game.h, and implemented in both physics.c and game.c. This causes a linker error due to duplicate symbol definition. Analyze which file should own CheckPockets — the function reads ball positions (physics data) but also modifies game state (player scores, scratch, win/loss). Propose the correct fix: should CheckPockets live in game.c and be called from UpdatePhysics via a function pointer, or should UpdatePhysics call an extern declaration, or should the function be merged into one location?"

**Purpose:** To resolve the most critical bug introduced during the refactoring — a duplicate symbol that prevents the project from linking.

### Prompt 4.4 — Input Clamping Bug Fix

> "In this pool game's HandleInput function, stickPullPixels is assigned directly from Distance(mousePos, cueBallPos) with no upper bound: `game->stickPullPixels = Distance(mousePos, cueBallPos)`. The power bar renders correctly because it caps at MAX_POWER_PIXELS visually, but the raw stickPullPixels value can exceed 160.0f, causing the cue stick to render at abnormal lengths. Add the missing clamp so stickPullPixels never exceeds MAX_POWER_PIXELS, and explain where in HandleInput this clamp should be inserted."

**Purpose:** To fix a subtle rendering/physics bug where dragging the mouse far from the cue ball causes the stick to extend beyond its intended maximum length.

---

## Stage 5: Documentation & Guide Prompts

### Prompt 5.1 — README Generation

> "Write a README.md for a 2-player 8-ball pool game written in C using raylib. The project has been refactored from a single main.c (ver1) into a modular structure: src/main.c, src/game.c, src/graphics.c, src/physics.c, src/utils.c, include/common.h, include/config.h, include/game.h, include/graphics.h, include/physics.h, include/utils.h. Include: project description, how to build (Makefile and build.bat), how to play (controls), file structure overview, and known limitations."

**Purpose:** To produce professional project documentation that a new developer (or course assessor) can read to understand the project immediately.

### Prompt 5.2 — Inline Documentation Pass

> "Add Doxygen-style documentation to all function declarations in these header files for a C raylib pool game: game.h, graphics.h, physics.h, utils.h. For each function, write a @brief description, @param for each parameter, and @return where applicable. Use the actual function signatures: InitGame, ResetBalls, UpdateGame, HandleInput, CheckPockets, DrawGame, DrawTable, DrawBalls, DrawCueStick, UpdatePhysics, CheckCollisions, ResolveElasticCollision, Distance, ClampBallSpeed, AreBallsMoving."

**Purpose:** To make every public API function self-documenting, reducing the need to read implementation files to understand what a function does.

---

## Prompt Design Philosophy

Every prompt used in this refactoring followed three principles:

**1. Reference actual code names.** Generic prompts like "refactor my game" produce generic results. Every prompt named specific functions (`HndlInput`, `ChkCollisions`), specific structs (`Game`, `Ball`), and specific files (`main.c`, `physics.c`). This forces the AI to reason about the actual codebase rather than producing template-style output.

**2. Define the desired output format explicitly.** Each prompt specifies what form the answer should take — "show a before/after table", "show the full implementation", "show exact function signatures for the header". This eliminates ambiguity and produces immediately usable output.

**3. Ask one stage at a time.** Rather than prompting "refactor everything", the work was broken into analysis → conventions → structure → code → documentation. Each stage builds on the output of the previous one, and errors can be caught and corrected before they propagate forward.

---

## Summary of Prompt Categories

| Stage | Focus | Prompts | Primary Output |
|-------|-------|---------|---------------|
| 1 | Analysis & Planning | 2 | Architecture inventory, file split blueprint |
| 2 | Naming & Constants | 2 | Naming convention table, config.h structure |
| 3 | File/Build Structure | 2 | Header dependency design, Makefile + build.bat |
| 4 | Code Improvements | 4 | Module extractions, bug fixes (duplicate symbol, input clamp) |
| 5 | Documentation | 2 | README, Doxygen headers |
| **Total** | | **12** | |

---

# PART B — DETAILED EXPLANATION OF CHANGES

---

## 9. Executive Summary

The 8-Ball Pool game was refactored from a single monolithic `main.c` (~600 lines) into a clean multi-file C project with 5 source files, 6 header files, a dedicated config header, and an upgraded build system. The core gameplay logic is identical between versions — the refactoring was purely architectural. All abbreviated naming was replaced with descriptive identifiers, all constants were extracted to `config.h`, and responsibilities were cleanly divided across modules so that physics, rendering, game logic, and utilities are fully decoupled.

---

## 10. Coding & Naming Conventions

### 10.1 ver1 Conventions (Problems)

Ver1 used heavy abbreviation throughout, making the code difficult to read without memorizing what each short name meant:

- Struct fields: `pos`, `vel`, `pktd`, `isStrp`, `blsLeft`, `nm`, `curPlr`, `frstShot`
- Enum values: `BL_CUE`, `BL_SOLID`, `GM_START`, `GM_PLAY`, `PLR_NONE`
- Functions: `HndlInput`, `UpdatePhys`, `ChkCollisions`, `ChkPockets`, `DrawGm`, `InitGm`
- Constants: `T_W`, `T_H`, `BL_R`, `PKT_R`, `RIL_W`, `FRIC`, `MIN_VEL`, `MAX_PWR_PX`
- Types: `BlType`, `GmState`, `PlrType`

### 10.2 ver2 Conventions (Improvements)

Ver2 uses fully descriptive naming throughout:

- Struct fields: `position`, `velocity`, `pocketed`, `isStriped`, `ballsRemaining`, `name`, `currentPlayer`, `firstShot`
- Enum values: `BALL_CUE`, `BALL_SOLID`, `GAME_START`, `GAME_PLAYING`, `PLAYER_NONE`
- Functions: `HandleInput`, `UpdatePhysics`, `CheckCollisions`, `CheckPockets`, `DrawGame`, `InitGame`
- Constants: `TABLE_WIDTH`, `TABLE_HEIGHT`, `BALL_RADIUS`, `POCKET_RADIUS`, `RAIL_WIDTH`, `FRICTION`, `MIN_VELOCITY`, `MAX_POWER_PIXELS`
- Types: `BallType`, `GameState`, `PlayerType`

### 10.3 Constants Centralization

Ver1 had all `#define` constants at the top of `main.c` in a single flat block with no grouping. Ver2 moves all constants to `config.h`, grouped by category:

| Group | Constants |
|-------|-----------|
| Window | `WINDOW_TITLE`, `TARGET_FPS` |
| Table | `TABLE_WIDTH`, `TABLE_HEIGHT`, `RAIL_WIDTH` |
| Balls | `MAX_BALLS`, `BALL_RADIUS`, `POCKET_RADIUS` |
| Physics | `FRICTION`, `MIN_VELOCITY`, `MAX_BALL_SPEED` |
| Shot Power | `MAX_POWER_PIXELS`, `MAX_SHOT_SPEED` |
| Stick | `STICK_LENGTH`, `STICK_RECOIL_TIME` |

---

## 11. Design Model

### 11.1 ver1 Architecture — Monolithic

```
main.c (single file, ~600 lines)
│
├── All #defines (constants)
├── All enums (BallType, GameState, PlayerType)
├── All structs (Ball, Player, Game)
├── All functions:
│   ├── InitGm / ResetBls
│   ├── UpdateGm / HndlInput
│   ├── UpdatePhys / ChkCollisions / ElasticCollision / ClampBlSpd
│   ├── ChkPockets / ChkWinCond / NxtTurn / ApplyScratch
│   ├── DrawGm / DrawTbl / DrawPwrBar
│   └── BlsMoving / Dist / plrIdxForTyp
└── main()
```

### 11.2 ver2 Architecture — Modular

```
include/
├── config.h       → All constants (no types)
├── common.h       → All shared types + GetPocketPositions()
├── game.h         → Game logic API
├── graphics.h     → Rendering API
├── physics.h      → Physics API
└── utils.h        → Math utility API

src/
├── main.c         → Entry point only (12 lines)
├── game.c         → State management, input, turns, pockets, win logic
├── graphics.c     → All DrawXxx functions
├── physics.c      → Movement, friction, bouncing, collisions
└── utils.c        → Distance, ClampBallSpeed, AreBallsMoving
```

### 11.3 Dependency Flow

```
main.c
  └── game.h, graphics.h

game.c
  └── physics.h, utils.h, common.h

graphics.c
  └── common.h (only)

physics.c
  └── utils.h, common.h

utils.c
  └── common.h (only)

common.h
  └── config.h, raylib.h
```

Dependencies flow downward: `main → game/graphics → physics → utils`. No upward or circular dependencies.

---

## 12. Design Patterns Analysis

### 12.1 Patterns Present in ver2

**State Pattern (informal):** The `GameState` enum (`GAME_START`, `GAME_PLAYING`, `GAME_SCRATCH`, `GAME_WON`, `GAME_LOST`) drives behavior in `UpdateGame`, `HandleInput`, `DrawOverlays`, and `DrawCueStick`. Each state produces different behavior in each module without nested if/else chains per function.

**Template Method (informal):** `DrawGame` acts as a template method — it defines the rendering order (table → pockets → balls → cue stick → power bar → HUD → overlays) and delegates each step to a specific sub-function. The order is fixed; the implementations are separate.

**Flyweight — Shared Pocket Positions:** `GetPocketPositions()` is a `static inline` function in `common.h` shared by both `game.c` (for pocket detection) and `graphics.c` (for pocket rendering). Both modules use the same position data without duplication.

### 12.2 Patterns That Could Be Added (Future Work)

| Pattern | Application |
|---------|-------------|
| **Command** | Encode each shot as a command object to enable undo/replay |
| **Observer** | Notify HUD module when game state changes rather than polling every frame |
| **Strategy** | Swap physics models (simple friction vs. spin/english) via function pointer struct |
| **Factory** | `CreateRackConfiguration()` factory for different rack layouts (9-ball, straight pool) |

---

## 13. Detailed Change Log (ver1 → ver2)

### 13.1 File Structure Changes

| Change | ver1 | ver2 |
|--------|------|------|
| Source files | 1 (`main.c`) | 5 (`main.c`, `game.c`, `graphics.c`, `physics.c`, `utils.c`) |
| Header files | 0 | 6 (`common.h`, `config.h`, `game.h`, `graphics.h`, `physics.h`, `utils.h`) |
| Build script | Manual `gcc` one-liner | `Makefile` + `build.bat` |
| Project structure | Flat directory | `src/`, `include/` separation |

### 13.2 Naming Changes

| Category | ver1 | ver2 |
|----------|------|------|
| Ball struct field | `pos`, `vel`, `pktd`, `isStrp` | `position`, `velocity`, `pocketed`, `isStriped` |
| Player struct field | `blsLeft`, `nm`, `typ` | `ballsRemaining`, `name`, `type` |
| Game struct field | `curPlr`, `frstShot`, `blsMoving` | `currentPlayer`, `firstShot`, `ballsMoving` |
| Enum — ball type | `BL_CUE`, `BL_SOLID`, `BL_STRIPE`, `BL_EIGHT` | `BALL_CUE`, `BALL_SOLID`, `BALL_STRIPE`, `BALL_EIGHT` |
| Enum — game state | `GM_START`, `GM_PLAY`, `GM_SCRATCH`, `GM_WIN`, `GM_LOSE` | `GAME_START`, `GAME_PLAYING`, `GAME_SCRATCH`, `GAME_WON`, `GAME_LOST` |
| Enum — player type | `PLR_NONE`, `PLR_SOLIDS`, `PLR_STRIPES` | `PLAYER_NONE`, `PLAYER_SOLIDS`, `PLAYER_STRIPES` |
| Functions | `HndlInput`, `UpdatePhys`, `ChkCollisions`, `ElasticCollision`, `BlsMoving`, `Dist` | `HandleInput`, `UpdatePhysics`, `CheckCollisions`, `ResolveElasticCollision`, `AreBallsMoving`, `Distance` |
| Constants | `T_W`, `T_H`, `BL_R`, `PKT_R`, `RIL_W` | `TABLE_WIDTH`, `TABLE_HEIGHT`, `BALL_RADIUS`, `POCKET_RADIUS`, `RAIL_WIDTH` |

### 13.3 Code-Level Improvements

| Improvement | Detail |
|-------------|--------|
| **Pocket position deduplication** | ver1 hardcoded pocket array in both `DrawGm` and `ChkPockets`. ver2 uses `GetPocketPositions()` shared inline helper in `common.h` |
| **Stick recoil constant** | ver1 hardcoded `0.12f` recoil time inline. ver2 uses `STICK_RECOIL_TIME` from `config.h` |
| **FPS constant** | ver1 hardcoded `1.0f/60.0f` in recoil calculation. ver2 uses `1.0f / TARGET_FPS` |
| **Ball colors** | ver1 used `GREEN` and `MAROON`. ver2 replaces `GREEN` with `SKYBLUE` for better visual distinction between stripe colors |
| **`main.c` size** | Reduced from ~600 lines to 12 lines |
| **Type assignment guard** | ver2 uses `assignedTypes` consistently; `firstShot` flag is still cleared on shot release (known issue, see Section 14) |

### 13.4 Build System Changes

| | ver1 | ver2 |
|-|------|------|
| Build method | Single `gcc` command with all files | `Makefile` with per-file `.o` compilation |
| Incremental build | Not supported | Supported — only changed files recompile |
| Windows quick build | Manual command | `build.bat` |
| Targets | None | `make all`, `make run`, `make clean` |
| Compiler flags | `-std=c99 -O2` | `-std=c99 -O2 -Wall -Wextra` |

---

## 14. What Was Improved & What Remains

### 14.1 Improvements Achieved

| Area | ver1 | ver2 |
|------|------|------|
| **File count** | 1 source file, 0 headers | 5 source files, 6 headers |
| **Naming** | Heavily abbreviated throughout | Fully descriptive names |
| **Constants** | Scattered `#define` blocks in `main.c` | Centralized, grouped `config.h` |
| **Responsibilities** | All in one file | Cleanly split by domain |
| **Build system** | Manual one-liner | Makefile + batch script |
| **Pocket logic** | Duplicated in draw and logic | Single shared `GetPocketPositions()` |
| **main.c size** | ~600 lines | 12 lines |

### 14.2 What Still Needs Work

| Area | Current State | Needed Change |
|------|--------------|---------------|
| **Duplicate `CheckPockets`** | Declared in both `physics.h` and `game.h`; defined in both `.c` files — causes linker error | Remove from `physics.h`/`physics.c`; call from `game.c` via extern or merge into one location |
| **`stickPullPixels` clamp** | Set to raw `Distance()` with no upper bound in `HandleInput` | Add `if (game->stickPullPixels > MAX_POWER_PIXELS) game->stickPullPixels = MAX_POWER_PIXELS;` |
| **`firstShot` timing** | Cleared on mouse release, not when balls stop | Move `game->firstShot = false` to after `AreBallsMoving()` returns false |
| **`src/` subdirectory** | `build.bat` references `src/main.c` etc. but `Makefile` compiles from current directory | Align both build files to the same directory convention |
| **Unit tests** | None | Add per-module test files for physics math and collision logic |
| **AI opponent** | Not implemented | Could be added as `ai.c`/`ai.h` without modifying existing modules |
| **Spin / English** | Physics assumes all balls have equal mass, no spin | Strategy pattern could swap physics model |

---

## 15. Lessons Learned

### 15.1 About Program Styling

- **Abbreviations compound over time.** In ver1, `pktd`, `vel`, `bls`, and `blsLeft` seemed harmless in a 600-line file. In a larger project they become a maintenance burden. Descriptive names cost nothing at write time and save significant time during every future read.
- **A `config.h` should exist from day one.** Scattering magic numbers through code forces anyone tuning the physics to hunt through hundreds of lines. A single config file with grouped, commented constants makes the game's behavior immediately legible.
- **File structure communicates architecture.** The ver2 folder layout tells you the design: `physics.c` handles physics, `graphics.c` handles rendering, `utils.c` handles math. A new developer understands the structure before reading a single line of code.

### 15.2 About Design Patterns in C

- **Patterns exist in C even without OOP.** The State pattern is implemented through a plain enum and switch logic. The Template Method pattern is implemented through a master `DrawGame` function calling sub-functions in order. Patterns describe solutions to recurring problems — the language is secondary.
- **`static inline` in headers is C's flyweight.** Sharing `GetPocketPositions()` across modules via a header-defined inline function avoids both code duplication and the overhead of a function call — the same pocket data is available everywhere with zero coupling.

### 15.3 About Using AI for Refactoring

- **Specific prompts produce specific results.** Naming exact functions (`HndlInput`, `ChkCollisions`) and exact field names (`pktd`, `vel`) in prompts produces targeted, usable output. Vague prompts produce generic advice.
- **AI catches bugs you introduce during refactoring.** The duplicate `CheckPockets` symbol bug and the missing `stickPullPixels` clamp were both identified through targeted audit prompts after the initial refactor. Code splitting introduces integration errors that a fresh AI analysis can surface.
- **Stages beat one large prompt.** Asking "split this file into modules" in one prompt produces a shallow answer. Asking "audit", then "plan", then "extract physics", then "fix bugs" in sequence produces a deeper, more correct result at each step because each prompt can reference the verified output of the last.
- **AI does not make pragmatic trade-offs automatically.** When asked to rename everything immediately, AI will do so. The decision to defer naming to a second pass — to avoid breaking a working game — is a human judgment call that must be explicitly stated in the prompt.

---

## Appendix A: File Metrics Comparison

| Metric | ver1 | ver2 | Change |
|--------|------|------|--------|
| Source files (`.c`) | 1 | 5 | +4 |
| Header files (`.h`) | 0 | 6 | +6 |
| Total lines (approx.) | ~600 | ~750 | +150 (separation overhead) |
| `main.c` lines | ~600 | 12 | −588 |
| Directories | 1 (flat) | 2+ (`src/`, `include/`) | +1 |
| Named constants | ~12 raw `#define` | 14 in grouped `config.h` | +2, fully organized |
| Public functions | ~17 | ~20 | +3 (split sub-functions) |
| Struct types | 3 (`Ball`, `Player`, `Game`) | 3 (same, renamed fields) | Same |
| Enum types | 3 (`BallType`, `GameState`, `PlayerType`) | 3 (same, renamed values) | Same |
| Build targets | 0 (manual `gcc`) | 3 (`all`, `run`, `clean`) | +3 |
| Known linker bugs | 0 | 1 (duplicate `CheckPockets`) | +1 (introduced in refactor) |

---

## Appendix B: Dependency Direction Verification

| Source File | Includes / Calls | Layer |
|-------------|-----------------|-------|
| `main.c` | `common.h`, `game.h`, `graphics.h` | Entry → Logic + Rendering |
| `game.c` | `game.h`, `physics.h`, `utils.h` | Logic → Physics + Utils |
| `graphics.c` | `graphics.h` (→ `common.h`) | Rendering → Shared types only |
| `physics.c` | `physics.h`, `utils.h` | Physics → Utils |
| `utils.c` | `utils.h` (→ `common.h`) | Utils → Shared types only |
| `common.h` | `config.h`, `raylib.h` | Types → Constants + External lib |

**Conclusion:** Dependencies flow strictly downward — `main → game/graphics → physics → utils → common → config`. No circular dependencies exist. `graphics.c` is correctly isolated: it depends only on `common.h` and reads the `Game` struct without calling any game logic or physics functions.
