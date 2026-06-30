# DEPTHRUN — Project Knowledge Base

## Project Overview

**Depthrun** is a diploma project: a 2D top-down roguelike shooter built in Unreal Engine 5 (Paper2D), designed to demonstrate a custom **adaptive enemy behavior module** based on FSM and a multi-layer threat assessment system.

**Target audience:** Diploma defense committee. The game itself is a demonstration polygon — the engineering deliverable is the `AdaptiveBehavior` software module.

**Core engineering result:** A 3-layer decision-making system implemented in pure C++ with no built-in UE AI systems, featuring dynamic weight adaptation, time-decay memory, N-gram pattern recognition, and utility-based state selection.

**Core domain concepts:**
- FSM (Finite State Machine) — architectural base, 5 states: Idle / Chase / Attack / Retreat / Flank
- AdaptiveBehavior module — 3-layer overlay on top of FSM that makes decisions. Contains bAdaptiveEnabled flag for A/B testing against vanilla FSM.
- Enemy Actor — owns FSMComponent + AdaptiveBehaviorComponent, contains NO behavior logic itself
- **Weapon system:** 2 slots — Slot 1 = Sword (key 1), Slot 2 = Bow (key 2); switch via `SwitchToWeaponSlot(int32)` in `ADepthrunCharacter`
- **Item system (per-run):** `URunItemInventory` holds `FRunItemData` structs; single `URunItemCollection` DataAsset (`DA_RunItemCollection`) pre-filled with all 7 items. Effects applied via `ApplyToWeapon()` + `ApplyToCharacter()`.
  - **7 Run Item Modifiers:**
    | Effect | Weapon | Что делает | Параметр |
    |---|---|---|---|
    | `ArrowRicochet` | Bow | Рикошет стрел | RicochetCount=2 |
    | `ArrowPierce` | Bow | Пробивание врагов | — |
    | `MeleeExtendedRange` | Sword | Увеличение радиуса удара | RangeMultiplier=1.5 |
    | `MeleeDoubleSwing` | Sword | Двойной взмах | — |
    | `FlatDamage` | Any | +5 flat урона | — |
    | `BonusMaxHP` | Any | +25 к MaxHP | NumericValue=25 |
    | `BonusMoveSpeed` | Any | +15% к скорости | NumericValue=0.15 |
    | `BonusProjectileCount` (Multishot) | Ranged | +1 стрела за выстрел (max 5) | NumericValue=1 |
- **Chest loot system (9E):** `UChestLootConfig` DataAsset (`DA_ChestLootConfig`) — диапазон алмазов, кол-во зелий, шанс предмета. Каждый сундук всегда даёт: Diamonds (рандом) + Potion + RunItem (70% шанс). Предмет берётся из `DA_RunItemCollection`.
- **Metaprogression:** in Hub level (`L_Hub`) before each run; persistent data via `UDepthrunSaveSubsystem` (SQLite)

---

## Stack & Architecture

- **Engine:** Unreal Engine 5.7
- **Plugins:** Paper2D, Niagara, Enhanced Input, SQLiteCore, UMG
- **Language:** 100% C++ for FSM and AdaptiveBehavior; UMG Editor for UI layout only
- **Pattern:** Component-based. Enemy Actor owns `UFSMComponent` and `UAdaptiveBehaviorComponent` — behavior logic lives exclusively in those components
- **Source root:** `Source/Depthrun/` — subfolders: `Core/`, `Player/`, `Enemy/`, `FSM/`, `AdaptiveBehavior/`, `Combat/`, `Items/`, `RoomGeneration/`, `UI/`, `Data/`, `Utils/`
- **Level flow:** `L_MainMenu` → (Play) → `L_Hub` → (Start Run) → `L_Gameplay` (procedural dungeon)
- **Room Generation:** Grid-based system using "Room Cells" (e.g. 16x12 tiles). Supports multi-cell shapes: 1x1, 2x1, 1x2, L-shape (3 cells), 2x2 (4 cells).
- **Key commands:**
  - Build: VS → **Development Editor**, Win64 (CRITICAL: DebugGame Editor produces a different .dll that UE Editor does NOT load — always use Development Editor)
  - Include Convention: Subfolders like `Core/` are added to `PrivateIncludePaths` in `Build.cs` to allow flat `#include` statements for globally used utilities.
  - Git: `git log --oneline --graph` for defense presentation
  - CSV export: via `UDemonstrationSubsystem` for diploma charts

