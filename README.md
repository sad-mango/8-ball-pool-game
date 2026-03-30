# 0714-02-CSE-2100
Course Code: 0714 02 CSE 2100 || Course Title : Advanced Programming Laboratory
 || ID : 240232 , 240235

# 8 Ball Pool Game: C to C++ Transition with OOD and SOLID Implementation

**Course:** Advanced Programming Lab (2nd Year CSE)  
**Project:** 8 Ball Pool Game   

---

## 1. Executive Overview

This report presents a structured transition plan from legacy C-style implementation to modern C++ architecture for the 8 Ball Pool Game, with explicit focus on:

1. Object-Oriented Design (OOD)
2. SOLID principles implementation
3. Incremental migration without gameplay regression
4. Maintainability, testability, and extensibility

The goal is not only to modernize syntax, but to redesign responsibility boundaries so the game remains correct, scalable, and easier to evolve.

---

## 2. Why Transition from C to C++

The transition is justified by both engineering and academic value.

### Engineering motivations

1. Better encapsulation of ball state, player data, and physics behavior through classes and access control.
2. Cleaner module boundaries using interfaces and polymorphism instead of manual type checks.
3. Reduced memory risk via RAII (`BallManager`, `std::unique_ptr`, `std::vector`).
4. Easier feature extension (new ball types, rendering styles, rule variants, input schemes).
5. Better testability through dependency injection and interface contracts.

### Academic motivations

1. Demonstrates practical application of OOD in a real-time interactive game project.
2. Demonstrates applied SOLID in a non-trivial domain combining physics simulation and game rule logic.
3. Demonstrates design pattern usage under real architectural constraints.

---

## 3. Transition Objectives

### Primary objectives

1. Preserve gameplay correctness — physics, pocketing, turn logic, and win/loss conditions all behave identically.
2. Replace the monolithic `Game` struct and god-file design with cohesive domain classes.
3. Introduce clear abstraction layers:
- Presentation and input
- Game logic and rules
- Core physics and ball domain
4. Implement all five SOLID principles in concrete code structure.

### Non-objectives

1. No risky big-bang rewrite.
2. No algorithmic redesign of the physics simulation logic.
3. No behavior-breaking removal before a safe replacement is in place.

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

Dependencies flow downward only:

`Presentation -> Application -> Domain -> Infrastructure`

No domain class depends on rendering or input concerns.

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

This split is designed to enforce both ISP and DIP.

---

## 6. SOLID Implementation Plan

## 6.1 S: Single Responsibility Principle (SRP)

### Problem pattern
A single god-file (`game.c`) and giant struct (`Game`) mixed input processing, turn switching, scratch logic, win checking, ball initialization, and status messages all in one place.

### Implementation
1. Separate responsibilities across focused classes:
- `InputHandler` — processes mouse and keyboard input only
- `TurnManager` — manages player turns and type assignment only
- `BallManager` — owns ball lifetime and movement queries only
- `PhysicsEngine` — simulates movement, friction, and collisions only

2. Each `Ball` subclass knows how to draw itself only, with no game rule awareness.

3. `Renderer` assembles the full frame from subsystem data only.

### Expected result
Each class has one clear reason to change. A change to rendering never touches physics.

## 6.2 O: Open/Closed Principle (OCP)

### Implementation
1. Use interface-driven extension points for:
- ball types and rack layouts
- rendering backends
- rule variants and input schemes

2. Add new behavior by creating new subclasses or implementing interfaces, not modifying stable core classes.

### Expected result
New ball types, rendering backends, and rule variants are added by extension, not by modification.

## 6.3 L: Liskov Substitution Principle (LSP)

### Implementation
1. Define strict behavioral contracts for every interface.
2. Ensure all `Ball` subclasses are fully substitutable — same preconditions and postconditions.
3. Add interface-level test suites reused by all implementations.

### Expected result
Any ball subclass, physics implementation, or renderer can be swapped safely without breaking the game loop.

## 6.4 I: Interface Segregation Principle (ISP)

### Implementation
1. Split broad game access into small role-based interfaces:
- `IPhysics` — physics only
- `IRenderer` — rendering only
- `IInputHandler` — input only
- `IDrawable` — drawing only

