# 0714-02-CSE-2100
Course Code: 0714 02 CSE 2100 || Course Title : Advanced Programming Laboratory
 || ID : 240232 , 240235

# 8 Ball Pool Game: C to C++ Transition with OOD and SOLID Implementation

**Course:** Advanced Programming Lab (2nd Year CSE)  
**Project:** 8 Ball Pool Game   

---

## 1. Executive Overview

This report outlines a deliberate and step-by-step transition from a procedural C implementation to a modern C++ architecture for the 8 Ball Pool Game, with focused attention on:

1. Object-Oriented Design (OOD)
2. SOLID principles implementation
3. Incremental migration without gameplay regression
4. Maintainability, testability, and extensibility

The aim goes beyond updating language syntax — the core intention is to redraw responsibility boundaries so the game stays correct, grows cleanly, and can be extended without disturbing what already works.

---

## 2. Why Transition from C to C++

Both engineering needs and academic goals make this transition worthwhile.

### Engineering motivations

1. Classes and access control allow ball state, player data, and physics behavior to be properly enclosed rather than left as open struct fields.
2. Interfaces and polymorphism replace manual type-checking with clean, contract-driven module boundaries.
3. Memory safety improves through RAII (`BallManager`, `std::unique_ptr`, `std::vector`).
4. The system becomes easier to extend — new ball types, rendering styles, rule variants, and input schemes can be added without touching stable existing code.
5. Dependency injection through interface pointers makes every subsystem independently swappable and testable.

### Academic motivations

1. Demonstrates practical application of OOD inside a real-time interactive game project.
2. Demonstrates applied SOLID in a domain that combines physics simulation with game rule logic.
3. Demonstrates how design patterns resolve real architectural problems under genuine constraints.

---

## 3. Transition Objectives

### Primary objectives

1. Gameplay correctness must be fully preserved — physics, pocketing, turn logic, and win/loss conditions all produce identical results.
2. The overloaded `Game` struct and god-file pattern must be replaced with small, focused domain classes.
3. Clear abstraction layers must be introduced:
- Presentation and input
- Game logic and rules
- Core physics and ball domain
4. All five SOLID principles must be present and traceable in the final code structure.

### Non-objectives

1. No risky big-bang rewrite.
2. No algorithmic redesign of the physics simulation logic.
3. No removal of working behavior before a tested replacement is ready.

---

## 4. Target OOD Architecture

### Layered model

1. **Presentation Layer**
- Renderer (table, balls, cue stick, power bar, HUD, overlays)
- Input handler (mouse aiming, drag-to-shoot, scratch placement)

2. **Application/Service Layer**
- Game controller (orchestrates subsystems, drives the game loop)
- Turn manager (player turns, type assignment, current/other player)

3. **Domain Layer**
- Ball hierarchy and state (position, velocity, pocketed flag)
- Ball manager (lifetime ownership, reset, movement queries)
- Rule logic (pocket consequences, scratch, win, loss)

4. **Infrastructure Layer**
- Physics engine (movement, friction, bouncing, collision resolution)
- Configuration constants
- Utility helpers (distance, speed clamping)

### Dependency rule

All dependencies travel in one direction only — downward:

`Presentation -> Application -> Domain -> Infrastructure`

No domain class carries any knowledge of rendering or input concerns above it.

---

## 5. Core Class and Interface Design

### Key classes (concrete)

1. `Ball` (base), `CueBall`, `SolidBall`, `StripeBall`, `EightBall`
2. `BallManager`
3. `TurnManager`
4. `PhysicsEngine`
5. `Renderer`
6. `InputHandler`
7. `Game`

### Key interfaces (contracts)

1. `IPhysics` (physics simulation update contract)
2. `IRenderer` (rendering backend contract)
3. `IInputHandler` (input processing contract)
4. `IDrawable` (any object that can draw itself)
5. `IBallFactory` (ball construction and rack layout contract)
6. `IRuleJudge` (game rule evaluation contract — pocket consequences, win/loss)
7. `IGameState` (narrow read/write contract so subsystems access game state without depending on the concrete `Game` class)

This separation is what gives both ISP and DIP meaningful enforcement across the architecture.

---

## 6. SOLID Implementation Plan

## 6.1 S: Single Responsibility Principle (SRP)

### Problem pattern
A single overloaded-file (`game.c`) and an oversized struct (`Game`) packed input handling, turn switching, scratch detection, win checking, ball setup, and status messages all into one place. Any edit to one concern risked silently breaking another.

### Implementation
1. Distribute responsibilities across focused, single-purpose classes:
- `InputHandler` — processes mouse and keyboard input only
- `TurnManager` — manages player turns and type assignment only
- `BallManager` — owns ball lifetime and movement queries only
- `PhysicsEngine` — simulates movement, friction, and collisions only