---

## Constraints & Rules
- **UHT Enum Order**: In `AdaptiveTypes.h` (and similar files), `UENUM` definitions must precede their usage in `USTRUCT` or `UCLASS` to avoid code generation errors ("hash is zero").

**Hard technical constraints:**
- Behavior Tree is **FORBIDDEN**. No Blackboard, no EQS, no BT nodes.
- No ML, no neural networks, no black-box systems.
- No Blueprint logic for FSM or AdaptiveBehavior — 100% C++.
- Enemy Actor must NOT contain behavior logic.
- FSM must compile independently from AdaptiveBehavior module (no circular deps).
- All `UObject*` members must be wrapped in `UPROPERTY()`.
- **Include Order (CRITICAL):** The `.generated.h` file MUST be the **last** `#include` in any header file. UnrealHeaderTool will fail otherwise.
- Adaptive evaluation via `FTimerManager` (0.2–0.5s interval), not per-frame Tick.

**Git & Workspace:**
- IDE-specific plugins (like `VisualStudioTools`) must be kept in `.gitignore` to maintain a clean repository for diploma defense.

**Naming — critical:**
- Module folder: `AdaptiveBehavior/`
- Log category: `LogAdaptiveBehavior`
- Main component: `UAdaptiveBehaviorComponent`
- Config asset: `UAdaptiveConfig`
- **NEVER use:** `AdaptiveAI`, `LogAdaptiveAI`, or `AI` in any class/folder/log name related to this module

**Task ownership:**
- `[AI]` — code, configs, folder creation in Source and Content
- `[HUMAN]` — anything done in Unreal Editor: project creation, plugin toggles, asset import, flipbook creation, collision setup, tilemap setup, VS solution generation, UMG widget layout, sprite assignment

**Stage order is strict:** Cannot start Stage N+1 before Stage N is complete.

**Debug widget (`UDebugAdaptiveWidget`) is mandatory** — the defense committee must see the module working in real time (T_final, weights, state scores, recognized pattern).

**Diploma note formatting:**
- Work on Word section drafts in `Diagram/word/`; `my.docx` is the formatting reference, and generated section 4 output is `NEWCODEX.docx`.
- In explanatory prose use "entity" terminology in Russian text (`сущность`) instead of "actor" terminology (`актор`), because it is clearer for the diploma committee.
- UE/C++ identifiers copied from implementation must use Courier New 12 pt; normal English words such as state names or actions stay in Times New Roman 14 pt unless they are exact code identifiers.
- In prose, remove Unreal prefixes (`U`, `F`, `E`, `A`) from class/struct/enum names when a human-readable name is enough; keep exact prefixes inside code fragments.
- Cambria Math is reserved for formulas only; do not use italics in ordinary text.

---**2D-специфичные ограничения (КРИТИЧНО для каждого изменения физики/коллизий):**
- Игра — **2D top-down** на **Paper2D**. Гравитация = 0 для всех акторов.
- Все движения — только по плоскости XY. Z-компонента игнорируется (использовать `GetSafeNormal2D()` и `FVector::Dist2D()` везде).
- Персонаж и враги используют **`MOVE_Flying`** (не Walking) — иначе при потере пола появляется скольжение.
- **Коллизионные капсулы** у персонажей — плоские: `HalfHeight=2`, `Radius≈14` (спрайты, не 3D-объекты).
- Правильный базовый поворот (Rotation) для спавна всех 2D-объектов в сцене (тайлмапы, враги, пропсы): `Pitch=-90, Yaw=0, Roll=90` (`-90, 0, 90`). Это гарантирует, что спрайты будут корректно лежать в плоскости XY и смотреть вверх.
- **Projectile** движется вручную через Tick `AddActorWorldOffset(bSweep=true)` — без PMC, PMC конфликтует с настройками коллизии в Blueprint.
- Враги стреляют в **любую сторону (360°)** — никаких ограничений по оси. Использовать `(PlayerLoc - EnemyLoc).GetSafeNormal()` для вычисления направления.
- Collision profile для снарядов: `Pawn=Overlap` (бьёт врагов/игрока), `WorldStatic=Block` (останавливается об стены), всё остальное = Ignore.