2. Make modules depend only on the methods they actually need.

### Expected result
Reduced coupling and smaller compile-time dependency surface.

## 6.5 D: Dependency Inversion Principle (DIP)

### Implementation
1. High-level modules depend on abstractions (`IPhysics`, `IRenderer`, `IInputHandler`).
2. Concrete implementations are injected via constructor or factory wiring.
3. Remove direct calls from `Game` into concrete low-level physics and rendering internals.

### Expected result
Architecture becomes testable, replaceable, and stable under change.

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

1. Build and run the game after every incremental change — catch regressions immediately.
2. Keep the original C version runnable as a reference throughout the transition.
3. Verify all 15 balls pocket correctly and win/loss triggers correctly after each phase.
4. Profile the physics loop before and after virtual dispatch introduction.
5. Maintain a working game build at all times — never break the main loop mid-refactor.

---

## 10. Validation and Quality Gates

Transition is accepted only if all checks pass:

1. Game compiles cleanly with no warnings under `-std=c++17`.
2. Break shot, ball collisions, and friction all behave identically to the C baseline.
3. All 6 pockets correctly detect and pocket balls.
4. Scratch placement flow works correctly after cue ball is pocketed.
5. Win and loss states trigger correctly (8-ball pocketed at right/wrong time).
6. Player type assignment (Solids/Stripes) triggers correctly on first pocket.
7. No new static analysis findings introduced by the refactor.
8. Frame rate remains at target FPS with no physics regression.

---

## 11. **AI Prompt Set for Execution**


## Prompt 1 - Migration Inventory
"Analyze the 8 Ball Pool project and list where we still use old C-style code. For each file, suggest the C++ replacement in simple terms and explain the risk level. Give us a safe order to start, so we can begin with low-risk files first."

## Prompt 2 - Baseline Lock
"Create a clear baseline before we refactor anything. Include compile checks, gameplay behavior snapshots, and quick sanity checks for pocketing and win/loss logic with expected outcomes. This baseline will help us prove that behavior did not break after changes."

## Prompt 3 - Interface Scaffold
"Introduce core interfaces like IPhysics, IRenderer, IInputHandler, IDrawable, and IBallFactory. Keep adapters so old code can still run while we migrate step by step. Do this with minimal behavior change so we stay stable."

## Prompt 4 - Encapsulation Pass
"Replace direct Game struct field access with getter and setter methods in core modules. After each safe batch, make those fields private so outside code cannot change them directly. Keep this gradual and compile after every batch."

## Prompt 5 - SRP Decomposition
"Find the three biggest files that are doing too many jobs. Split each file into smaller focused classes or components, where each one has one main responsibility. Keep behavior exactly the same while doing this split."

## Prompt 6 - DIP Enforcement
"Refactor the Game controller and rendering code so they talk to interfaces, not concrete engine internals. Remove direct calls to specific physics and ball implementations. This will reduce coupling and make testing easier."

## Prompt 7 - RAII and Ownership
"Replace manual memory handling with RAII-based C++ structures. Use safe ownership tools like BallManager with a proper destructor and smart pointers where needed. Keep object lifetime behavior the same as before."

## Prompt 8 - Regression Gate
"After every refactor batch, run compile checks and gameplay checks. Compare results with the baseline and show pass or fail clearly. If there is any difference, point to the exact place where it changed."

## Prompt 9 - Performance Gate
"Benchmark game performance before and after refactoring. Measure frame rate and physics throughput using the same settings. Flag any slowdown above the accepted threshold and report where it appears."

## Prompt 10 - Final SOLID Audit
"Run a final code audit focused on SOLID quality. Rank issues by severity and explain each issue in simple words. For every remaining violation, suggest a concrete and practical fix."



## Conclusion

This transition plan establishes a disciplined path from legacy C-style procedural code to modern C++ architecture with OOD and SOLID at the center. The design balances correctness, maintainability, and extensibility through incremental refactoring, strict validation gates, and interface-driven boundaries.

The final outcome is an 8 Ball Pool architecture suitable for both academic evaluation and long-term engineering evolution.