2. Each `Ball` subclass is responsible for drawing itself only, with no awareness of game rules.

3. `Renderer` composes the complete frame from subsystem data only.

### Expected result
Every class has a single, clear reason to change. Updating rendering logic has no pathway to affect physics behavior.

## 6.2 O: Open/Closed Principle (OCP)

### Implementation
1. Use interface-driven extension points for:
- ball types and rack layouts
- rendering backends
- rule variants and input schemes

2. Extend the system by creating new subclasses or new interface implementations, never by modifying stable core classes.

### Expected result
New ball types, rendering backends, and rule variants are delivered through extension, not through modification of existing code.

## 6.3 L: Liskov Substitution Principle (LSP)

### Implementation
1. Define strict behavioral contracts for every interface.
2. Ensure all `Ball` subclasses are fully substitutable — identical preconditions and postconditions to the base.
3. Add interface-level test suites shared across all implementations.

### Expected result
Any ball subclass, physics implementation, or renderer can be swapped into the system without breaking the game loop.

## 6.4 I: Interface Segregation Principle (ISP)

### Implementation
1. Break broad game access into narrow, role-specific interfaces:
- `IPhysics` — physics only
- `IRenderer` — rendering only
- `IInputHandler` — input only
- `IDrawable` — drawing only

2. Each module depends only on the interface methods it actually uses.

### Expected result
Coupling is reduced and the compile-time dependency surface shrinks across the system.

## 6.5 D: Dependency Inversion Principle (DIP)

### Implementation
1. High-level modules depend on abstractions (`IPhysics`, `IRenderer`, `IInputHandler`).
2. Concrete implementations are wired in through constructor injection or factory setup.
3. Direct calls from `Game` into concrete physics and rendering internals are removed entirely.

### Expected result
The architecture becomes testable, every subsystem becomes replaceable, and the system stays stable when individual parts change.

---

## 7. Design Patterns Used

1. **Strategy Pattern**
- interchangeable physics implementations and rendering backends

2. **Factory Pattern**
- centralized construction of balls and rack layout via `IBallFactory`

3. **Template Method Pattern**
- `Ball::Draw()` defines the contract; subclasses override selectively

4. **Facade Pattern**
- `Game` acts as a simplified facade; the main loop calls only `Update()` and `Draw()`

5. **Observer Pattern** (optional expansion)
- notifications for game events (ball pocketed, scratch, win, loss) broadcast to decoupled listeners

---

## 8. Migration Roadmap (Phased)

## Phase 1: Baseline and Safety Net

1. Freeze the working C version as the functional baseline.
2. Document expected gameplay behavior: break shot, pocket logic, scratch placement, win/loss conditions.
3. Establish build verification for both versions.

**Deliverable:** confirmed working C baseline.

## Phase 2: Type Modernization

1. Convert `#define` constants to typed `constexpr` values.
2. Convert C-style enums to `enum class` (`BallType`, `GameState`, `PlayerType`).
3. Introduce typed `Vector2` usage consistently.

**Deliverable:** modernized type layer with no behavior change.

## Phase 3: Interface Introduction

1. Add `IPhysics`, `IRenderer`, `IInputHandler`.
2. Add `IDrawable` for ball rendering contracts.
3. Keep existing logic intact while wiring through interfaces.

**Deliverable:** compile-safe abstraction layer.

## Phase 4: Core Refactor

1. Introduce `Ball` base class and subclasses (`CueBall`, `SolidBall`, `StripeBall`, `EightBall`).
2. Introduce `BallManager` to own ball lifetime.
3. Introduce `TurnManager` to own player and turn state.
4. Encapsulate `Game` struct fields behind class methods.

**Deliverable:** reduced direct field coupling, clean domain objects.

## Phase 5: Service and Presentation Decoupling

1. Route rendering through `IRenderer`, input through `IInputHandler`, physics through `IPhysics`.
2. Remove direct cross-module dependencies.
3. Introduce `IBallFactory` for ball construction.

**Deliverable:** clean layered boundaries, fully injectable subsystems.

## Phase 6: Hardening and Cleanup

1. Apply RAII — ensure `BallManager` destructor handles all heap cleanup correctly.
2. Disable copy constructors where ownership semantics require it.
3. Fix identified logical bugs (aiming line direction, rule/physics separation).
4. Final static analysis and SOLID review.

**Deliverable:** production-ready OOD/SOLID architecture.

---

## 9. Risk Management

### Key risks

1. Functional regression in pocketing logic, scratch placement, or win/loss detection.
2. Hidden dependency breakage when encapsulating the `Game` struct fields.
3. Vtable dispatch overhead in the hot physics loop.

### Mitigations