**КРИТИЧНО — CharacterMovement без AIController:**
- Враги не используют AIController (FSM сама управляет). По умолчанию у `UCharacterMovementComponent` физика **не запускается** если `CharacterOwner->Controller == nullptr` — CMC относится к пешке как к remote.
- Следствие: `AddMovementInput` накапливает вектор, но `Velocity` всегда 0, актор стоит на месте.
- **Решение:** в конструкторе `ABaseEnemy` обязательно ставить `CMC->bRunPhysicsWithNoController = true`. Без этого флага все настройки FSM/Adaptive бесполезны — враги не двигаются.
- Применимо ко ВСЕМ Character-pawn без AIController. Игрок не страдает от этого, потому что у него PlayerController.


### VFX System (Niagara)
- **Hit VFX:** `NS_HitMelee` / `NS_HitRanged` UPROPERTY on `ABaseEnemy` — spawned in `AAdaptiveEnemy::HandleHealthChanged()` based on `bIsRangedMode`
- **Death VFX:** `NS_Death` UPROPERTY on `ABaseEnemy` — spawned in `ABaseEnemy::OnKilled()`
- **Chest VFX:** `NS_ChestOpen` UPROPERTY on `AChestActor` — spawned in `AChestActor::DistributeLoot()`
- All use `UNiagaraFunctionLibrary::SpawnSystemAtLocation()` with `ENCPoolMethod::AutoRelease` for automatic cleanup
- Niagara systems must be created in editor and assigned in Blueprint defaults (see `TODO_USER.md`)

## Architecture Decisions & Rationale

### Why non-linear formulas (not a simple weighted sum)
A linear formula with static weights (`T = w1·D + w2·W + ...`) is a reactive system, not adaptive — equivalent to a chain of if/else with a numeric threshold. If the committee asks "where is the adaptation?" there is no answer.

**Solution — 8 enhancements on top of the base FSM (Variant E, simplified for defense clarity):**
1. Normalize all inputs to `[0, 1]` (linear for distance, quadratic α=2 for HP)
2. Dynamic weight adjustment via feedback loop (not ML — fully deterministic, explainable)
3. Simple window-counter memory: aggressiveness = count(Shot+Melee in last 10s) / 10, clamped [0,1]
4. N-gram pattern recognition for player actions (window=15, 2-grams + 3-grams)
5. Utility-based state selection with bell curves per state (replaces threshold if/else)
6. State inertia + 6×6 transition cost matrix to prevent jitter
7. Personality traits per enemy (Bravery + CombatStyle) affecting curve scoring
8. T_final = clamp(Σ w_i × f_i, 0, 1) — no cross-terms, no smoothing, no confidence fallback

**Removed in Variant E (over-engineered for a diploma demo):**
- Cross-terms (T_cross = β × f_D × f_H + ...)
- Exponential smoothing (T_smooth = α×T_raw + (1-α)×T_prev)
- Confidence scoring (C = 1/(1+σ²))
- DefaultThreat fallback, AdaptiveThresholdWindow (μ±σ shift)
- Sigmoid for distance normalization → replaced with linear
- Time-decay memory (exp(-λΔt)) → replaced with simple counter

### Why not utility AI without FSM
FSM is a mandatory requirement in the ToR and the correct architectural decision. Utility-scoring is an overlay on top of FSM, not a replacement.

### Why UObject for AdaptiveBehavior subsystems
`UObject` provides `UPROPERTY()`, automatic GC, and reusability. A monolithic class would violate SRP and be unreadable during defense.

