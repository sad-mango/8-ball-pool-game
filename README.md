# 8 Ball Pool — MVC Architecture

> **What this project does:** This project takes the existing **8 Ball Pool** game (built using SOLID principles in C++ with raylib) and refactors it into the **Model-View-Controller (MVC)** design pattern. Every original file from `balls/`, `interfaces/`, and `managers/` is preserved untouched. The MVC layer is introduced as a clean new `mvc/` folder containing three classes that replace the old monolithic `Game`, `Renderer`, `InputHandler`, and `PhysicsEngine` responsibilities.

---

## Table of Contents

1. [Project Overview](#project-overview)
2. [Why MVC — and Why Now?](#why-mvc--and-why-now)
3. [Architecture at a Glance](#architecture-at-a-glance)
4. [Folder Structure](#folder-structure)
5. [The MVC Triad in Detail](#the-mvc-triad-in-detail)
   - [Model — `GameModel`](#model--gamemodel)
   - [View — `GameView`](#view--gameview)
   - [Controller — `GameController`](#controller--gamecontroller)
6. [How the Pieces Connect — `main.cpp` as the Orchestrator](#how-the-pieces-connect--maincpp-as-the-orchestrator)
7. [The Full Game Loop (Step by Step)](#the-full-game-loop-step-by-step)
8. [Data Flow Diagram](#data-flow-diagram)
9. [What Changed vs the SOLID Version](#what-changed-vs-the-solid-version)
10. [Supporting Systems (Unchanged)](#supporting-systems-unchanged)
    - [balls/ — Ball Hierarchy](#balls--ball-hierarchy)
    - [managers/ — BallManager & TurnManager](#managers--ballmanager--turnmanager)
    - [interfaces/ — IPhysics, IRenderer, IInputHandler](#interfaces--iphysics-irenderer-iinputhandler)
11. [Class Responsibility Table](#class-responsibility-table)
12. [Key Design Decisions](#key-design-decisions)
13. [Building and Running](#building-and-running)

---

## Project Overview

**8 Ball Pool** is a two-player billiards game built in C++ with the raylib graphics library. Players take turns shooting the cue ball with drag-and-release mouse controls. The game handles elastic ball collisions, pocket detection, scratch rules, ball-type assignment (solids vs stripes), and win/loss conditions. The game runs at 60 FPS in a 800×500 window.

**Tech stack:** C++17 · raylib 5.0 · MinGW-w64 (Windows build)

---

## Why MVC — and Why Now?

In the original SOLID version, the `Game` class acted as the central hub — it held all game state AND coordinated rendering AND processed input AND ran the physics loop. While SOLID principles (dependency injection via `IPhysics`, `IRenderer`, `IInputHandler`) kept individual systems clean, `Game.h` itself was still a God Object that every other class depended on.

`Game` owned all of this at once:

- All game state: `GameState`, `BallManager`, `TurnManager`, positions, power, aiming flags
- The update loop: physics, recoil animation, turn transitions
- The draw call: delegating to `Renderer`
- Input delegation: calling `InputHandler`

MVC solves this by giving each concern its own dedicated class:

| Concern | MVC Role | Old Class | New Class |
|---|---|---|---|
| What the game data looks like | **Model** | `Game` (partially) | `GameModel` |
| How the data is drawn on screen | **View** | `Renderer` | `GameView` |
| How input and physics mutate the data | **Controller** | `InputHandler` + `Game::Update()` + `PhysicsEngine` | `GameController` |

The result is a game loop that is just two lines:

```cpp
controller.Update(model);   // C writes state
view.Render(model);         // V reads state
```

---

## Architecture at a Glance

```
main()
  ├── GameModel       ← M: owns all game state
  ├── GameView        ← V: reads model, renders everything
  └── GameController  ← C: reads input, runs physics, writes model

           supported by (unchanged):
  ├── balls/          ← Ball, CueBall, SolidBall, StripeBall, EightBall
  ├── managers/       ← BallManager, TurnManager, Player
  └── interfaces/     ← IPhysics, IRenderer, IInputHandler
```

---

## Folder Structure

```
8BallPool_MVC/
│
├── main.cpp                        ← Entry point: creates M, V, C and runs loop
├── config.h                        ← Compile-time constants (unchanged)
├── types.h                         ← BallType, GameState, PlayerType enums (unchanged)
├── build.bat                       ← Windows build script (updated for MVC)
├── 8ballpool_MVC.exe               ← Ready-to-run Windows executable
│
├── mvc/                            ← NEW: the MVC layer
│   ├── GameModel.h                 ← MODEL: all game state & domain methods
│   ├── GameView.h                  ← VIEW: all rendering, reads model only
│   └── GameController.h           ← CONTROLLER: input + physics → writes model
│
├── balls/                          ← UNCHANGED from SOLID version
│   ├── Ball.h / Ball.cpp           ← Base ball class
│   ├── CueBall.h                   ← White cue ball
│   ├── SolidBall.h                 ← Balls 1–7
│   ├── StripeBall.h                ← Balls 9–15
│   └── EightBall.h                 ← The 8-ball
│
├── managers/                       ← UNCHANGED from SOLID version
│   ├── BallManager.h               ← Creates and tracks all 16 balls
│   ├── TurnManager.h               ← Manages player turns and ball-type assignment
│   └── Player.h                    ← Player data struct
│
└── interfaces/                     ← UNCHANGED from SOLID version
    ├── IPhysics.h
    ├── IRenderer.h
    └── IInputHandler.h
```

---

## The MVC Triad in Detail

### Model — `GameModel`

**File:** `mvc/GameModel.h`

The Model is the single source of truth for all game data. It holds no raylib draw calls, reads no keyboard input, and knows nothing about how data will be displayed. Everything the game needs to remember between frames lives here.

```cpp
class GameModel {
public:
    // Game state
    GameState   state;           // START, PLAYING, SCRATCH, WON, LOST
    std::string statusMessage;   // Shown in the HUD
    Vector2     cueBallPos;      // Position used when cue ball is re-placed

    // Aiming / cue stick state
    float   power;               // Shot power 0.0–1.0
    bool    aiming;              // Is player currently dragging?
    bool    ballsMoving;         // Are any balls still rolling?
    bool    firstShot;
    float   stickPullPixels;     // How far the stick is pulled back
    float   stickLength;
    bool    stickRecoil;
    float   recoilTimer;

    // Domain objects
    BallManager ballManager;     // Owns all 16 Ball* pointers
    TurnManager turnManager;     // Manages players, types, turn order

    // Domain methods
    void Init();                 // Reset everything for a new game
    void NextTurn();             // Advance to other player
    void ApplyScratch();         // Handle cue ball pocketed
    void CheckWinCondition();    // Check if 8-ball is shootable
};
```

**What was moved here from `Game.h`:**
All state fields that were previously public members of `Game` now live in `GameModel`. The domain methods (`NextTurn`, `ApplyScratch`, `CheckWinCondition`, `Init`) are also migrated here — they are pure state transitions that belong to the Model, not to any rendering or input system.

---

### View — `GameView`

**File:** `mvc/GameView.h`

The View receives a `const GameModel&` (read-only reference) every frame and draws the complete scene. It never writes to the model. All drawing code that previously lived in `core/Renderer.cpp` has been moved here.

```cpp
class GameView {
public:
    void Render(const GameModel& model);  // Called once per frame

private:
    void DrawTable();
    void DrawPockets();
    void DrawBalls(const GameModel& model);
    void DrawCueStick(const GameModel& model);
    void DrawPowerBar(const GameModel& model);
    void DrawHUD(const GameModel& model);
    void DrawOverlays(const GameModel& model);  // Scratch / Win / Lose overlays
};
```

**What was moved here from `core/Renderer.cpp`:**
Every private drawing method is a 1-to-1 migration from `Renderer::DrawXxx(Game* game)`. The only change is that `Game*` is replaced with `const GameModel&` — proving the View is read-only.

---

### Controller — `GameController`

**File:** `mvc/GameController.h`

The Controller is the most active layer. Every frame it reads mouse and keyboard input, advances the physics simulation, and writes all resulting changes back into `GameModel`. It never draws anything.

```cpp
class GameController {
public:
    void Update(GameModel& model);   // Called once per frame

private:
    // Input (migrated from core/InputHandler.cpp)
    void HandleInput(GameModel& model);
    void HandleScratchPlacement(GameModel& model);
    void HandleAiming(GameModel& model);
    void HandleShoot(GameModel& model);

    // Physics (migrated from core/PhysicsEngine.cpp)
    void UpdatePhysics(GameModel& model);
    void MoveBalls(GameModel& model);
    void CheckCollisions(GameModel& model);
    void ResolveElasticCollision(Ball* a, Ball* b);
    void CheckPockets(GameModel& model);

    // Utilities
    float Distance(Vector2 a, Vector2 b) const;
    void  ClampSpeed(Ball* b, float maxSpeed) const;
};
```

**What was moved here:**

| Origin | What moved |
|---|---|
| `core/InputHandler.cpp` | `Handle()`, `HandleScratchPlacement()`, `HandleAiming()`, `HandleShoot()` |
| `core/Game.h::Update()` | Recoil animation, ball-stop detection, turn-end logic |
| `core/PhysicsEngine.cpp` | `MoveAndBounce()`, `CheckCollisions()`, `ResolveElasticCollision()`, `CheckPockets()` |

---

## How the Pieces Connect — `main.cpp` as the Orchestrator

```cpp
#include "mvc/GameModel.h"
#include "mvc/GameView.h"
#include "mvc/GameController.h"

int main(void) {
    InitWindow(TABLE_WIDTH, TABLE_HEIGHT + 100, "8 Ball Pool - MVC Edition");
    SetTargetFPS(TARGET_FPS);

    GameModel      model;       // M: owns all state
    GameView       view;        // V: renders model
    GameController controller;  // C: handles input & physics

    while (!WindowShouldClose()) {
        controller.Update(model);  // C writes to model
        view.Render(model);        // V reads from model
    }

    CloseWindow();
    return 0;
}
```

**Compare to the old `main.cpp`:**

```cpp
// OLD — SOLID version
PhysicsEngine physics;
Renderer      renderer;
InputHandler  input;
Game game(&physics, &renderer, &input);

while (!WindowShouldClose()) {
    game.Update();   // Game calls physics + input internally
    game.Draw();     // Game calls renderer internally
}
```

The MVC version requires no constructor injection. Each layer is self-contained and the data flow direction is explicit in the game loop itself.

---

## The Full Game Loop (Step by Step)

```
① controller.Update(model) is called
│
├─② HandleInput(model)
│   ├─ KEY_R pressed?          → model.Init()
│   ├─ state == SCRATCH?       → HandleScratchPlacement (click to place cue ball)
│   ├─ ballsMoving?            → skip (no input while balls roll)
│   ├─ mouse pressed on cue?   → model.aiming = true
│   ├─ mouse dragging?         → model.stickPullPixels updated
│   └─ mouse released?         → HandleShoot → set cue velocity in model
│
├─③ UpdatePhysics(model)
│   ├─ Recoil animation        → model.stickPullPixels decays
│   ├─ MoveBalls               → positions += velocity, bounce off rails
│   ├─ CheckCollisions         → elastic velocity swap between overlapping balls
│   ├─ CheckPockets            → ball.Stop(), update score / trigger scratch / win
│   ├─ Detect balls stopped    → model.ballsMoving = false
│   └─ End of turn             → model.NextTurn()
│
④ view.Render(model) is called  (reads model, never writes)
│
├─ DrawTable()       → green felt, brown rails
├─ DrawPockets()     → 6 black circles
├─ DrawBalls()       → each non-pocketed Ball calls its own Draw()
├─ DrawCueStick()    → brown stick aimed at mouse, aiming guide line
├─ DrawPowerBar()    → red fill based on model.stickPullPixels
├─ DrawHUD()         → player names, balls remaining, current turn
└─ DrawOverlays()    → scratch / WIN / LOSE full-screen overlays
```

---

## Data Flow Diagram

```
┌─────────────────────────────────────────────────┐
│                    main.cpp                     │
│  Creates GameModel, GameView, GameController    │
└───────────┬─────────────────────────────────────┘
            │ owns all three
            ▼
┌───────────────────────────────────────────────────────────────┐
│                         Game Loop                             │
│        controller.Update(model)   →   view.Render(model)     │
└───────────────────────────────────────────────────────────────┘
            │                                   │
            ▼                                   ▼
  ┌──────────────────┐                 ┌──────────────────┐
  │  GameController  │                 │    GameView      │
  │                  │    writes       │                  │
  │  Reads:          │ ─────────────► │  Reads:          │
  │  · Mouse/KB      │                 │  · model.state   │
  │  · Ball positions│    GameModel    │  · ballManager   │
  │                  │ ◄───────────── │  · turnManager   │
  │  Writes:         │    (shared)     │  · stickPull     │
  │  · velocities    │                 │  · statusMessage │
  │  · model.state   │                 │                  │
  │  · model.aiming  │                 │  Never writes    │
  │  · ballsRemaining│                 │  to model        │
  └──────────────────┘                 └──────────────────┘
            │
            │  uses (unchanged)
            ▼
  ┌────────────────────────────────────────────┐
  │  balls/   managers/   interfaces/          │
  │  Ball *   BallManager   TurnManager        │
  └────────────────────────────────────────────┘

  Mouse/Keyboard ──────────────────► GameController
  Physics results ─────────────────► GameModel (via Controller)
  Frame render reads ──────────────► GameView ◄── GameModel
```

---

## What Changed vs the SOLID Version

| File | SOLID Version | MVC Version |
|---|---|---|
| `main.cpp` | Instantiates `Game`, `PhysicsEngine`, `Renderer`, `InputHandler` | Instantiates `GameModel`, `GameView`, `GameController` |
| `core/Game.h` | God object: holds state + calls all systems | **Removed** — state split into `GameModel`, logic into `GameController` |
| `core/Renderer.cpp/.h` | Draws by reading `Game*` | **Removed** — migrated into `GameView` |
| `core/InputHandler.cpp/.h` | Handles input by writing to `Game*` | **Removed** — migrated into `GameController` |
| `core/PhysicsEngine.cpp/.h` | Physics by reading/writing `Game*` | **Removed** — migrated into `GameController` |
| `mvc/GameModel.h` | Did not exist | **NEW** — owns all state |
| `mvc/GameView.h` | Did not exist | **NEW** — all rendering |
| `mvc/GameController.h` | Did not exist | **NEW** — all input + physics |
| `balls/` | Unchanged | Unchanged |
| `managers/` | Unchanged | Unchanged |
| `interfaces/` | Unchanged | Unchanged |
| `config.h` / `types.h` | Unchanged | Unchanged |

**One additional fix made:** `TurnManager::Other()` gained a `const` overload so that `GameView` (which takes a `const GameModel&`) can call it during the win overlay render without compiler error.

```cpp
// Added to managers/TurnManager.h
const Player& Other() const { return players[1 - currentPlayer]; }
```

---

## Supporting Systems (Unchanged)

### balls/ — Ball Hierarchy

`Ball` is the abstract base class. It stores `position`, `velocity`, `color`, `type`, `pocketed`, and defines `Draw()` and `IsMoving()`. The four concrete subclasses each override `Draw()` with their own visual:

- `CueBall` — solid white with a grey dot
- `SolidBall` — flat color with number label
- `StripeBall` — color with white horizontal stripe and number
- `EightBall` — solid black with number 8

The `GameView` calls `ball->Draw()` polymorphically on every non-pocketed ball. The `GameController` reads `ball->position` and writes `ball->velocity` during physics.

### managers/ — BallManager & TurnManager

`BallManager` owns the array of 16 `Ball*` pointers, handles triangle rack setup in `Reset()`, and exposes `AnyMoving()` and `GetCueBall()`.

`TurnManager` owns the two `Player` structs, tracks `currentPlayer`, handles `AssignTypes()` (solids/stripes assignment on first pocket), and `PlayerIndexForType()`.

Both are members of `GameModel` and are read by the `GameView` for rendering and written by the `GameController` during physics and turn transitions.

### interfaces/ — IPhysics, IRenderer, IInputHandler

These abstract interfaces from the SOLID version are preserved in the project but are no longer used in the MVC main loop. They remain available for future extension or testing (e.g. injecting a mock physics engine for unit tests).

---

## Class Responsibility Table

| Class | Layer | Reads Input | Reads Model | Writes Model | Runs Physics | Renders |
|---|---|---|---|---|---|---|
| `GameModel` | Model | ✗ | ✓ (self) | ✓ (self) | ✗ | ✗ |
| `GameView` | View | ✗ | ✓ (const) | ✗ | ✗ | ✓ |
| `GameController` | Controller | ✓ | ✓ | ✓ | ✓ | ✗ |
| `BallManager` | Support | ✗ | ✓ | ✓ (via Controller) | ✗ | ✗ |
| `TurnManager` | Support | ✗ | ✓ | ✓ (via Controller) | ✗ | ✗ |
| `Ball` subclasses | Support | ✗ | ✓ (self) | ✓ (velocity/pocketed) | ✗ | ✓ (self) |

---

## Key Design Decisions

**1. Physics moved into the Controller, not a separate class.**
The original `PhysicsEngine` was a standalone class that received `Game*` and wrote directly to its fields. In MVC, the Controller already has full write access to `GameModel`, so inlining physics into `GameController` removes an unnecessary indirection while keeping it out of the Model and View. The logic is identical — just reorganized.

**2. The View receives a `const` reference to the Model.**
`GameView::Render(const GameModel& model)` is enforced at the compiler level. The View physically cannot write to the Model. This is a stronger guarantee than a coding convention.

**3. All original files are preserved.**
`balls/`, `managers/`, `interfaces/`, `config.h`, and `types.h` are copied unchanged. The MVC refactor does not rewrite or delete any pre-existing code — it builds on top of it by introducing the `mvc/` folder as a new layer.

**4. `main.cpp` is the only orchestrator.**
Unlike the SOLID version where `Game` wired everything together internally, in MVC the wiring is explicit and visible in `main.cpp`. Any developer reading the file immediately sees the three components and the two-line game loop.

**5. No observer or event bus.**
raylib's `IsKeyPressed` / `IsMouseButtonDown` API is already a frame-synchronous polling model. Adding a separate notification pattern would replicate raylib's own infrastructure. The current approach — Controller polls input, Controller runs physics, View reads model — is idiomatic for raylib game architecture.

---

## Building and Running

### Run directly (no installation needed)

Download `8BallPool_MVC_Final.zip`, extract it, and double-click `8ballpool_MVC.exe`. No raylib installation required — the executable is statically linked.

### Build from source (Windows with raylib)

**Prerequisites:** MinGW-w64, raylib installed at `C:\raylib\`

```bat
build.bat
```

This runs:

```bat
C:\raylib\w64devkit\bin\g++.exe ^
    main.cpp ^
    balls/Ball.cpp ^
    -I. ^
    -IC:\raylib\raylib\src ^
    -LC:\raylib\raylib\src ^
    -std=c++17 ^
    -lraylib -lopengl32 -lgdi32 -lwinmm ^
    -o 8ballpool_MVC.exe
```

Note: `core/PhysicsEngine.cpp`, `core/Renderer.cpp`, and `core/InputHandler.cpp` are **no longer compiled** — their logic now lives inside `mvc/GameController.h` and `mvc/GameView.h` as header-only implementations.

### Game Controls

| Action | Control |
|---|---|
| Aim | Move mouse around the white cue ball |
| Set power | Click the cue ball and drag backward |
| Shoot | Release the mouse button |
| Place cue ball (after scratch) | Click anywhere inside the rails |
| Restart | Press **R** |
