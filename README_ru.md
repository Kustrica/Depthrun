# Depthrun

> **Дипломный проект** — 2D top-down roguelike-шутер на **Unreal Engine 5.7** и **Paper2D**.
> Инженерный результат — собственный программный модуль **адаптивного поведения врагов**: трёхслойный конвейер принятия решений на чистом C++ без штатных систем AI Unreal Engine (без Behavior Tree, Blackboard, EQS и машинного обучения).

![Engine](https://img.shields.io/badge/Engine-Unreal%20Engine%205.7-313131?logo=unrealengine)
![Language](https://img.shields.io/badge/Language-C%2B%2B%20100%25-00599C?logo=cplusplus)
![Genre](https://img.shields.io/badge/Жанр-2D%20Top--Down%20Roguelike-purple)
![Status](https://img.shields.io/badge/Статус-Предзащитная%20сборка-success)

---

## Содержание

1. [О проекте](#о-проекте)
2. [Ключевые особенности](#ключевые-особенности)
3. [Модуль адаптивного поведения](#модуль-адаптивного-поведения)
4. [Стек](#стек)
5. [Структура исходников](#структура-исходников)
6. [Игровой цикл](#игровой-цикл)
7. [Процедурная генерация комнат](#процедурная-генерация-комнат)
8. [Предметы и улучшения](#предметы-и-улучшения)
9. [Сохранения (SQLite)](#сохранения-sqlite)
10. [Сборка из исходников](#сборка-из-исходников)
11. [Сборка пакета (exe)](#сборка-пакета-exe)
12. [Консольные команды](#консольные-команды)
13. [Инструменты отладки](#инструменты-отладки)
14. [Ограничения проекта](#ограничения-проекта)
15. [Фраза для защиты](#фраза-для-защиты)
16. [Авторство](#авторство)

---

## О проекте

Depthrun — это **демонстрационный полигон** для дипломного исследования. Игра полностью играбельна, но её основная задача — продемонстрировать рабочий программный модуль адаптивного поведения: трёхслойный конвейер, который превращает скалярный контекст (расстояние, HP, тип оружия, плотность, память) в высокоуровневое решение FSM, попутно дообучая собственные веса по обратной связи из боя.

Сдаётся комиссии **не игра**, а модуль `AdaptiveBehavior`. Игра существует только как среда, в которой модулю есть куда жить и на что реагировать.

**Платформа:** Windows x64.
**Целевая аудитория:** дипломная комиссия + все, кому интересны патерны адаптивного AI без ML, пригодные для прода в UE5.

---

## Ключевые особенности

### Геймплей
- **Процедурный данж** из многосекционных комнат (1×1, 2×1, 1×2, 2×2, L-форма), сетка 8×6 тайлов на ячейку.
- **Два слота оружия** — меч (слот 1) и лук (слот 2), мгновенное переключение клавишами `1` / `2`.
- **Система предметов** на один забег: 7 модификаторов (Рикошет, Пробивание, Расширенный радиус, Двойной взмах, Плоский урон, Бонус HP, Скорость, Multishot). Применяются при подборе, сбрасываются в конце забега.
- **Награда из сундука** — каждый сундук всегда даёт Diamonds + Health Potions + предмет с шансом 70 %. Настраивается через `DA_ChestLootConfig`.
- **Метапрогрессия в хабе** — 4 апгрейда (Урон, Дальность, Кол-во стрел, MaxHP), покупаются за алмазы, накопленные между забегами.
- **Босс-комната** завершает забег экраном победы и выдаёт 100 % алмазов; смерть выдаёт 50 %.
- **Niagara VFX** — попадание (ближний/дальний), смерть, открытие сундука, экраны победы и поражения.
- **Музыка** — `UMusicSubsystem` на двух `UAudioComponent` с crossfade, боевой триггер реагирует на активную комнату.

### Инженерная часть
- **Чистый C++ FSM** с 5 состояниями: `Idle / Chase / Attack / Retreat / Flank` — без Blueprint и Behavior Tree.
- **Трёхслойный адаптивный оверлей** над FSM принимает решение о смене состояния с учётом динамических весов, памяти с затуханием, N-граммного распознавания паттернов и utility-кривых.
- **A/B-тест** через флаг `bAdaptiveEnabled` в компоненте — отключение оверлея откатывает врага к ванильному FSM, удобно для эмпирического сравнения на защите.
- **Реалтаймовый отладочный виджет** показывает `T_final`, все веса `w_i`, скоры всех состояний, распознанный N-gram и текущее состояние автомата.
- **Экспорт CSV** через `UDemonstrationSubsystem` — для графиков и таблиц в дипломе.

---

## Модуль адаптивного поведения

Сердце проекта. Лежит в `Source/Depthrun/AdaptiveBehavior/`, навешивается на каждого врага как `UAdaptiveBehaviorComponent`. Оценка крутится на `FTimerManager` раз в **0,3 с** (настраивается в диапазоне 0,2–0,5 с) — **никакого** ежекадрового Tick.

### Слой 1 — сбор контекста (`ContextEvaluator`)
Считывает сырое состояние мира для конкретного врага и нормализует всё в диапазон `[0, 1]`:

| Фактор | Источник | Нормализация |
|---|---|---|
| `DistanceNorm` | расстояние до игрока | линейная |
| `WeaponThreatNorm` | активное оружие игрока (ближний 0.6 / дальний 0.8) | дискретный lookup |
| `EnemyHPRatioNorm` | HP / MaxHP врага | квадратичная (`α=2`) |
| `AllyCountNorm` | союзники в радиусе поддержки | clamp(count / 5) |
| `RoomDensityNorm` | сущности в радиусе 600 единиц | clamp(count / 8) |
| `MemoryAggressiveness` + `MemoryMobility` | из `AdaptiveMemory` | оконный счётчик |

### Слой 2 — расчёт угрозы (`ThreatCalculator`)
Считает скаляр угрозы для слоя 3:

```
T_final = clamp( Σ w_i × f_i , 0, 1 )
```

Без cross-terms, без экспоненциального сглаживания, без confidence-fallback — нарочно простая и объяснимая формула под защиту. Веса `w_1…w_6` приходят из **`DynamicWeightManager`**, который корректирует их детерминированной обратной связью:

```
w_i += η · reward · contribution_i
contribution_i = f_i(x_i) / Σ f_j(x_j)
```

`reward = +1.0` при успешном уроне игроку, `−1.0` при получении урона врагом. **Это не машинное обучение** — это прозрачное правило, которое можно вручную трассировать.

### Слой 3 — выбор состояния (`StateTransitionResolver`)
Для каждого из 5 состояний автомата вычисляется:

```
FinalScore(s) = Utility(s, T_final, Context)  −  TransitionCost(current → s)
                                              +  InertiaBonus(current, s, TimeInState)
                                              +  PatternModifier(s)
```

- **Utility** — колокол / рамп-кривая на состояние (`UtilityCurves`), пик там, где это состояние имеет смысл.
- **TransitionCost** — матрица 5×5 (`TransitionCostMatrix`), штрафует прыжки между «далёкими» состояниями.
- **InertiaBonus** — `min(InertiaMax, rate × TimeInState)`, поощряет оставаться в текущем состоянии и душит дребезг.
- **PatternModifier** — бонус/штраф по распознанному N-gram (например, *Shot+Shot* добавляет Flank `+0.65`).

Победитель — состояние с максимальным `FinalScore`. Если оно отличается от текущего, `FSMComponent::TransitionTo` срабатывает.

### Поддерживающие подсистемы
- **`AdaptiveMemory`** — короткая память о действиях игрока. Aggressiveness = `count(Shot+Melee за 10 с) / 10`.
- **`PatternRecognizer`** — скользящее окно из 15 действий, частотный счёт 2-грамм и 3-грамм, возвращает доминирующий паттерн.
- **`UAdaptiveConfig` (UDataAsset)** — все настраиваемые константы (η, λ, параметры кривых, размеры окон, точки гистерезиса) лежат тут и редактируются в Editor без перекомпиляции.

### Архитектура боевых дистанций
Каждый `ABaseEnemy` хранит разделённые радиусы:
- `MeleeAttackRange` — вход в ближнюю атаку (по умолчанию 80)
- `RangedAttackRange` — оптимум для дальнего боя (320, у `ARangedEnemy` — 450)
- `RangedTooFarRange` — выход из дальней атаки обратно в Chase (480)
- `MinAttackRange` — слишком близко → Retreat
- `SafeDistance` — выход из Retreat

`GetEffectiveAttackRange()` — virtual; `AAdaptiveEnemy` переопределяет его исходя из `bIsRangedMode`. Переключение melee↔ranged идёт через **гистерезис** — отдельные точки SwitchUp/SwitchDown, смещённые по характеру `CombatStyle`, чтобы исключить дребезг режима.

---

## Стек

| Слой | Технология |
|---|---|
| Движок | Unreal Engine **5.7** |
| Рендер | Paper2D (2D top-down, гравитация = 0, движение только по XY) |
| Ввод | Enhanced Input |
| Эффекты | Niagara |
| Сохранения | SQLiteCore (встроенный плагин UE 5) |
| UI | UMG (вёрстка) + C++ (логика, никаких Blueprint в виджетах) |
| Звук | 2× `UAudioComponent` + crossfade через `UMusicSubsystem` |
| Язык | **100 % C++** для FSM, AdaptiveBehavior, геймплея и UI-логики |

---

## Структура исходников

```
Source/Depthrun/
├── Core/             — GameMode, GameInstance, log channels
├── Player/           — DepthrunCharacter, CombatComponent, PlayerEconomy, ActionTracker
├── Enemy/            — BaseEnemy, MeleeEnemy, RangedEnemy, AdaptiveEnemy
├── FSM/              — FSMComponent + 5 классов FSMState_*
├── AdaptiveBehavior/ — ★ КЛЮЧЕВОЙ МОДУЛЬ
│   ├── AdaptiveBehaviorComponent  — оркестратор (timer, конвейер слоёв)
│   ├── ContextEvaluator           — Слой 1: нормализация факторов
│   ├── ThreatCalculator           — Слой 2: взвешенная сумма → T_final
│   ├── StateTransitionResolver    — Слой 3: argmax FinalScore
│   ├── AdaptiveMemory             — короткая память о действиях игрока
│   ├── PatternRecognizer          — 2/3-граммный частотный анализ
│   ├── DynamicWeightManager       — коррекция весов по обратной связи
│   ├── UtilityCurves              — utility-функции на состояние
│   ├── TransitionCostMatrix       — матрица 5×5 + инерция
│   └── AdaptiveConfig (DataAsset) — все настройки
├── Combat/           — BaseWeapon, MeleeWeapon, RangedWeapon, Projectile, ChestActor
├── Items/            — RunItemInventory, RunItemConfig, RunItemCollection
├── RoomGeneration/   — RoomBase, RoomTemplate, RoomGeneratorSubsystem, TrapdoorActor
├── UI/               — HUDOverlayWidget, HealthBar, PauseMenu, DeathScreen, VictoryScreen,
│                       DebugAdaptiveWidget, ChestRewardWidget, HubWidget, MainMenuWidget
├── Data/             — DepthrunSaveSubsystem, схема SQLite
├── Audio/            — MusicSubsystem, CombatMusicTrigger, UISoundLibrary
└── Utils/            — математика (sigmoid, bell, decay)
```

---

## Игровой цикл

```
L_MainMenu ──▶ L_Hub ──▶ L_Gameplay ──▶ (смерть ИЛИ зачистка босса) ──▶ L_Hub
                ▲                                                              │
                └──────────────────────────────────────────────────────────────┘
```

1. **Главное меню** — Play / Settings / Quit. Настройки (мастер-громкость) сохраняются в `UDepthrunGameInstance`.
2. **Хаб** — профиль, покупка 4 апгрейдов за алмазы, запуск забега.
3. **Забег** — процедурный данж. Каждая боевая комната закрывает двери → спавнит врагов → после зачистки открывает двери и с шансом 10 % спавнит сундук.
4. **Босс-комната** — после зачистки спавнится люк `ATrapdoorActor`; вход в люк = победа и 100 % алмазов.
5. **Смерть** — экран Game Over, фейд 3 с, 50 % алмазов идут в профиль, возврат в хаб.
6. Каждый забег вписывает строку в `run_history` независимо от исхода.

---

## Процедурная генерация комнат

- Сетка 8×6 тайлов на ячейку (16×16 px, при scale 2.6 ≈ 332 единицы ширины).
- Многоклеточные комнаты: 1×1, 2×1, 1×2, L-форма (3 ячейки), 2×2.
- Z-координата — **single source of truth** в `RoomTemplate`: `PlayerLockedZ`, `EnemyLockedZ`, `DoorLockedZ`. Никаких скрытых clamp.
- Слои тайлмапы:
  - Layer 0 — Пол (обычный + Shadowed Floor в проёмах)
  - Layer 1 — Слой дверей
  - Layer 2 — Стены (автоматически зашивают закрытые проходы)
- Декор — кодом: факелы на верхних стенах (не в углах, не в проёмах, шанс 50 %), кости на полу, сундук с шансом 10 %.
- **Стартовая комната** не спавнит двери и не закрывается — чистый проход.
- **Безопасный спавн врагов:** `OccupiedTiles` (ячейки дверных проёмов) пропускаются, чтобы враги не вставали в дверях.

---

## Предметы и улучшения

### Предметы на забег (теряются при смерти)
| Эффект | Оружие | Что делает | Параметр |
|---|---|---|---|
| `ArrowRicochet` | Лук | Рикошет стрел | `RicochetCount = 2` |
| `ArrowPierce` | Лук | Пробивание врагов | — |
| `MeleeExtendedRange` | Меч | Увеличенный радиус удара | `×1.5` |
| `MeleeDoubleSwing` | Меч | Двойной взмах | — |
| `FlatDamage` | Любое | +5 плоского урона | — |
| `BonusMaxHP` | Любое | +25 к MaxHP | `25` |
| `BonusMoveSpeed` | Любое | +15 % к скорости | `0.15` |
| `Multishot` (`BonusProjectileCount`) | Дальний бой | +1 стрела за выстрел (max 5) | `1` |

Все предметы предзаполнены в одном `DA_RunItemCollection`. Эффекты применяются через `ApplyToWeapon()` + `ApplyToCharacter()`.

### Апгрейды в хабе (постоянные)
| Апгрейд | Max уровень | Формула цены |
|---|---|---|
| Damage | 5 | `floor(50 × 1.6^level)` |
| Range | 5 | `floor(50 × 1.6^level)` |
| ArrowCount | 3 | `floor(150 × 2^level)` |
| MaxHP | 5 | `floor(50 × 1.6^level)` |

---

## Сохранения (SQLite)

**Файл:** `Saved/Depthrun.db`
**Подсистема:** `UDepthrunSaveSubsystem` (GameInstance scope)
**Просмотр:** VSCode-расширение `alexcvzz.vscode-sqlite` открывает БД из коробки.

### Схема

**`player_profile`** (одна строка, `id = 1`)
| Поле | Тип | Описание |
|---|---|---|
| `id` | INTEGER PK | всегда 1 |
| `TotalDiamonds` | INTEGER | накопленные алмазы |
| `Damage_Lvl` | INTEGER | 0–5 |
| `Range_Lvl` | INTEGER | 0–5 |
| `ArrowCount_Lvl` | INTEGER | 0–3 |
| `MaxHP_Lvl` | INTEGER | 0–5 |

**`run_history`** (строка на забег)
| Поле | Тип | Описание |
|---|---|---|
| `id` | INTEGER PK | rowid |
| `Rooms` | INTEGER | зачищенных комнат |
| `Won` | INTEGER | 1 = победа, 0 = смерть |
| `RunDuration` | REAL | секунды |
| `Timestamp` | TEXT | `datetime('now')` |

Результат пишется **один раз за забег**: `ShowVictoryScreen()` зовёт `SaveRunResult(..., true)` после начисления 100 % алмазов; `Die()` зовёт `SaveRunResult(..., false)` после начисления 50 %.

---

## Сборка из исходников

**Требования:**
- Windows 10 / 11 x64
- Visual Studio 2022 с компонентом *Game development with C++*
- Unreal Engine **5.7** через Epic Games Launcher

**Шаги:**
1. Склонировать репозиторий.
2. Правый клик по `Depthrun.uproject` → **Generate Visual Studio project files**.
3. Открыть `Depthrun.sln` в Visual Studio 2022.
4. Конфигурация: **`Development Editor | Win64`**.
   > ⚠ **Критично:** никогда не используй *DebugGame Editor* — он собирает другую `.dll`, которую редактор UE не подхватывает.
5. Build → запустить редактор → открыть `L_MainMenu` → Play.

---

## Сборка пакета (exe)

В редакторе: **File → Package Project → Windows**.

В пакет должны попасть все три уровня. В проекте уже прописано в `Config/DefaultGame.ini`:

```ini
[/Script/UnrealEd.ProjectPackagingSettings]
+MapsToCook=(FilePath="/Game/Levels/L_MainMenu")
+MapsToCook=(FilePath="/Game/Levels/L_Hub")
+MapsToCook=(FilePath="/Game/Levels/L_Gameplay")
```

Без этих записей кнопка Play в запакованном билде молча не работает — кукер кладёт только `GameDefaultMap`, и вызов `OpenLevel("L_Hub")` падает в никуда.

---

## Консольные команды

Консоль открывается клавишей `~` в PIE.

### Экономика
| Команда | Эффект |
|---|---|
| `AddRunDiamonds <N>` | +N алмазов в текущий забег |
| `AddProfileDiamonds <N>` | +N алмазов в профиль (persistent) |
| `BuyUpgradeCmd <Type>` | Купить апгрейд: `Damage` / `Range` / `ArrowCount` / `MaxHP` |
| `ShowProfile` | Показать профиль (TotalDiamonds + уровни) |
| `ResetProfileCmd` | Сбросить профиль |

### Предметы
| Команда | Эффект |
|---|---|
| `ListItems` | Список всех предметов в `DA_RunItemCollection` |
| `GiveItem <часть имени>` | Выдать предмет по частичному совпадению имени |
| `ClearRunItems` | Очистить инвентарь, восстановить базовые статы |

### База данных
| Команда | Эффект |
|---|---|
| `DBCheck` | Дамп `player_profile` и `run_history` в Output Log |

### Отладка
| Команда | Эффект |
|---|---|
| `ToggleGodMode` | Неуязвимость вкл/выкл |
| `ToggleSuperAttack` | Супер-урон вкл/выкл |

---

## Инструменты отладки

- **`UDebugAdaptiveWidget`** — оверлей над каждым врагом в реальном времени: `T_final`, все веса, `FinalScore` всех состояний, распознанный N-gram, текущее состояние FSM. Обязателен для защиты — комиссия должна видеть, как модуль думает.
- **`bAdaptiveEnabled`** — флаг в `UAdaptiveBehaviorComponent`. Снимай галку в редакторе или меняй в рантайме, чтобы откатить врага к ванильному FSM и сравнить поведение side-by-side.
- **`UDemonstrationSubsystem`** — пишет решения, веса и значения угрозы в CSV (в `Saved/`) для графиков диплома.
- **Категории логов** — `LogDepthrun`, `LogAdaptiveBehavior`, `LogDepthrunLoot`, `LogDepthrunSave`. Фильтруй Output Log по нужной подсистеме.

---

English version: [`README.md`](./README.md).