### Why Timer instead of Tick
No need to recalculate threat every frame (60fps). `FTimerManager` at 0.2–0.5s saves CPU and produces more realistic behavior.

### Why UDataAsset for configuration
Hardcoding constants (`SigmoidSteepness`, `MemoryDecayLambda`, etc.) is unprofessional and prevents live tuning. `UAdaptiveConfig` as `UDataAsset` allows parameter changes in the Editor without recompilation.

### Academic defensibility of key formulas
- **Sigmoid for distance:** maximum sensitivity at mid-range, tapering at extremes — can show the graph at defense
- **Power α=2 for HP:** at full health the factor is negligible (0.04 at 80% HP); at critical it dominates (1.0 at 0%) — quadratic with near-zero derivative in normal zone
- **Confidence coefficient `C = 1/(1+σ²)`:** when data is unstable (chaotic player, high T_raw variance) the enemy defaults to conservative behavior; when stable — acts decisively. Academically: "system robustness"
- **N-gram patterns:** deterministic frequency analysis, explainable at defense with a pattern table. Defense phrase: "The system recognizes the player's dominant tactic via N-gram frequency analysis and adapts utility modifiers accordingly"
- **bAdaptiveEnabled Flag:** Allows disabling the decision-making pipeline while keeping the FSM active. Critical for A/B testing and performance benchmarking during the demonstration. This component can be toggled manually in the Editor to compare "vanilla" behavior vs "adaptive" behavior.

---

### Combat Range Architecture (Hotfix — split per mode)
The legacy single `AttackRange` field has been **removed**. Each `ABaseEnemy` now exposes:
- `MeleeAttackRange` — Attack-entry distance for melee mode (default 80)
- `RangedAttackRange` — optimal shooting distance for ranged mode (default 320 / 450 for `ARangedEnemy`)
- `RangedTooFarRange` — beyond this, ranged Attack exits to Chase to close the gap (default 480)
- `MinAttackRange` — too close → Retreat (ranged only)
- `SafeDistance` — Retreat exit distance

**`virtual float GetEffectiveAttackRange() const`** — returns the right Attack-entry range based on mode. Overridden in `AAdaptiveEnemy` (uses `bIsRangedMode`) and `ARangedEnemy` (always ranged).

**Auto-Ranged toggle (in `UAdaptiveBehaviorComponent`)** uses **hysteresis** — separate thresholds for switching up to ranged (`SwitchUp ≈ MeleeR*1.5`) and down to melee (`SwitchDown ≈ MeleeR*1.2`), biased by `CombatStyle` (MeleeOriented / Balanced / RangedOriented). Forced ranged in Flank/Retreat; forced melee when committed inside `MeleeAttackRange`.

**Why this matters**: previously ranged Attack used `ExitRange = DetectionRange*0.9` → enemies stuck strafing forever and never closed back to melee. Now `ExitRange = RangedTooFarRange` so enemies can transition `RangedAttack → Chase → MeleeAttack` cleanly when CombatStyle/distance favours melee.

### Adaptive Behavior Tactics (Stage 12 Refinements)
- **Ranged Pressure Response**: Enemies now increase `Chase` utility when the player is shooting aggressively (detected via `MemoryAggressiveness` > 0.4). This creates a "push" mechanic to counter kiting.
- **Flanking**: Replaced 90° wide arcs with a "Shallow Angled Approach" (approx. 30-45°). Enemies move towards the player at an angle rather than orbiting them widely.
- **Hybrid Combat**: `AAdaptiveEnemy` supports shooting while in `Retreat` state if `bIsRangedMode` is active. Fire rate is reduced (1.5x cooldown) while retreating to simulate difficult aiming.
- **Projectile Spawn Parity**: `AAdaptiveEnemy` ranged fire path now mirrors `ARangedEnemy` (`PerformMeleeAttack` + `ActuallyFire`, deferred spawn, self-ignore on collision sphere, `MuzzleOffset=40`) to avoid frame hitches seen only on adaptive ranged shots.
- **Crowd Spacing**: `GetSeparationSteering()` implemented in `BaseEnemy` to prevent clumping. Used in `Chase` and `Attack` states.
- **Lifecycle**: `AdaptiveBehaviorComponent` MUST stop evaluation on owner death by clearing `EvaluationTimerHandle`.
**Key phrase for defense:**
> "The game environment is a demonstration polygon. The engineering result is a software module of adaptive behavior implementing a 3-layer threat assessment model with dynamic weight correction, time-decay memory, N-gram pattern recognition, and utility-based state selection."

