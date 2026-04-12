# Depthrun

> **Дипломный проект** — 2D top-down roguelike shooter на Unreal Engine 5 (Paper2D).  
> Основной инженерный результат: программный модуль **адаптивного поведения врагов** на базе FSM и многоуровневой системы оценки угрозы.

---

## Описание

Depthrun — демонстрационный полигон для дипломной работы. Игровая механика служит средой для доказательства работоспособности модуля `AdaptiveBehavior` — 3-слойной системы принятия решений, реализованной на чистом C++ без встроенных AI-систем Unreal Engine.

### Ключевые особенности

- **FSM** — кастомная конечная автоматная машина, 5 состояний: Idle / Chase / Attack / Retreat / Flank
- **AdaptiveBehavior** — 3-слойный оверлей над FSM:
  - Слой 1: нормализация и нелинейные преобразования контекста
  - Слой 2: вычисление угрозы T_final с cross-terms, confidence и сглаживанием
  - Слой 3: выбор состояния через utility-функции + матрица стоимостей переходов
- **Динамические веса** — адаптация через feedback loop (без ML)
- **Time-decay память** — события затухают по `M(t) = I · exp(-λ·Δt)`
- **N-gram паттерны** — распознавание тактики игрока (окно=15, 2+3-граммы)
- **Debug-виджет** — реалтаймовый оверлей T_final, весов, скоров состояний

---

## Стек

| Компонент | Технология |
|---|---|
| Engine | Unreal Engine 5.7 |
| Рендер спрайтов | Paper2D (built-in plugin) |
| Ввод | Enhanced Input |
| Эффекты | Niagara |
| Сохранение | SQLiteCore (built-in UE5) |
| UI | UMG (layout) + C++ (логика) |
| Язык | 100% C++ для FSM и AdaptiveBehavior |

---

## Архитектура модулей

```
Source/Depthrun/
├── Core/          — GameMode, GameInstance, LogChannels
├── Player/        — Character, CombatComponent, ActionTracker
├── Enemy/         — BaseEnemy, MeleeEnemy, RangedEnemy, AdaptiveEnemy
├── FSM/           — FSMComponent, FSMState (5 states)
├── AdaptiveBehavior/  — ★ CORE MODULE (60% диплома)
│   ├── ContextEvaluator      — Layer 1
│   ├── ThreatCalculator      — Layer 2
│   ├── StateTransitionResolver — Layer 3
│   ├── AdaptiveMemory        — time-decay
│   ├── PatternRecognizer     — N-grams
│   ├── DynamicWeightManager  — feedback weights
│   ├── UtilityCurves         — utility functions
│   └── TransitionCostMatrix  — 5×5 cost + inertia
├── Combat/        — BaseWeapon, Ranged, Melee, Projectile
├── RoomGeneration/ — RoomBase, RoomGeneratorSubsystem
├── UI/            — HUD, HealthBar, DebugAdaptiveWidget
├── Data/          — SaveSubsystem, SQLiteManager
└── Utils/         — MathUtils (sigmoid, bell, decay)
```

---

## Сборка

1. Установить UE 5.7+ через Epic Games Launcher
2. Открыть `Depthrun.uproject` в Unreal Editor
3. `File → Generate Visual Studio Project Files`
4. Открыть `Depthrun.sln` в Visual Studio 2022
5. Установить конфигурацию `Development Editor | Win64`
6. Build → Run

---

## Ключевая фраза для защиты

> «Игровая среда является демонстрационным полигоном. Инженерный результат — программный модуль адаптивного поведения, реализующий 3-слойную модель оценки угрозы с динамической коррекцией весов, памятью с временным затуханием, N-gram распознаванием паттернов и utility-based выбором состояния.»

---

## Git-статистика

Target: **30–50+ коммитов** с осмысленными сообщениями для подтверждения авторства.  
Формат: `<type>(<scope>): <description>`

---

*Depthrun © 2026 — Diploma project, UE5 C++*
