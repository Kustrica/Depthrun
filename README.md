# Depthrun

> **Diploma project** — a 2D top-down roguelike shooter built in **Unreal Engine 5.7** with **Paper2D**.
> The engineering deliverable is a custom **Adaptive Enemy Behavior** module: a 3-layer decision-making system implemented in pure C++ with no built-in UE AI infrastructure (no Behavior Trees, no Blackboards, no EQS, no ML).

![Engine](https://img.shields.io/badge/Engine-Unreal%20Engine%205.7-313131?logo=unrealengine)
![Language](https://img.shields.io/badge/Language-C%2B%2B%20100%25-00599C?logo=cplusplus)
![Genre](https://img.shields.io/badge/Genre-2D%20Top--Down%20Roguelike-purple)

---

## Table of Contents

1. [Project Overview](#project-overview)
2. [Key Features](#key-features)
3. [The Adaptive Behavior Module](#the-adaptive-behavior-module)
4. [Tech Stack](#tech-stack)
5. [Source Layout](#source-layout)
6. [Gameplay Loop](#gameplay-loop)
7. [Procedural Room Generation](#procedural-room-generation)
8. [Item & Upgrade Systems](#item--upgrade-systems)
9. [Save System (SQLite)](#save-system-sqlite)
10. [Running the Prebuilt Game](#running-the-prebuilt-game)
11. [Building and Running from Source](#building-and-running-from-source)
12. [Troubleshooting](#troubleshooting)
13. [Console Commands](#console-commands)
14. [Debug Tools](#debug-tools)

---

`W, A, S, D - Movement`
`LMB – Attack`
`1 – Sword`
`2 – Bow`
`E – Use Potion`

`F3 – Open DebugWidget`
`Y – God Mode`
`U – Super Attack`

## Project Overview

Depthrun is a **demonstration polygon** for a diploma research project. The game is fully playable, but its primary purpose is to showcase a working software module that implements adaptive enemy behavior — a 3-layer pipeline that turns scalar context (distance, HP, weapon type, density, memory) into a high-level FSM decision while continuously adapting its own weights based on combat feedback.

The committee deliverable is **not** the game; it is the `AdaptiveBehavior` module. The game exists so the module has somewhere to live and something to react to.

**Target platform:** Windows x64.
**Target audience:** Diploma defense committee + anyone interested in shippable, non-ML adaptive AI patterns in UE5.

---

## Key Features

### Gameplay
- **Procedurally generated dungeon** of multi-cell rooms (1×1, 2×1, 1×2, 2×2, L-shape) on a grid of 8×6 tiles per cell.
- **Two-slot weapon system** — Sword (slot 1) and Bow (slot 2), instant switch with keys `1` / `2`.
- **Per-run item system** with 7 modifiers (Ricochet, Pierce, Extended Range, Double Swing, Flat Damage, Bonus HP, Move Speed, Multishot) applied at pickup and discarded on run end.
- **Chest loot** — every chest always rolls Diamonds + Health Potions + a 70% chance for a Run Item, configured via `DA_ChestLootConfig`.
- **Hub metaprogression** between runs — persistent upgrades (Damage, Range, Arrow Count, Max HP), bought with diamonds carried over from previous runs.
- **Boss room** ends the run with a Victory screen and 100 % diamond payout; death pays out 50 %.
- **VFX** powered by Niagara — hit (melee/ranged), death, chest open, victory, defeat.
- **Music system** — `UMusicSubsystem` with two `UAudioComponent`s and seamless crossfade; combat trigger reacts to active rooms.

### Engineering
- **Pure C++ FSM** with 5 states: `Idle / Chase / Attack / Retreat / Flank` — no Blueprint, no Behavior Tree.
- **3-layer Adaptive Behavior overlay** that decides which state to enter, with dynamic weights, time-decay memory, N-gram pattern recognition, and utility curves.
- **A/B-testable** via the `bAdaptiveEnabled` flag on the component — disable the overlay to fall back to vanilla FSM, useful for empirical comparison during defense.
- **Real-time debug widget** showing `T_final`, every weight `w_i`, every state's `FinalScore`, the recognized N-gram, and the current FSM state.
- **CSV export** via `UDemonstrationSubsystem` for diploma plots and charts.

---

## The Adaptive Behavior Module

This is the heart of the project. It lives in `Source/Depthrun/AdaptiveBehavior/` and is attached to every enemy as a `UAdaptiveBehaviorComponent`. Evaluation runs on a `FTimerManager` tick every **0.3 s** (configurable 0.2–0.5 s) — never per-frame.

### Layer 1 — Context Collection (`ContextEvaluator`)
Reads raw world state for the owning enemy and normalizes everything into the `[0, 1]` range:

| Factor | Source | Normalization |
|---|---|---|
| `DistanceNorm` | distance to player | linear |
| `WeaponThreatNorm` | active player weapon (melee 0.6 / ranged 0.8) | discrete lookup |
| `EnemyHPRatioNorm` | enemy HP / MaxHP | quadratic (`α=2`) |
| `AllyCountNorm` | allies within support radius | clamp(count / 5) |
| `RoomDensityNorm` | entities within 600 units | clamp(count / 8) |
| `MemoryAggressiveness` + `MemoryMobility` | from `AdaptiveMemory` | window counter |

### Layer 2 — Threat Calculation (`ThreatCalculator`)
Computes the scalar threat used by Layer 3:

```
T_final = clamp( Σ w_i × f_i , 0, 1 )
```

No cross-terms, no exponential smoothing, no confidence fallback — deliberately kept simple and explainable for defense. Weights `w_1…w_6` come from the **`DynamicWeightManager`**, which adjusts them with a deterministic feedback loop:

```
w_i += η · reward · contribution_i
contribution_i = f_i(x_i) / Σ f_j(x_j)
```

Reward is `+1.0` when the enemy successfully damages the player, `−1.0` when the enemy takes damage. This is **not** machine learning — it is a transparent, fully traceable rule.

### Layer 3 — State Selection (`StateTransitionResolver`)
For each of the 5 FSM states evaluates:

```
FinalScore(s) = Utility(s, T_final, Context)  −  TransitionCost(current → s)
                                              +  InertiaBonus(current, s, TimeInState)
                                              +  PatternModifier(s)
```

- **Utility** — bell-curve / ramp-curve per state (`UtilityCurves`), peaks at the threat level each state is designed to handle.
- **TransitionCost** — 5×5 matrix (`TransitionCostMatrix`), penalizes thrashing between distant states.
- **InertiaBonus** — `min(InertiaMax, rate × TimeInState)`, rewards staying in the current state to suppress jitter.
- **PatternModifier** — bonus/penalty per recognized N-gram (e.g. *Shot+Shot* boosts Flank by `+0.65`).

The state with the highest `FinalScore` wins. If it differs from the current state, `FSMComponent::TransitionTo` fires.

### Supporting Systems
- **`AdaptiveMemory`** — short-term memory of player actions. Aggressiveness = `count(Shot+Melee in last 10 s) / 10`.
- **`PatternRecognizer`** — sliding window of 15 player actions; counts 2-grams and 3-grams; returns the dominant pattern.
- **`UAdaptiveConfig` (UDataAsset)** — every tunable constant (η, λ, curve params, window sizes, hysteresis points) lives here and can be edited in the Editor without recompilation.

### Combat Range Architecture
Each `ABaseEnemy` exposes split ranges:
- `MeleeAttackRange` — melee attack entry (default 80)
- `RangedAttackRange` — optimal ranged distance (default 320, 450 for `ARangedEnemy`)
- `RangedTooFarRange` — break out of ranged Attack back to Chase (default 480)
- `MinAttackRange` — too close → Retreat
- `SafeDistance` — Retreat exit distance

`GetEffectiveAttackRange()` is virtual; `AAdaptiveEnemy` overrides it based on `bIsRangedMode`. The melee↔ranged switch uses **hysteresis** (separate Up/Down thresholds biased by personality `CombatStyle`) to avoid mode-flip oscillation.

---

## Tech Stack

| Layer | Technology |
|---|---|
| Engine | Unreal Engine **5.7** |
| Rendering | Paper2D (2D top-down, gravity = 0, all movement on XY plane) |
| Input | Enhanced Input |
| Effects | Niagara |
| Persistence | SQLiteCore (built-in UE 5 plugin) |
| UI | UMG layout + C++ logic (no Blueprint logic in widgets) |
| Audio | `UAudioComponent` × 2 + crossfade via `UMusicSubsystem` |
| Language | **100 % C++** for FSM, AdaptiveBehavior, gameplay, UI logic |

---

## Source Layout

```
Source/Depthrun/
├── Core/             — GameMode, GameInstance, log channels
├── Player/           — DepthrunCharacter, CombatComponent, PlayerEconomy, ActionTracker
├── Enemy/            — BaseEnemy, MeleeEnemy, RangedEnemy, AdaptiveEnemy
├── FSM/              — FSMComponent + 5 FSMState_* subclasses
├── AdaptiveBehavior/ — ★ THE CORE MODULE
│   ├── AdaptiveBehaviorComponent  — orchestrator (timer tick, layer pipeline)
│   ├── ContextEvaluator           — Layer 1: factor normalization
│   ├── ThreatCalculator           — Layer 2: weighted sum → T_final
│   ├── StateTransitionResolver    — Layer 3: argmax FinalScore
│   ├── AdaptiveMemory             — short-term player-action memory
│   ├── PatternRecognizer          — 2/3-gram frequency analysis
│   ├── DynamicWeightManager       — feedback-loop weight adjustment
│   ├── UtilityCurves              — per-state utility functions
│   ├── TransitionCostMatrix       — 5×5 cost + inertia
│   └── AdaptiveConfig (DataAsset) — all tunables live here
├── Combat/           — BaseWeapon, MeleeWeapon, RangedWeapon, Projectile, ChestActor
├── Items/            — RunItemInventory, RunItemConfig, RunItemCollection
├── RoomGeneration/   — RoomBase, RoomTemplate, RoomGeneratorSubsystem, TrapdoorActor
├── UI/               — HUDOverlayWidget, HealthBar, PauseMenu, DeathScreen, VictoryScreen,
│                       DebugAdaptiveWidget, ChestRewardWidget, HubWidget, MainMenuWidget
├── Data/             — DepthrunSaveSubsystem, SQLite schema
├── Audio/            — MusicSubsystem, CombatMusicTrigger, UISoundLibrary
└── Utils/            — math helpers (sigmoid, bell, decay)
```

---

## Gameplay Loop

```
L_MainMenu ──▶ L_Hub ──▶ L_Gameplay ──▶ (death OR boss cleared) ──▶ L_Hub
                ▲                                                        │
                └────────────────────────────────────────────────────────┘
```

1. **Main Menu** — Play / Settings / Quit. Settings (master volume) persist in `UDepthrunGameInstance`.
2. **Hub** — view profile, spend persistent diamonds on 4 upgrades, start a run.
3. **Run** — procedural dungeon. Each combat room locks doors → spawns enemies → on clear unlocks doors + may spawn a chest (`bSpawnChest`, 10 % default).
4. **Boss room** — clearing spawns the exit `ATrapdoorActor`; entering it triggers Victory (100 % diamonds).
5. **Death** — Game Over screen, 3 s fade, 50 % diamonds banked, return to Hub.
6. Every run records a row into `run_history` regardless of outcome.

---

## Procedural Room Generation

- Grid-based, 8×6 tiles per cell (16×16 px each, ~332 units wide at scale 2.6).
- Multi-cell rooms supported: 1×1, 2×1, 1×2, L-shape (3 cells), 2×2.
- Z is **single-source-of-truth** via `RoomTemplate`: `PlayerLockedZ`, `EnemyLockedZ`, `DoorLockedZ` — no hidden clamping.
- Tile layers:
  - Layer 0 — Floor (regular + Shadowed Floor in doorways)
  - Layer 1 — Door layer
  - Layer 2 — Walls (auto-built to seal blocked passages)
- Props placed in code: torches on top walls (not in corners, not in doorways, 50 % chance), bones on floor, chest 10 % chance.
- **Start rooms** never spawn doors and never lock — clean passage for the player.
- **Enemy spawn safety:** `OccupiedTiles` (doorway cells) are skipped to prevent enemies blocking exits.

---

## Item & Upgrade Systems

### Run Items (per-run, lost on death)
| Effect | Weapon | Description | Param |
|---|---|---|---|
| `ArrowRicochet` | Bow | Arrows ricochet between targets | `RicochetCount = 2` |
| `ArrowPierce` | Bow | Arrows pierce enemies | — |
| `MeleeExtendedRange` | Sword | Increased swing radius | `×1.5` |
| `MeleeDoubleSwing` | Sword | Two swings per attack | — |
| `FlatDamage` | Any | +5 flat damage | — |
| `BonusMaxHP` | Any | +25 MaxHP | `25` |
| `BonusMoveSpeed` | Any | +15 % move speed | `0.15` |
| `Multishot` (`BonusProjectileCount`) | Ranged | +1 arrow per shot (max 5) | `1` |

All items are pre-filled into a single `DA_RunItemCollection` DataAsset. Effects apply via `ApplyToWeapon()` + `ApplyToCharacter()`.

### Hub Upgrades (persistent)
| Upgrade | Max Level | Cost Formula |
|---|---|---|
| Damage | 5 | `floor(50 × 1.6^level)` |
| Range | 5 | `floor(50 × 1.6^level)` |
| ArrowCount | 3 | `floor(150 × 2^level)` |
| MaxHP | 5 | `floor(50 × 1.6^level)` |

---

## Save System (SQLite)

**Subsystem:** `UDepthrunSaveSubsystem` (GameInstance scope)
**Path in code:** `FPaths::ProjectSavedDir() / "Depthrun.db"`
**Viewer:** the VSCode extension `alexcvzz.vscode-sqlite` works out of the box.

**Where `Depthrun.db` actually lives, by launch mode:**

| Launch | Path |
|---|---|
| Editor / PIE | `<ProjectFolder>/Saved/Depthrun.db` |
| Packaged build (Windows) | `%LOCALAPPDATA%\Depthrun\Saved\Depthrun.db` |

> ⚠ In a packaged build the DB is stored under `%LOCALAPPDATA%` and is **not** wiped by rebuilding or reinstalling the game. To reset progress, delete `Depthrun.db` at that path (or the whole `Saved` folder), or run `ResetProfileCmd` in the in-game console.

### Schema

**`player_profile`** (single row, `id = 1`)
| Column | Type | Notes |
|---|---|---|
| `id` | INTEGER PK | always 1 |
| `TotalDiamonds` | INTEGER | persistent diamonds |
| `Damage_Lvl` | INTEGER | 0–5 |
| `Range_Lvl` | INTEGER | 0–5 |
| `ArrowCount_Lvl` | INTEGER | 0–3 |
| `MaxHP_Lvl` | INTEGER | 0–5 |

**`run_history`** (one row per run)
| Column | Type | Notes |
|---|---|---|
| `id` | INTEGER PK | rowid |
| `Rooms` | INTEGER | cleared rooms count |
| `Won` | INTEGER | 1 = victory, 0 = death |
| `RunDuration` | REAL | seconds |
| `Timestamp` | TEXT | `datetime('now')` |

Run results are written **once per run**: `ShowVictoryScreen()` calls `SaveRunResult(..., true)` after committing 100 % diamonds; `Die()` calls `SaveRunResult(..., false)` after committing 50 % diamonds.

---

## Running the Prebuilt Game

The simplest path. No additional software is required (except possibly the Visual C++ Redistributable, see [Troubleshooting](#troubleshooting)).

1. Open the folder with the prebuilt game (`Game/` from the archive, or your own build produced via `File → Package Project`).
2. Find `Depthrun.exe`.
3. Double-click to launch.
4. The game starts in fullscreen: main menu, **Play** button.

Quit via the in-game menu or `Alt + F4`.

---

## Building and Running from Source

Use this path to open the project in the Unreal Engine editor, study the code, modify logic, or produce your own `.exe`.

### Installing Unreal Engine 5.7

1. Download **Epic Games Launcher** from the official site: <https://store.epicgames.com/en-US/download>
2. Install and sign in (free Epic Games account).
3. Open the **Unreal Engine** tab → **Library**.
4. Click **+** next to *Engine Versions* and pick **5.7**.
5. Click **Install** (60–100 GB on disk).

### Installing Visual Studio 2022

Without Visual Studio the project will not build — the engine cannot compile the C++ sources.

1. Download **Visual Studio 2022 Community** (free edition): <https://visualstudio.microsoft.com/downloads/>
2. In the installer's **Workloads** tab, enable **Game development with C++**.
3. In the **Individual components** list make sure these are checked:
   - **MSVC v143** (latest)
   - **Windows 10 SDK** or **Windows 11 SDK**
4. Click **Install** (~15–30 GB).
5. Reboot after installation.

### Opening the Project

1. Go to the source folder (`Source/` from the archive or the repository root).
2. Right-click `Depthrun.uproject` → **Generate Visual Studio project files**. Wait 1–3 minutes.
3. Double-click `Depthrun.uproject`. A dialog may offer to rebuild modules — click **Yes**. The first compile takes 5–15 minutes.
4. The editor opens. The **Play** button in the top toolbar runs the game in PIE.

### Packaging a Standalone `.exe`

1. Open the project in the editor.
2. From the top menu pick **Platforms → Windows → Package Project**.
3. Choose a target folder and wait for the build to finish.
4. A standalone `Depthrun.exe` with all assets appears in that folder.

The package must include all three levels. The project ships with the correct entry in `Config/DefaultGame.ini`:

```ini
[/Script/UnrealEd.ProjectPackagingSettings]
+MapsToCook=(FilePath="/Game/Levels/L_MainMenu")
+MapsToCook=(FilePath="/Game/Levels/L_Hub")
+MapsToCook=(FilePath="/Game/Levels/L_Gameplay")
```

Without these entries the Play button in the packaged build silently fails — the cooker only ships the `GameDefaultMap` and `OpenLevel("L_Hub")` falls through.

### Building via Visual Studio

Alternative for development:

1. Open `Depthrun.sln` (generated by **Generate Visual Studio project files**).
2. Set configuration to **`Development Editor`** and platform to **`Win64`**.
3. **Build → Build Solution** or press **F7**.
4. After a successful build, launch the editor via `Depthrun.uproject` or run it from VS with **F5**.

> ⚠ **Critical:** always use **Development Editor** for the editor, never *DebugGame Editor*. DebugGame produces a different `.dll` that the UE Editor refuses to load.

---

## Troubleshooting

**First thing to try for any weirdness — turn off your VPN.**

Many Unreal Engine and project glitches come from an active VPN connection:

- `Depthrun.uproject` won't open, the engine hangs or throws an error;
- no audio in the Unreal Engine editor;
- assets fail to load, the editor takes ages to start;
- random compile failures.

Disable the VPN completely, close the editor, and try again — that fixes it most of the time.

### `Depthrun.exe` fails with a missing-DLL error (`MSVCP140.dll`, `VCRUNTIME140.dll`, etc.)

Install **Microsoft Visual C++ Redistributable for Visual Studio 2015–2022 (x64)** from the official Microsoft site: <https://aka.ms/vs/17/release/vc_redist.x64.exe>
Reboot and try again.

### Windows SmartScreen blocks the `.exe`

Click **More info → Run anyway**. The build has no paid code-signing certificate, which is expected.

### Low FPS on a laptop

Make sure the game uses the discrete GPU, not the integrated one.
**Windows Settings → Display → Graphics** → add `Depthrun.exe` → **High performance**.

### `Depthrun.uproject` opens with a module compilation error

Verify that **Unreal Engine 5.7** and **Visual Studio 2022** with the *Game development with C++* workload are installed. Then right-click `Depthrun.uproject` → **Generate Visual Studio project files**, open `Depthrun.sln`, and build in **`Development Editor | Win64`**.

### Sound drops out or input behaves strangely

Restart the game. If that doesn't help, delete the `Saved/` folder next to the `.exe` (or in the project's `Source/` directory) — it is recreated automatically.

---

## Console Commands

Open the console with `~` in PIE.

### Economy
| Command | Effect |
|---|---|
| `AddRunDiamonds <N>` | +N diamonds to the current run |
| `AddProfileDiamonds <N>` | +N diamonds to the profile (persistent) |
| `BuyUpgradeCmd <Type>` | Buy upgrade: `Damage` / `Range` / `ArrowCount` / `MaxHP` |
| `ShowProfile` | Print profile (TotalDiamonds + upgrade levels) |
| `ResetProfileCmd` | Reset profile to defaults |

### Items
| Command | Effect |
|---|---|
| `ListItems` | List every item in `DA_RunItemCollection` |
| `GiveItem <partial-name>` | Give an item by partial name match |
| `ClearRunItems` | Clear run inventory, restore base stats |

### Database
| Command | Effect |
|---|---|
| `DBCheck` | Dump `player_profile` and all `run_history` rows to the Output Log |

### Debug
| Command | Effect |
|---|---|
| `ToggleGodMode` | Invulnerability on/off |
| `ToggleSuperAttack` | Super-damage on/off |

---

## Debug Tools

- **`UDebugAdaptiveWidget`** — real-time overlay above each enemy showing `T_final`, every weight, every state's `FinalScore`, the recognized N-gram, and the current FSM state. Mandatory for defense — the committee must *see* the module thinking.
- **`bAdaptiveEnabled`** — flag on `UAdaptiveBehaviorComponent`. Untick in the Editor or set at runtime to disable the overlay and fall back to vanilla FSM, side-by-side A/B comparison.
- **`UDemonstrationSubsystem`** — exports decisions, weights and threat values into CSV files (under `Saved/`) for diploma charts.
- **Log categories** — `LogDepthrun`, `LogAdaptiveBehavior`, `LogDepthrunLoot`, `LogDepthrunSave`. Filter the Output Log to follow any subsystem.

---

A Russian version of this document is available at [`README_ru.md`](./README_ru.md).