---

## Common Architecture Mistakes

| Mistake | Why It Weakens Defense | How to Avoid |
|---|---|---|
| Behavior logic inside Enemy Actor | Violates SRP, module "dissolves" into game | Enemy Actor only calls FSM; all logic in FSM and Adaptive modules |
| FSM in Blueprint | Doesn't show C++ skills, committee counts it as visual scripting | 100% C++ for FSM and Adaptive |
| Behavior Tree | Built-in UE mechanism, no custom implementation | Forbidden in ToR |
| Static weights without feedback | System is reactive, not adaptive | Mandatory: DynamicWeightManager |
| No module visualization | Committee can't see how module makes decisions | Debug widget is mandatory |
| One enemy type | Doesn't show FSM reusability | 3 types: melee, ranged, adaptive |
| No input normalization | Formula is mathematically incorrect | Normalize all inputs to [0,1] |
| No logging | Impossible to debug and demonstrate | Detailed UE_LOG for every stage |
| Procedural generation before FSM | Puts gameplay above diploma module | Order: FSM → Adaptive → Rooms |
| No Data Asset for config | Hardcoded constants — unprofessional | `UAdaptiveConfig` as `UDataAsset` |
| Too frequent Tick in adaptive module | Wastes CPU unnecessarily | Timer 0.2–0.5s |
| Giant monolithic class | Violates SRP | Split into UObject subsystems |

---

## Glossary

> Этот раздел — на русском. Термины, аббревиатуры и нетривиальные концепции проекта.