1. Build and run the game after every incremental change — regressions are caught at their source.
2. Keep the original C version compilable throughout the transition as a reference point.
3. After every refactor phase, confirm all 15 balls pocket correctly and all win/loss conditions fire at the right moment.
4. Benchmark the physics loop before and after virtual dispatch is introduced.
5. Always maintain a working game build — never leave the main loop in a broken state mid-refactor.

---

## 10. Validation and Quality Gates

The transition is accepted only when every check below passes without exception:

1. Game compiles cleanly with no warnings under `-std=c++17`.
2. Break shot behavior, ball collisions, and friction all match the C baseline exactly.
3. All 6 pockets correctly detect and pocket balls.
4. Scratch placement flow completes correctly after the cue ball is pocketed.
5. Win and loss states trigger correctly (8-ball pocketed at right/wrong time).
6. Player type assignment (Solids/Stripes) triggers correctly on the first pocket.
7. No new static analysis findings are introduced by the refactor.
8. Frame rate holds at target FPS with no physics regression.

---

## 11. **AI Prompt Set for Execution**


## Prompt 1 - C Project Review
"I have a working 8 Ball Pool game written in C using raylib. The entire game state lives in one Game struct inside common.h, and most of the logic is in game.c. Read through the files and tell me what the main structural problems are and which parts would be hardest to extend or test."

## Prompt 2 - Full C to C++ Conversion Request
"I am sharing all the files of my C project with you — main.c, common.h, config.h, game.c, game.h, physics.c, physics.h, graphics.c, graphics.h, utils.c, and utils.h. I want to convert this entire project to C++ with proper object-oriented design. Explain how I should approach this conversion, what needs to change in each file, how the folder structure should look in C++, and what new classes and interfaces I need to introduce to make the design clean and maintainable."

## Prompt 3 - Ball Type Refactor
"In my C project, DrawBalls() in graphics.c uses if/else chains to check isStriped and type for every ball. I want to replace this with a Ball base class in C++ where each subclass like CueBall, SolidBall, StripeBall, and EightBall handles its own Draw(). Show me how to do this without changing how any ball looks on screen."

## Prompt 4 - BallManager Introduction
"My C project stores all balls in an array inside the Game struct and ResetBalls() in game.c sets them all up manually. I want to move this into a dedicated BallManager class that owns the ball array, handles cleanup automatically, and resets through an IBallFactory interface. How should I structure this?"

## Prompt 5 - TurnManager Extraction
"In my C project, player data and turn switching are embedded directly inside game.c using the players array and currentPlayer index on the Game struct. I want to extract this into a standalone TurnManager class. What should TurnManager own and what methods should it expose?"

## Prompt 6 - Interface Injection into Game
"My C++ Game class currently creates PhysicsEngine, Renderer, and InputHandler objects directly inside itself. I want to change this so Game only holds IPhysics, IRenderer, and IInputHandler pointers and receives the concrete objects through its constructor. Show me what this looks like in Game.h and main.cpp."

## Prompt 7 - SRP Violation in PhysicsEngine
"In my C++ project, PhysicsEngine::CheckPockets() does two things — it detects when a ball enters a pocket and also decides what happens next, like applying scratch or setting the win/loss state. This is an SRP violation. How do I separate pocket detection from rule consequences using an IRuleJudge interface?"

## Prompt 8 - ISP Improvement with IGameState
"All three of my interfaces — IPhysics, IRenderer, and IInputHandler — currently receive the full concrete Game pointer. This gives each subsystem access to more state than it needs. I want to introduce an IGameState interface that exposes only what each subsystem actually requires. What should IGameState look like?"

## Prompt 9 - Bug Fixes
"I found these bugs in both my C and C++ projects. The aiming guide line in DrawCueStick points toward the mouse instead of the shot direction. The win condition check is split across two files. ApplyScratch and NextTurn contain identical logic. The ball number field gets overwritten in the reset loop after the constructor already set it. Fix each one and explain what was wrong."

## Prompt 10 - RAII and Copy Safety
"In my BallManager class, I have a raw pointer array and a destructor that deletes everything. I have already disabled copy construction and copy assignment. I want to confirm this is safe and check if there are any other places in the C++ project where ownership is unclear or memory could leak."

## Prompt 11 - Final SOLID Audit
"Do a final review of my complete C++ project — all the ball classes, BallManager, TurnManager, PhysicsEngine, Renderer, InputHandler, and Game. For each SOLID principle, tell me what is well applied and what still needs improvement. Rank any remaining issues by severity and suggest a fix for each one."


---

## Conclusion

This transition establishes a clear and disciplined path from procedural C code to a modern C++ architecture built around OOD and SOLID. The design keeps gameplay correct throughout while steadily improving maintainability, testability, and extensibility through incremental refactoring, strict validation gates, and interface-driven module boundaries.

The outcome is an 8 Ball Pool architecture that holds up to both academic evaluation and longer-term engineering evolution.