### Glossary / Глоссарий
| Термин | Значение |
|---|---|
| AdaptiveBehavior | Ключевой модуль диплома — 3-слойная система оценки угрозы и принятия решений |
| FSM | Finite State Machine — кастомный C++, 5 состояний: Idle / Chase / Attack / Retreat / Flank |
| T_final | Финальное значение угрозы ∈ [0,1] после нормализации, кросс-термов, confidence и сглаживания |
| Utility curve | Функция-колокол или sigmoid, отображающая T → предпочтение к конкретному FSM-состоянию |
| Time-decay memory | `M_i(t) = Intensity · exp(-λ·Δt)` — события затухают экспоненциально |
| Shallow Angled Approach | Подход под острым углом (вместо широкого обхода по дуге) |
| Separation Steering | Рулевое поведение разделения (предотвращает наслоение врагов друг на друга) |
| Hybrid Retreat | Гибридное отступление (стрельба во время движения назад) |
| Confidence Coefficient | Коэффициент уверенности, снижающий агрессивность при высокой дисперсии входных данных |
| N-gram pattern | Частотный анализ последовательностей действий игрока (окно=15, 2-граммы + 3-граммы) |
| Inertia | Стоимость выхода из текущего состояния: `min(InertiaMax, rate · TimeInState)` |
| Confidence | `C = 1/(1+σ²)` — обратная дисперсия T_raw, управляет консерватизмом врага |
| UAdaptiveConfig | UDataAsset — все настраиваемые параметры, редактируемые в Editor без перекомпиляции |
| Pattern Counter-Tactics Matrix | Матрица тактических ответов на распознанные паттерны (например, Flank против Melee Spam) |
| Adaptive Thresholds | Адаптивные пороги принятия решений, вычисляемые через скользящее среднее и отклонение T_final |
| Debug widget | `UDebugAdaptiveWidget` — оверлей в реальном времени: T_final, веса, скоры состояний |
| bAdaptiveEnabled | Флаг в `UAdaptiveBehaviorComponent`, позволяющий вручную включать/выключать систему адаптации для сравнения с базовым поведением. |
| DynamicWeightManager | Корректирует веса w1..w6 через feedback loop: `w_i += η·reward·contribution_i` |
| Contribution_i | Относительный вклад фактора i: `f_i(x_i) / Σ f_j(x_j)` |
| RunItem | Предмет на один забег; `URunItemConfig` (DataAsset) — эффект + параметры; `URunItemInventory` хранит активные предметы |
| Hub | Лобби между забегами (`L_Hub`): выбор предметов + метапрогрессия |
| EEnemySpawnState | Состояние точки спавна в комнате (Free/Occupied) — используется для предотвращения наслоения врагов при генерации. |
| UAdaptiveConfig | UDataAsset — все настраиваемые параметры, редактируемые в Editor без перекомпиляции |
| Pattern Counter-Tactics Matrix | Матрица тактических ответов на распознанные паттерны (например, Flank против Melee Spam) |
| Adaptive Thresholds | Адаптивные пороги принятия решений, вычисляемые через скользящее среднее и отклонение T_final |
| Debug widget | `UDebugAdaptiveWidget` — оверлей в реальном времени: T_final, веса, скоры состояний |
| bAdaptiveEnabled | Флаг в `UAdaptiveBehaviorComponent`, позволяющий вручную включать/выключать систему адаптации для сравнения с базовым поведением. |
| DynamicWeightManager | Корректирует веса w1..w6 через feedback loop: `w_i += η·reward·contribution_i` |
| Contribution_i | Относительный вклад фактора i: `f_i(x_i) / Σ f_j(x_j)` |
| RunItem | Предмет на один забег; `URunItemConfig` (DataAsset) — эффект + параметры; `URunItemInventory` хранит активные предметы |
| Hub | Лобби между забегами (`L_Hub`): выбор предметов + метапрогрессия |
| Слот оружия | 2 слота у персонажа: 1=меч (key 1), 2=лук (key 2); переключение через `SwitchToWeaponSlot()` |
| [AI] | Задача выполняется AI-разработчиком (код, конфиги, создание папок) |
| [HUMAN] | Задача выполняется вручную в Unreal Editor |
| Полигон | Игровая среда как демонстрационная площадка для инженерного модуля (термин для защиты) | PerformMeleeAttack | Virtual метод |BaseEnemy, вызываемый FSMState_Attack. У ближников — урон в радиусе. У ARangedEnemy — спавн снаряда в направлении игрока |
| EnemyRanged.h/cpp | ДУБЛИКАТ — удалить вручную. Правильный класс: ARangedEnemy в RangedEnemy.h |
| MOVE_Flying | Режим движения персонажей в UE5 для 2D без гравитации — не вызывает переход в Falling при потере пола |
| AddActorWorldOffset(bSweep) | Ручное движение снаряда через Tick с детекцией перекрытий вдоль пути (замена PMC) |
| Room Cell | Единица измерения комнаты. В текущей версии: 8x6 тайлов (16x16px каждый). |
| Exit Hatch | Объект-выход, появляющийся после зачистки последней комнаты уровня. |
| Shadowed Floor | Специальный тайл пола с тенями, который всегда кладется под дверные проемы. |

---

## Console Commands (все Exec-команды, ~ в PIE)

### Economy
| Команда | Описание |
|---|---|
| `AddRunDiamonds <N>` | +N алмазов в текущий забег |
| `AddProfileDiamonds <N>` | +N алмазов в профиль (persistent) |
| `BuyUpgradeCmd <Type>` | Купить апгрейд: Damage / Range / ArrowCount / MaxHP |
| `ShowProfile` | Показать профиль (TotalDiamonds + уровни апгрейдов) |
| `ResetProfileCmd` | Сбросить профиль к дефолту |

### Items
| Команда | Описание |
|---|---|
| `ListItems` | Список всех предметов в DA_RunItemCollection |
| `GiveItem <name>` | Выдать предмет по частичному имени (пример: `GiveItem Bonus HP`) |
| `ClearRunItems` | Очистить инвентарь предметов, восстановить базовые статы |

### Database
| Команда | Описание |
|---|---|
| `DBCheck` | Вывести player_profile и все строки run_history в Output Log |

### Debug
| Команда | Описание |
|---|---|
| `ToggleGodMode` | Неуязвимость вкл/выкл |
| `ToggleSuperAttack` | Супер-урон вкл/выкл |

---

## SQLite Database Schema

**Файл:** `Saved/Depthrun.db`  
**Инструмент просмотра:** VSCode extension `alexcvzz.vscode-sqlite`

### `player_profile` (1 строка, id=1)
| Поле | Тип | Описание |
|---|---|---|
| `id` | INTEGER PRIMARY KEY | Всегда 1 |
| `TotalDiamonds` | INTEGER | Накопленные алмазы (метапрогрессия) |
| `Damage_Lvl` | INTEGER | Уровень апгрейда урона (0–5) |
| `Range_Lvl` | INTEGER | Уровень апгрейда дальности (0–5) |
| `ArrowCount_Lvl` | INTEGER | Уровень апгрейда стрел (0–3) |
| `MaxHP_Lvl` | INTEGER | Уровень апгрейда здоровья (0–5) |

### `run_history` (по одной строке на забег)
| Поле | Тип | Описание |
|---|---|---|
| `id` | INTEGER PRIMARY KEY | Автоинкремент (rowid) |
| `Rooms` | INTEGER | Количество зачищенных комнат |
| `Won` | INTEGER | 1=победа, 0=поражение |
| `RunDuration` | REAL | Длительность забега в секундах |
| `Timestamp` | TEXT | Время записи `datetime('now')` |

> **Примечание:** `sqlite_sequence` — системная таблица SQLite. С удалением `AUTOINCREMENT` она больше не создаётся в новых БД. Удали старый файл `Saved/Depthrun.db` чтобы схема пересоздалась чисто.

---

### Stage 8: Procedural Generation Rules
- **Grid Size**: 8x6 tiles per cell. Total width ~332 units (at 2.6 scale).
- **Spawn Offset Behavior**: `SpawnOffset` is now applied literally from the DataAsset (no hidden fallback). If `SpawnOffset = (0,0,0)`, runtime keeps it as zero.
- **Z Configuration (Single Source of Truth):** `RoomTemplate` has three Z fields — all applied directly from DataAsset, **no clamping**:
  - `PlayerLockedZ` — player spawn Z AND plane-constraint origin. Default `1.0`.
  - `EnemyLockedZ` — enemy spawn Z AND plane-constraint origin (set via `ABaseEnemy::SetLockedZ()`). Default `1.0`.
  - `DoorLockedZ` — door actor spawn Z. Default `1.0`.
  - `PlayerSpawnZ` field removed (was redundant, `PlayerLockedZ` covers both spawn and lock).
- **Enemy Z Fix (Apr 2026):** Root cause of enemies spawning under the floor was `bSnapToPlaneAtStart=true` in `ABaseEnemy` constructor — UE teleported enemies to Z=0 immediately after spawn. Fixed: `bSnapToPlaneAtStart=false`, and `RoomBase::SpawnEnemies()` calls `SetLockedZ(EnemyLockedZ)` after each spawn.
- **Start Room Doors:** Door actors are NOT spawned in Start rooms (`RoomType == Start`). Previously they were spawned open, but their BoxComponent Z-extent (40 units) still blocked players on low Z values. Removing them entirely ensures clean passage.
- **Door Collision Z-extent:** Reduced from 40 → 8 units. This prevents doors from blocking actors on adjacent Z planes and keeps collision tight to the 2D gameplay plane.
- **Character Collision Fix**: Reduced player capsule `HalfHeight` from 16.f to 4.f to prevent `PenetrationDepth` issues with landscape collision. Original problem: capsule was too tall (32 units) and penetrated 12 units into landscape, causing movement failures.
- **Door Openings**: For connected sides, code explicitly clears wall tiles in doorway cells and optionally applies `DoorFloorShadow*` on floor layer.
- **Start Room Door Logic**: Start room (index 0) no longer closes doors on activation. Doors remain open to allow player exit. Only combat rooms (index > 0) close doors when activated.
- **Tile Layers**:
  - `Layer 0`: Floor (Обычный пол + Shadowed Floor в проемах).
  - `Layer 1`: Door_Layer (Специфический слой для дверей/проходов).
  - `Layer 2`: Walls (Стены. Стены застраивают заблокированные проходы).
- **Prop Placement (Code-driven)**:
  - **Torches**: Only on top walls (`Layer 2`, `Y=0`), not in corners (`X=0,7`), not in doors (`X=3,4`). Random chance (50%).
  - **Bones**: Randomly scattered on floor (`Layer 0`), no collision.
  - **Chest**: 10% spawn chance in room center after `DeactivateRoom()` is called.
- **Spawn Offset Baseline (Validated Apr 2026):** Player spawn now centers inside the room first (`+RoomSizeX*0.5`, `+RoomSizeY*0.5`). The DataAsset `SpawnOffset` is applied **on top** of that baseline. Prevents spawning inside walls when `SpawnOffset = (0,0,0)`.
- **Tile Clear Safety:** `SetTileInLayer` uses `FPaperTileInfo()` default constructor for empty tiles to prevent flickering / ghost tiles.
- **Collision Rebuild:** `TileMapComponent->RebuildCollision()` is called after all doorway wall removals so physics matches visuals.
- **Collision Policy Flags (RoomTemplate):**
  - `bUseProceduralWallColliders` (default `false`) — when disabled, no extra red wall boxes are spawned; wall blocking comes from tilemap collision.
  - `bUseDoorSpriteCollision` (default `true`) — doors block via sprite collision geometry when closed and fully disable collision when open.
  - `bUseObstacleSpriteCollision` (default `true`) — floor obstacles use each sprite variant's own collision geometry (supports mixed obstacle silhouettes without a fixed box).
- **Door Orientation:** Vertical doors (Left/Right) receive an extra `+90° Yaw` on top of `DoorRotation` so they align correctly with the doorway axis.
- **Enemy Spawn Safety:** `SpawnEnemies()` checks `OccupiedTiles` (doorway cells) and skips them to prevent enemies spawning in doorways or outside the room.
- **Player First-Frame Stability:** `DepthrunGameMode` hides the player before procedural generation starts; `RoomGeneratorSubsystem` teleports to start-room position and unhides/re-enables collision afterward. This removes visible "spawn at PlayerStart then jump" artifacts.
- **Player Animation Fallback:** `ADepthrunCharacter::UpdateAnimation()` now falls back to walk flipbooks when idle flipbooks are missing, preventing "invisible until first movement" behavior.
- **Encounter Logic:**
  - `OnRoomEntry` -> Lock Doors -> Spawn Enemies (combat rooms only).
  - `AllEnemiesDead` -> Unlock Doors -> Try Spawn Chest.
- **Diamonds** — постоянная валюта, сохраняется через SQLite `player_profile`. При смерти сохраняется 50%, при выходе в Hub — 100%
- **HealthPotion** — расходник на забег, сбрасывается при смерти. +50 HP, cooldown 0.5s, клавиша E
- **HubUpgrade** — 4 апгрейда (Damage / Range / ArrowCount / MaxHP). ArrowCount max 3 levels (остальные max 5). ArrowCount цена `floor(150 × 2^level)` (150/300/600), остальные `floor(50 × 1.6^level)`
- **MusicSubsystem** — `UGameInstanceSubsystem`, 2× `UAudioComponent` + crossfade
- **ChestLootTable** — `UDataAsset` с весами, `RollLoot()` → `TArray<FChestRewardLine>`
- **CombatMusicTrigger** — компонент на игроке, дебаунс 1.5s, активируется через `RoomBase::IsCombatActive()`
