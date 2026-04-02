

ЧАСТЬ 1: КРИТИЧЕСКИЙ АНАЛИЗ ТЕКУЩЕЙ АРХИТЕКТУРЫ
1.1 Достаточна ли сложность текущей адаптивной модели для дипломного уровня?

Вердикт: НЕТ. Текущая реализация — реактивная, а не адаптивная.

Текущая система: FSM (5 состояний) + 3-слойная надстройка (контекст → угроза → переход). Архитектурный скелет верный, но глубина реализации каждого слоя критически недостаточна.

Ключевая проблема: при фиксированных весах w1..w6 система не адаптируется — она лишь вычисляет условие по текущему состоянию мира. Это эквивалентно цепочке if/else с числовым порогом. На вопрос комиссии «в чём адаптация?» — ответа нет.

Для дипломного уровня «адаптивный» должно означать:

    Поведение меняется в процессе боя на основе опыта (динамические веса)
    Враг помнит действия игрока и эта память затухает (time decay)
    Враг распознаёт повторяющиеся паттерны игрока
    Переходы между состояниями имеют инерцию (враг не дёргается)
    Пороги «что считать опасным» сдвигаются в зависимости от обстановки

1.2 Достаточна ли формула угрозы?

Текущая формула:

text

T = w1·D + w2·W + w3·H + w4·A + w5·R + w6·M

Вердикт: СЛИШКОМ ПРОСТА. На защите это будет слабым местом.
Проблема	Суть	Последствие
Линейность	Все факторы суммируются линейно	Нет синергии: HP=5% + дистанция=вплотную должны давать экспоненциальный рост угрозы, а не арифметическую сумму
Статические веса	w1..w6 — константы	Нет адаптации. Система вычисляет, но не учится. Комиссия спросит: «где адаптация?»
Нет нормализации	D в пикселях (0–2000), H в процентах (0–1), A в штуках (0–5)	Один фактор (D) будет доминировать над всеми остальными
Нет взаимодействия факторов	Факторы не перемножаются	Комбинаторные ситуации не учитываются
Память (M) не формализована	«хранит shot/dash/attack» — но как это число?	Нет функции преобразования событий в числовой вклад
Нет временной размерности	Когда произошло событие — неважно	Выстрел 30 секунд назад и выстрел 0.5 секунды назад дают одинаковый вклад
1.3 Достаточна ли архитектура FSM + адаптивный слой?

Вердикт: Архитектурное разделение верное, но механика переходов слабая.

FSM + адаптивная надстройка — это правильный подход. Проблема не в архитектуре, а в отсутствии:

    Матрицы стоимости переходов — сейчас переход Flank → Idle стоит столько же, сколько Chase → Attack. В реальности некоторые переходы «дороже» (опаснее, требуют времени).
    Инерции состояний — враг не должен переключаться каждый тик. Должен быть гистерезис: чем дольше в состоянии, тем больше «стоимость» выхода.
    Utility-scoring для выбора состояния — вместо простого порога if T > 0.7 → Retreat нужна утилитарная функция для каждого состояния с разными кривыми отклика.
    Коэффициента уверенности — система должна знать, насколько она «уверена» в текущей оценке.

1.4 Есть ли лучшие альтернативы (без встроенных AI-систем)?

Полностью менять подход не нужно. FSM остаётся базой — это обязательное требование и правильное решение. Но адаптивный слой должен быть значительно усилен:
Механизм	Зачем	Реализуемость
Utility-based state scoring	Более гибкий выбор состояний чем пороги	Легко в C++, массив utility-функций
Динамическая корректировка весов	Делает систему действительно адаптивной	Простой feedback loop, не ML
Time-decay memory	Временное затухание событий	Экспоненциальное затухание, O(n)
Распознавание паттернов	Враг видит, что игрок повторяет dash→attack	Подсчёт N-грамм последовательностей
State inertia (гистерезис)	Предотвращает дёрганье	Простой счётчик времени в состоянии
Transition cost matrix	Реалистичная стоимость переходов	5×5 матрица
Confidence coefficient	Оценка надёжности текущих данных	Обратная дисперсия
Adaptive thresholds	Пороги сдвигаются по скользящему среднему	Простая статистика
1.5 Итоговые рекомендации перед созданием roadmap

Для дипломного уровня необходимо реализовать минимум 6 из 8 усилений:

    Нелинейные преобразования входных факторов (sigmoid, power)
    Нормализация всех входов к [0, 1]
    Динамическая корректировка весов через feedback loop
    Time-decay memory с экспоненциальным затуханием
    Utility-based state scoring (вместо пороговых if/else)
    State inertia + transition cost matrix
    Confidence coefficient
    Adaptive thresholds (скользящие пороги)

Все 8 — реализуемы в C++, без ML, полностью формульно, объяснимы на защите.
ЧАСТЬ 2: УСИЛЕННАЯ АРХИТЕКТУРА АДАПТИВНОГО МОДУЛЯ
2.1 Усиленная формула угрозы (Diploma-Grade)
Фаза 1: Нормализация входов → [0, 1]

text

D_norm = clamp(1.0 - (Distance / MaxEngagementRange), 0, 1)
W_norm = WeaponThreatLookup[PlayerWeaponType]            // 0.0–1.0
H_norm = 1.0 - (CurrentHP / MaxHP)                       // 0 = full HP, 1 = almost dead
A_norm = clamp(NearbyAllyCount / MaxAllyThreshold, 0, 1) // больше союзников = безопаснее
R_norm = clamp(EntitiesInRoom / MaxRoomCapacity, 0, 1)
M_norm = вычисляется подсистемой памяти (см. 2.2)

Фаза 2: Нелинейные преобразования

text

f_D(x) = 1 / (1 + e^(-k_D * (x - 0.5)))        // sigmoid, k_D ≈ 10
f_H(x) = x^α,  α = 2.0                           // квадратичная чувствительность при низком HP
f_W(x) = x                                         // линейно (дискретные значения)
f_A(x) = 1.0 - x^0.5                              // инверсия: больше союзников = меньше угроза
f_R(x) = x                                         // линейно
f_M(x) = вычисляется через time-decay (см. 2.2)

Зачем нелинейность: На защите вы показываете графики функций. Sigmoid для дистанции означает «на средней дистанции чувствительность максимальна, на краях — затухает». Power function для HP — «при полном здоровье фактор почти не влияет, при критическом — доминирует». Это академически сильная позиция.
Фаза 3: Динамическая корректировка весов

text

w_i(t+1) = clamp(w_i(t) + η · reward · contribution_i, w_min, w_max)

Где:

    η = 0.03–0.05 — скорость адаптации
    reward = +1.0 если враг нанёс урон игроку, -1.0 если получил урон
    contribution_i = f_i(x_i) / Σ f_j(x_j) — вклад фактора i в решение
    w_min = 0.05, w_max = 0.4 — ограничения, чтобы один фактор не убил остальные

Критично: это НЕ machine learning. Это простой feedback loop с ограниченной адаптацией. Полностью объяснимый, формульный, детерминированный при одинаковых входах. На защите: «система корректирует свои приоритеты на основе результатов боевого взаимодействия».
Фаза 4: Многокомпонентная угроза с взаимодействием факторов

text

T_base = Σ w_i(t) · f_i(x_i)

T_cross = w_DH · f_D(D) · f_H(H)  +  w_WM · W_norm · M_attack_decay

T_raw = T_base + β · T_cross       // β ∈ [0.1, 0.3]

Зачем кросс-термы: Низкое HP × близкий игрок = экстренная ситуация. Эти факторы должны усиливать друг друга мультипликативно, а не просто складываться.
Фаза 5: Коэффициент уверенности и сглаживание

text

σ² = variance(T_raw за последние N оценок),  N = 5–10
C = 1.0 / (1.0 + σ²)                          // высокая дисперсия = низкая уверенность

T_smooth = α · T_raw + (1-α) · T_prev         // экспоненциальное сглаживание, α = 0.3
T_final = C · T_smooth + (1-C) · T_default    // при неуверенности — откат к безопасному значению

На защите: «Когда данные нестабильны (игрок ведёт себя хаотично), враг переходит к консервативному поведению. Когда данные стабильны — действует решительно.»
Фаза 6: Utility-based выбор состояния

Для каждого состояния s ∈ {Idle, Chase, Attack, Retreat, Flank}:

text

U_Idle(T)    = max(0, 1.0 - 4·T²)              // высоко только при T ≈ 0
U_Chase(T)   = bell(T, center=0.3, width=0.2)  // при низко-средней угрозе
U_Attack(T)  = bell(T, center=0.5, width=0.25) // при средней угрозе
U_Flank(T)   = bell(T, center=0.6, width=0.2) · A_norm  // средне-высокая + есть союзники
U_Retreat(T) = sigmoid(T, center=0.75, k=12)   // при высокой угрозе

Где bell(x, c, w) = e^(-(x-c)²/(2·w²)) — колоколообразная функция.
Фаза 7: Матрица стоимости переходов + инерция

text

Score(s) = U(s, T_final) - CostMatrix[current][s] + Inertia(current, s)

Transition Cost Matrix (5×5):

text

         Idle  Chase  Attack  Retreat  Flank
Idle      0.0   0.05   0.10    0.05    0.15
Chase     0.10  0.0    0.05    0.15    0.10
Attack    0.15  0.10   0.0     0.20    0.15
Retreat   0.10  0.15   0.20    0.0     0.25
Flank     0.15  0.10   0.10    0.15    0.0

Инерция:

text

Inertia(s) = min(InertiaMax, InertiaGrowthRate · TimeInCurrentState)
// Если s == current_state → +Inertia, иначе 0

Финальный выбор:

text

s* = argmax(Score(s))  для всех s ∈ States

Фаза 8: Адаптивные пороги

text

μ_T = скользящее среднее T_final за последние K оценок  (K = 20)
σ_T = скользящее стандартное отклонение

HighThreatThreshold = μ_T + σ_T
LowThreatThreshold  = μ_T - σ_T

Эти пороги используются в utility-кривых (сдвигают center колоколов). Враг, привыкший к высокому давлению, будет считать средний уровень давления «нормальным» и не паниковать.
2.2 Усиленная система памяти
Структура события памяти:

C++

USTRUCT(BlueprintType)
struct FMemoryEvent
{
    GENERATED_BODY()

    UPROPERTY() EPlayerActionType ActionType;  // Shot, Dash, MeleeAttack, Heal, SpecialAbility
    UPROPERTY() float Timestamp;               // GameTime when occurred
    UPROPERTY() FVector Location;              // Where it happened
    UPROPERTY() float Intensity;               // Base threat of this action [0,1]
};

Временное затухание:

text

M_i(t) = Intensity_i · e^(-λ · (t_current - t_event))

Где λ = 0.3 — скорость затухания. Событие 3 секунды назад сохраняет ~40% веса, 10 секунд назад — ~5%.
Агрегированные метрики памяти:

text

PlayerAggressiveness = Σ M_i(t)  для ActionType ∈ {Shot, MeleeAttack}
PlayerMobility       = Σ M_i(t)  для ActionType ∈ {Dash}
PlayerCaution        = Σ M_i(t)  для ActionType ∈ {Heal, Retreat}

Распознавание паттернов (N-граммы):

text

Хранить последние 10–15 действий игрока как последовательность.
Подсчитать частоту 2-грамм и 3-грамм:
  - (Dash, MeleeAttack) — "ворвался в ближний бой"
  - (Shot, Shot, Dash) — "стреляет и уходит"
  - (Dash, Dash, MeleeAttack) — "агрессивный раш"

PlayerDominantPattern = argmax(frequency(n-gram))

На основе доминантного паттерна — дополнительный модификатор для utility-функций:

    Если игрок часто дашит в ближний бой → Retreat/Flank получают бонус
    Если игрок стреляет издалека → Chase получает бонус

На защите: «Система распознаёт типичную тактику игрока через частотный анализ N-грамм последовательностей действий и адаптирует выбор состояния.»
2.3 Полная схема принятия решений (обобщение)

text

[Мир] → Layer 1: Context Evaluation
                ↓
         Нормализация + нелинейные преобразования
                ↓
        Layer 2: Threat Calculator
         T_base + T_cross → T_raw
         Confidence → T_smooth → T_final
                ↓
         Dynamic Weight Update (feedback)
                ↓
        Layer 3: State Transition Resolver
         Utility scoring + Cost Matrix + Inertia
                ↓
        Memory Module → Pattern Recognition → Utility Modifiers
                ↓
         s* = argmax(Score)
                ↓
        FSM.TransitionTo(s*)

2.4 Почему эти усиления критичны для дипломной защиты
Усиление	Что покажете на защите	Академическая ценность
Нелинейные преобразования	Графики sigmoid/power с объяснением	Демонстрация математической грамотности
Динамические веса	График изменения весов в бою	Доказательство адаптивности системы
Time-decay memory	Кривая затухания событий	Формальная модель «забывания»
N-gram паттерны	Таблица распознанных паттернов	Оригинальность алгоритма
Utility curves	Графики утилитарных функций для каждого состояния	Обоснованный выбор, а не if/else
Transition cost	Матрица 5×5	Формализация стоимости переходов
Confidence	Поведение при нестабильных данных	Робастность системы
Adaptive thresholds	Сдвиг порогов со временем	Адаптация к стилю игрока
ЧАСТЬ 3: ПОЛНАЯ ИЕРАРХИЯ КЛАССОВ
Source Structure

text

Source/Depthrun/
├── Core/
│   ├── DepthrunGameMode.h/.cpp
│   ├── DepthrunGameInstance.h/.cpp
│   └── DepthrunLogChannels.h
│
├── Player/
│   ├── DepthrunCharacter.h/.cpp          // APaperCharacter-derived
│   ├── PlayerCombatComponent.h/.cpp      // UActorComponent
│   └── PlayerActionTracker.h/.cpp        // UActorComponent — records actions for enemy memory
│
├── Enemy/
│   ├── BaseEnemy.h/.cpp                  // APaperCharacter-derived
│   ├── MeleeEnemy.h/.cpp
│   ├── RangedEnemy.h/.cpp
│   ├── AdaptiveEnemy.h/.cpp              // MAIN DEMONSTRATION ENEMY
│   ├── EnemyHealthComponent.h/.cpp       // UActorComponent
│   └── EnemyTypes.h                      // Enums, shared types
│
├── FSM/
│   ├── FSMTypes.h                        // EFSMStateType enum, delegates
│   ├── FSMComponent.h/.cpp               // UActorComponent — manages states
│   ├── FSMState.h/.cpp                   // UObject — abstract base state
│   └── States/
│       ├── FSMState_Idle.h/.cpp
│       ├── FSMState_Chase.h/.cpp
│       ├── FSMState_Attack.h/.cpp
│       ├── FSMState_Retreat.h/.cpp
│       └── FSMState_Flank.h/.cpp
│
├── AdaptiveBehavior/                           // ★ CORE MODULE — 60% of diploma
│   ├── AdaptiveTypes.h                   // All structs, enums for adaptive system
│   ├── AdaptiveBehaviorComponent.h/.cpp  // UActorComponent — orchestrator
│   ├── ContextEvaluator.h/.cpp           // UObject — Layer 1
│   ├── ThreatCalculator.h/.cpp           // UObject — Layer 2
│   ├── StateTransitionResolver.h/.cpp    // UObject — Layer 3
│   ├── AdaptiveMemory.h/.cpp             // UObject — memory with time decay
│   ├── PatternRecognizer.h/.cpp          // UObject — N-gram pattern detection
│   ├── DynamicWeightManager.h/.cpp       // UObject — weight adaptation via feedback
│   ├── TransitionCostMatrix.h/.cpp       // UObject or struct — 5×5 cost matrix
│   ├── UtilityCurves.h/.cpp              // UObject — utility functions per state
│   └── AdaptiveConfig.h                  // UDataAsset — tunable parameters
│
├── Combat/
│   ├── BaseWeapon.h/.cpp                 // AActor
│   ├── RangedWeapon.h/.cpp
│   ├── MeleeWeapon.h/.cpp
│   └── BaseProjectile.h/.cpp             // AActor
│
├── RoomGeneration/
│   ├── RoomBase.h/.cpp                   // AActor — base room
│   ├── RoomGeneratorSubsystem.h/.cpp     // UWorldSubsystem
│   ├── RoomTemplate.h/.cpp               // UDataAsset
│   └── RoomTransitionVolume.h/.cpp       // AActor — trigger for room change
│
├── UI/
│   ├── DepthrunHUD.h/.cpp                // AHUD or UUserWidget
│   ├── HealthBarWidget.h/.cpp            // UUserWidget
│   └── DebugAdaptiveWidget.h/.cpp        // UUserWidget — debug overlay for module
│
├── Data/
│   ├── DepthrunSaveSubsystem.h/.cpp      // UGameInstanceSubsystem
│   └── SQLiteManager.h/.cpp              // Wrapper around SQLiteCore
│
└── Utils/
    └── MathUtils.h                       // Sigmoid, bell curve, clamp helpers

Что строго в C++ vs что допустимо в Blueprint
Система	C++	Blueprint допустим
FSM (все состояния, переходы, логика)	✅ Обязательно	❌ Запрещено
AdaptiveAI (все компоненты)	✅ Обязательно	❌ Запрещено
ThreatCalculator, Memory, Patterns	✅ Обязательно	❌ Запрещено
Player movement	✅ Обязательно	❌
Combat system (damage, weapons)	✅ Обязательно	❌
Enemy base logic	✅ Обязательно	❌
Room generation	✅ Обязательно	❌
SQLite save/load	✅ Обязательно	❌
UI виджеты (визуал)	Базовый класс C++	✅ Вёрстка в UMG Editor
Flipbook assignment	—	✅ Human в Editor
Niagara эффекты	—	✅ Настройка в Editor
Tilemap setup	—	✅ Human в Editor
Level design	—	✅ Human в Editor
Debug visualization widget	C++ логика	✅ Вёрстка Blueprint
ЧАСТЬ 4: PRODUCTION ROADMAP
STAGE 0: Создание проекта и начальная настройка

Цель: Пустой работающий UE5 C++ проект со сгенерированным Visual Studio solution.
Действия:

    [HUMAN] Установить UE 5.4+ через Epic Games Launcher
    [HUMAN] Создать новый C++ проект: Template = «Blank», Name = Depthrun, C++ (не Blueprint)
    [HUMAN] Открыть проект, убедиться что Editor запускается
    [HUMAN] File → Generate Visual Studio Project Files
    [HUMAN] Открыть .sln в Visual Studio, проверить Build (Development Editor, Win64)
    [HUMAN] Инициализировать Git-репозиторий: git init, создать .gitignore для UE5
    [AI] Сгенерировать .gitignore для UE5 проекта (исключить Binaries/, Intermediate/, Saved/, DerivedDataCache/, .vs/)
    [AI] Сгенерировать начальный README.md с описанием проекта

Структура на этом этапе:

text

Depthrun/
├── Source/Depthrun/
│   ├── Depthrun.Build.cs
│   ├── Depthrun.h
│   ├── Depthrun.cpp
│   └── (GameMode files from template)
├── Content/
├── Config/
├── Depthrun.uproject
└── .gitignore

Валидация:

    Проект компилируется в VS без ошибок
    Editor запускается
    Git commit: init: empty UE5 C++ project

STAGE 1: Настройка движка и плагинов

Цель: Включить все необходимые плагины и модули.
Действия:

    [HUMAN] Edit → Plugins → включить Paper2D (обычно включён по умолчанию, проверить)
    [HUMAN] Edit → Plugins → включить Niagara (обычно включён)
    [HUMAN] Edit → Plugins → проверить Enhanced Input (включён по умолчанию в UE5)
    [HUMAN] Edit → Project Settings → Input → установить Default Classes для Enhanced Input (UEnhancedInputComponent, UEnhancedInputLocalPlayerSubsystem)
    [HUMAN] Edit → Project Settings → Maps & Modes → указать свой GameMode (если уже создан)
    [AI] Обновить Depthrun.Build.cs — добавить модули:

csharp

PublicDependencyModuleNames.AddRange(new string[] {
    "Core", "CoreUObject", "Engine", "InputCore",
    "EnhancedInput",
    "Paper2D",
    "Niagara",
    "SQLiteCore",       // built-in UE5 SQLite
    "UMG", "Slate", "SlateCore"
});

    [HUMAN] Build в VS → убедиться что компилируется с новыми зависимостями
    [HUMAN] Перезапустить Editor после изменения Build.cs

Валидация:

    Компиляция с Paper2D, Niagara, SQLiteCore, EnhancedInput проходит без ошибок
    Git commit: setup: enable Paper2D, Niagara, SQLiteCore, EnhancedInput plugins

STAGE 2: Структура папок и базовые файлы

Цель: Создать полную структуру папок Content и Source.
Действия:

    [HUMAN] В Content Browser создать папки:

    text

    Content/
    ├── Characters/
    ├── Enemies/
    ├── FSM/
    ├── Combat/
    ├── Levels/
    ├── TileMaps/
    ├── UI/
    ├── Data/
    ├── ImportedAssets/
    │   ├── Sprites/
    │   ├── Flipbooks/
    │   └── Tilesets/
    ├── VFX/
    └── Audio/

    [AI] Создать всю структуру Source-папок (как в части 3)

    [AI] Создать DepthrunLogChannels.h:

C++

#pragma once
#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDepthrun, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogFSM, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogAdaptiveBehavior, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogCombat, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogRoomGen, Log, All);

    [AI] Создать MathUtils.h с helper-функциями:

C++

namespace DepthrunMath
{
    float Sigmoid(float X, float K = 10.f, float Center = 0.5f);
    float BellCurve(float X, float Center, float Width);
    float TimeDecay(float BaseValue, float Lambda, float DeltaTime);
    float NormalizeToRange(float Value, float Min, float Max);
}

    [AI] Создать DepthrunGameMode.h/.cpp и DepthrunGameInstance.h/.cpp (базовые, с логированием)

Валидация:

    Все папки видны в Content Browser и Explorer
    Source компилируется
    Git commit: structure: create project folder hierarchy and base utility files

STAGE 3: Игрок (Player Character)

Цель: Управляемый Paper2D персонаж с движением, анимациями, HP, и базовым оружием.
3A. Базовый Player

    [AI] Создать ADepthrunCharacter наследуясь от APaperCharacter:
        Enhanced Input для WASD-движения
        UPaperFlipbookComponent для анимаций
        4 направления (влево/вправо/вверх/вниз)
        Capsule collision для 2D
        Top-down camera (USpringArmComponent + UCameraComponent)
        UPROPERTY для MaxHP, CurrentHP
        EPlayerFacingDirection enum

    [AI] Создать UPlayerCombatComponent:
        Слот оружия (ABaseWeapon*)
        Функции Attack(), EquipWeapon()
        Делегат OnPlayerAction — для оповещения системы памяти врагов

    [AI] Создать UPlayerActionTracker:
        Запись каждого действия игрока с таймстампом
        Broadcast через делегат для всех врагов в комнате
        EPlayerActionType { Shot, Dash, MeleeAttack, Heal, SpecialAbility }
        FPlayerActionEvent { EPlayerActionType, float Timestamp, FVector Location }

    [AI] Создать Input Actions (C++ часть): IA_Move, IA_Attack, IA_Dash

    [HUMAN] Создать Input Mapping Context в Editor

    [HUMAN] Назначить Input Actions в IMC

    [HUMAN] Импортировать спрайты игрока в Content/ImportedAssets/Sprites/

    [HUMAN] Создать Flipbooks из спрайтов для 4 направлений: Idle, Walk, Attack

    [HUMAN] Настроить коллизии капсулы

    [HUMAN] Разместить персонажа на пустом уровне, протестировать движение

Зависимости: Stage 2 завершён.

Ожидаемый результат: Спрайт двигается по экрану в 4 направлениях, камера следует за ним.

Валидация:

    WASD-движение работает
    Анимации переключаются по направлению
    PlayerActionTracker логирует события
    Git commit: feat(player): implement character movement, input, and action tracker

3B. Оружие игрока

    [AI] Создать ABaseWeapon (AActor):
        EWeaponType { Melee, Ranged }
        Virtual Fire(), GetDamage(), GetWeaponType()
        Cooldown logic

    [AI] Создать ARangedWeapon:
        Spawn ABaseProjectile при Fire()
        Направление на основе FacingDirection

    [AI] Создать AMeleeWeapon:
        Damage zone перед персонажем
        USphereComponent или UBoxComponent overlap check

    [AI] Создать ABaseProjectile:
        UProjectileMovementComponent
        UPaperSpriteComponent для визуала
        USphereComponent для коллизии
        OnHit → apply damage → destroy

    [HUMAN] Назначить спрайты снарядам

    [HUMAN] Протестировать стрельбу и ближний удар

Валидация:

    Снаряды летят в нужном направлении
    Мили-удар наносит урон в зоне
    CombatComponent корректно переключает оружие
    Git commit: feat(combat): implement ranged and melee weapons with projectiles

3C. Dash

    [AI] Реализовать Dash() в ADepthrunCharacter:
        Быстрое перемещение в направлении движения
        Кулдаун
        Invincibility frames (опционально)
        Запись EPlayerActionType::Dash в PlayerActionTracker

Валидация:

    Даш работает, кулдаун соблюдается
    Git commit: feat(player): implement dash with cooldown and action tracking

STAGE 4: Базовый враг

Цель: ABaseEnemy с HP, смертью, спрайтами, коллизиями. Пока без FSM — просто стоит.
Действия:

    [AI] Создать ABaseEnemy (APaperCharacter):
        UEnemyHealthComponent — HP, TakeDamage, OnDeath delegate
        UPaperFlipbookComponent — анимации
        EEnemyType { Melee, Ranged, Adaptive } enum
        Базовые виртуальные методы: OnSpawned(), OnKilled()
        Не содержит логики поведения — только физический объект

    [AI] Создать UEnemyHealthComponent:
        MaxHP, CurrentHP (UPROPERTY)
        ApplyDamage(float) → clamp → check death
        OnHealthChanged multicast delegate
        OnDeath multicast delegate

    [AI] Создать AMeleeEnemy и ARangedEnemy (простые наследники, минимальная логика — одно состояние атаки, hardcoded поведение для контраста с адаптивным врагом)

    [HUMAN] Импортировать спрайты врагов

    [HUMAN] Создать Flipbooks

    [HUMAN] Разместить врага на уровне, протестировать получение урона

Валидация:

    Враг получает урон от оружия игрока
    HP уменьшается, при 0 — враг уничтожается
    Git commit: feat(enemy): implement base enemy with health component

STAGE 5: FSM Core Module

Цель: Полностью кастомная, переиспользуемая FSM, отделённая от логики актора.

Критично: Это начало ядра дипломной работы. FSM — архитектурная основа, на которую ложится адаптивный модуль.
5A. Базовые типы и абстрактный State

    [AI] Создать FSMTypes.h:

C++

UENUM(BlueprintType)
enum class EFSMStateType : uint8
{
    None    UMETA(DisplayName = "None"),
    Idle    UMETA(DisplayName = "Idle"),
    Chase   UMETA(DisplayName = "Chase"),
    Attack  UMETA(DisplayName = "Attack"),
    Retreat UMETA(DisplayName = "Retreat"),
    Flank   UMETA(DisplayName = "Flank")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnFSMStateChanged, EFSMStateType, OldState, EFSMStateType, NewState);

    [AI] Создать UFSMState (UObject):
        virtual void EnterState(ABaseEnemy* Owner)
        virtual void TickState(ABaseEnemy* Owner, float DeltaTime)
        virtual void ExitState(ABaseEnemy* Owner)
        EFSMStateType GetStateType() const
        float TimeInState — время пребывания (для инерции)

C++

UCLASS(Abstract)
class DEPTHRUN_API UFSMState : public UObject
{
    GENERATED_BODY()
public:
    virtual void EnterState(ABaseEnemy* Owner);
    virtual void TickState(ABaseEnemy* Owner, float DeltaTime);
    virtual void ExitState(ABaseEnemy* Owner);
    virtual EFSMStateType GetStateType() const PURE_VIRTUAL(UFSMState::GetStateType, return EFSMStateType::None;);

protected:
    float TimeInState = 0.f;
};

5B. Конкретные состояния

    [AI] Создать UFSMState_Idle:
        Стоит на месте, периодически осматривается
        Запрашивает переоценку каждые N секунд

    [AI] Создать UFSMState_Chase:
        Движение к игроку (простой MoveToward)
        Проверка дистанции для перехода в Attack

    [AI] Создать UFSMState_Attack:
        Если melee — ближняя атака
        Если ranged — стрельба
        Кулдауны, анимации

    [AI] Создать UFSMState_Retreat:
        Движение от игрока
        Поиск безопасного направления (от игрока, не в стену)

    [AI] Создать UFSMState_Flank:
        Движение перпендикулярно вектору к игроку
        Попытка зайти сбоку/сзади

5C. FSM Component

    [AI] Создать UFSMComponent (UActorComponent):
        TMap<EFSMStateType, UFSMState*> States — реестр состояний
        UFSMState* CurrentState
        void RegisterState(EFSMStateType, UFSMState*)
        void TransitionTo(EFSMStateType NewState) — вызывает Exit → Enter
        void TickComponent() — делегирует в CurrentState->TickState
        FOnFSMStateChanged OnStateChanged — делегат для логирования/debug
        EFSMStateType GetCurrentStateType()
        float GetTimeInCurrentState()
        НЕ принимает решений — только исполняет переходы

text

┌──────────────┐
│  Enemy Actor │ ← знает только "у меня есть FSM"
│              │
│ ┌──────────┐ │
│ │   FSM    │ │ ← управляет состояниями, не знает об адаптации
│ │Component │ │
│ └──────────┘ │
│ ┌──────────┐ │
│ │ Adaptive │ │ ← принимает решения, говорит FSM "перейди в X"
│ │Behavior  │ │
│ │Component │ │
│ └──────────┘ │
└──────────────┘

Валидация:

    FSM переключает состояния через TransitionTo()
    Логирование переходов в LogFSM
    Враг в состоянии Chase бежит к игроку
    Враг в Retreat убегает
    Нет зависимости FSM от AdapriveBehavior (компиляция FSM без AdapriveBehavior)
    Git commit: feat(fsm): implement custom FSM component with 5 states

STAGE 6: Адаптивный модуль (ЯДРО ДИПЛОМНОЙ РАБОТЫ)

Цель: Полная реализация 3-слойной адаптивной системы с 8 усилениями.

Это ~60% дипломной работы. Каждый подэтап — отдельный commit.
6A. Типы и конфигурация

    [AI] Создать AdaptiveTypes.h:

C++

UENUM(BlueprintType)
enum class EPlayerActionType : uint8
{
    Shot, Dash, MeleeAttack, Heal, SpecialAbility
};

USTRUCT(BlueprintType)
struct FMemoryEvent
{
    GENERATED_BODY()
    EPlayerActionType ActionType;
    float Timestamp;
    FVector Location;
    float Intensity;  // [0, 1]
};

USTRUCT(BlueprintType)
struct FContextData
{
    GENERATED_BODY()
    float DistanceToPlayer;
    EWeaponType PlayerWeaponType;
    float EnemyHPRatio;     // [0, 1]
    int32 NearbyAllyCount;
    float RoomDensity;       // [0, 1]
    TArray<FMemoryEvent> RecentMemory;
};

USTRUCT(BlueprintType)
struct FThreatAssessment
{
    GENERATED_BODY()
    float ThreatBase;
    float ThreatCross;
    float ThreatRaw;
    float ThreatSmoothed;
    float ThreatFinal;
    float Confidence;
};

USTRUCT(BlueprintType)
struct FStateScore
{
    GENERATED_BODY()
    EFSMStateType State;
    float UtilityValue;
    float TransitionCost;
    float InertiaBonus;
    float PatternModifier;
    float FinalScore;
};

    [AI] Создать UAdaptiveConfig (UDataAsset):
        Все настраиваемые параметры:
            MaxEngagementRange, MaxAllyThreshold, MaxRoomCapacity
            SigmoidSteepness_Distance
            HealthPowerExponent
            MemoryDecayLambda
            WeightLearningRate
            WeightMin, WeightMax
            CrossTermBeta
            SmoothingAlpha
            InertiaGrowthRate, InertiaMax
            PatternWindowSize
            AdaptiveThresholdWindow
            Initial weights w1..w6
            TransitionCostMatrix[5][5]
            Utility curve parameters per state

    Git commit: feat(adaptive): define core types, structs, and config data asset

6B. MathUtils

    [AI] Реализовать MathUtils.cpp:

C++

float DepthrunMath::Sigmoid(float X, float K, float Center)
{
    return 1.f / (1.f + FMath::Exp(-K * (X - Center)));
}

float DepthrunMath::BellCurve(float X, float Center, float Width)
{
    float Exponent = -FMath::Square(X - Center) / (2.f * FMath::Square(Width));
    return FMath::Exp(Exponent);
}

float DepthrunMath::TimeDecay(float BaseValue, float Lambda, float DeltaTime)
{
    return BaseValue * FMath::Exp(-Lambda * DeltaTime);
}

    Git commit: feat(adaptive): implement math utility functions (sigmoid, bell, decay)

6C. Layer 1 — Context Evaluator

    [AI] Создать UContextEvaluator (UObject):
        FContextData EvaluateContext(ABaseEnemy* Owner, ADepthrunCharacter* Player)
        Сбор всех данных: дистанция, оружие, HP, союзники (SphereOverlap), плотность комнаты
        Нормализация всех значений к [0, 1]
        Применение нелинейных преобразований (sigmoid для D, power для H)

C++

UCLASS()
class DEPTHRUN_API UContextEvaluator : public UObject
{
    GENERATED_BODY()
public:
    FContextData EvaluateContext(const ABaseEnemy* Owner,
                                 const ADepthrunCharacter* Player,
                                 const UAdaptiveConfig* Config) const;

private:
    float NormalizeDistance(float RawDistance, float MaxRange) const;
    float NormalizeHealth(float HPRatio, float Exponent) const;
    float EvaluateRoomDensity(const ABaseEnemy* Owner) const;
    int32 CountNearbyAllies(const ABaseEnemy* Owner, float Radius) const;
};

    Git commit: feat(adaptive): implement Layer 1 - context evaluator with normalization

6D. Система памяти

    [AI] Создать UAdaptiveMemory (UObject):
        TArray<FMemoryEvent> MemoryBuffer — кольцевой буфер (макс. 50 событий)
        void RecordEvent(FMemoryEvent Event) — добавить событие
        float GetDecayedAggressiveness(float CurrentTime, float Lambda) — агрессивность с затуханием
        float GetDecayedMobility(float CurrentTime, float Lambda) — мобильность с затуханием
        float GetDecayedCaution(float CurrentTime, float Lambda)
        void CleanupOldEvents(float CurrentTime, float MaxAge) — удалить старые

C++

float UAdaptiveMemory::GetDecayedMetric(float CurrentTime, float Lambda,
    TFunctionRef<bool(EPlayerActionType)> Filter) const
{
    float Total = 0.f;
    for (const FMemoryEvent& Event : MemoryBuffer)
    {
        if (Filter(Event.ActionType))
        {
            float DeltaT = CurrentTime - Event.Timestamp;
            Total += Event.Intensity * FMath::Exp(-Lambda * DeltaT);
        }
    }
    return Total;
}

    Git commit: feat(adaptive): implement time-decay memory system

6E. Распознавание паттернов (N-граммы)

    [AI] Создать UPatternRecognizer (UObject):
        Хранит последние N действий как TArray<EPlayerActionType> (N=15)
        Подсчёт 2-грамм и 3-грамм (TMap<FString, int32>)
        FString GetDominantPattern() — самый частый паттерн
        float GetPatternModifier(EFSMStateType CandidateState) — модификатор для каждого кандидатного состояния

C++

// Example pattern → state modifier mapping:
// "Dash+MeleeAttack" (aggressive rush) → Retreat +0.2, Flank +0.15
// "Shot+Shot+Dash" (kiting) → Chase +0.2
// "Dash+Dash" (evasive) → Attack -0.1

    Git commit: feat(adaptive): implement N-gram pattern recognition for player actions

6F. Layer 2 — Threat Calculator

    [AI] Создать UThreatCalculator (UObject):
        FThreatAssessment CalculateThreat(FContextData, UAdaptiveMemory*, UDynamicWeightManager*, UAdaptiveConfig*)
        Реализация:
            T_base = Σ w_i · f_i(x_i)
            T_cross = w_DH · f_D · f_H + w_WM · W · M_attack
            T_raw = T_base + β · T_cross
            Confidence: C = 1 / (1 + σ²) по последним N оценкам
            Smoothing: T_smooth = α · T_raw + (1-α) · T_prev
            T_final = C · T_smooth + (1-C) · T_default
        Хранит историю T_raw (циклический буфер, N=10) для вычисления σ²

    Git commit: feat(adaptive): implement Layer 2 - threat calculator with confidence and smoothing

6G. Dynamic Weight Manager

    [AI] Создать UDynamicWeightManager (UObject):
        TArray<float> Weights — текущие веса (6 штук)
        void UpdateWeights(float Reward, FContextData LastContext, UAdaptiveConfig*)
        Реализация:
            contribution_i = f_i(x_i) / Σ f_j(x_j) — вклад каждого фактора
            w_i(t+1) = clamp(w_i(t) + η · reward · contribution_i, w_min, w_max)
            Reward: +1 если враг нанёс урон, -1 если получил
        const TArray<float>& GetWeights() const
        void ResetToDefaults(UAdaptiveConfig*)

    Git commit: feat(adaptive): implement dynamic weight adjustment via feedback loop

6H. Utility Curves

    [AI] Создать UUtilityCurves (UObject):
        Для каждого из 5 состояний — функция float EvaluateUtility(EFSMStateType, float ThreatFinal, FContextData)
        Кривые (как описано в 2.1 Фаза 6):
            Idle: max(0, 1 - 4T²)
            Chase: BellCurve(T, 0.3, 0.2)
            Attack: BellCurve(T, 0.5, 0.25)
            Flank: BellCurve(T, 0.6, 0.2) · A_norm
            Retreat: Sigmoid(T, 12, 0.75)
        Параметры кривых читаются из UAdaptiveConfig

    Git commit: feat(adaptive): implement utility curve evaluation for all FSM states

6I. Transition Cost Matrix

    [AI] Создать UTransitionCostMatrix (UObject или FStruct):
        float CostMatrix[5][5] — заполняется из Config
        float GetCost(EFSMStateType From, EFSMStateType To) const
        Инерция: float CalculateInertia(EFSMStateType Current, float TimeInState, UAdaptiveConfig*)

    Git commit: feat(adaptive): implement transition cost matrix and state inertia

6J. Layer 3 — State Transition Resolver

    [AI] Создать UStateTransitionResolver (UObject):
        EFSMStateType ResolveNextState(EFSMStateType Current, FThreatAssessment, UUtilityCurves*, UTransitionCostMatrix*, UPatternRecognizer*, float TimeInCurrentState, FContextData, UAdaptiveConfig*)
        Реализация:
            Для каждого состояния s:
            Score(s) = Utility(s) - TransitionCost(current, s) + Inertia(current, s) + PatternModifier(s)
            s* = argmax(Score(s))
            Если s* == current → не переходить (стабильность)
            Иначе → вернуть s*

    Git commit: feat(adaptive): implement Layer 3 - state transition resolver with full scoring

6K. Адаптивные пороги

    [AI] Добавить в UThreatCalculator:
        Скользящее среднее μ и σ по T_final (окно K=20)
        float GetHighThreatThreshold() → μ + σ
        float GetLowThreatThreshold() → μ - σ
        Передавать в UUtilityCurves для сдвига center колоколов

    Git commit: feat(adaptive): implement adaptive thresholds with running statistics

6L. Orchestrator — AdaptiveBehaviorComponent

    [AI] Создать UAdaptiveBehaviorComponent (UActorComponent):
        Владеет всеми подсистемами: UContextEvaluator*, UThreatCalculator*, UStateTransitionResolver*, UAdaptiveMemory*, UPatternRecognizer*, UDynamicWeightManager*, UUtilityCurves*, UTransitionCostMatrix*
        UAdaptiveConfig* Config — назначается в Editor
        void InitializeSubsystems() — создание всех UObject'ов в BeginPlay
        Tick (или Timer, ~0.2–0.5 сек):
            Context = ContextEvaluator->Evaluate()
            Threat = ThreatCalculator->Calculate(Context, Memory, Weights)
            NewState = Resolver->Resolve(CurrentState, Threat, ...)
            If NewState != CurrentState → FSM->TransitionTo(NewState)
            Weights->UpdateWeights(reward) при получении/нанесении урона (через делегат)
        Делегат от EnemyHealthComponent → OnDamageTaken → reward = -1
        Делегат от CombatSystem → OnDamageDealt → reward = +1
        Делегат от PlayerActionTracker → OnPlayerAction → Memory->RecordEvent, PatternRecognizer->AddAction

text

┌─────────────────────────────────────────────┐
│         UAdaptiveBehaviorComponent           │
│                                              │
│  ┌──────────────┐  ┌──────────────────────┐ │
│  │ContextEval   │→ │ ThreatCalculator     │ │
│  │(Layer 1)     │  │ (Layer 2)            │ │
│  └──────────────┘  │  + AdaptiveThresholds│ │
│                     │  + Confidence        │ │
│  ┌──────────────┐  └──────────┬───────────┘ │
│  │AdaptiveMemory│             │              │
│  │+ TimeDecay   │───┐        ↓              │
│  └──────────────┘   │  ┌──────────────────┐ │
│  ┌──────────────┐   ├─→│StateTransition   │ │
│  │PatternRecog  │───┘  │Resolver (Layer 3)│ │
│  │(N-grams)     │      │+ UtilityCurves   │ │
│  └──────────────┘      │+ CostMatrix      │ │
│  ┌──────────────┐      │+ Inertia         │ │
│  │DynamicWeight │──────┘                   │ │
│  │Manager       │   ↓                      │ │
│  └──────────────┘   FSM->TransitionTo(s*)  │ │
└─────────────────────────────────────────────┘

    Git commit: feat(adaptive): implement AdaptiveBehaviorComponent orchestrator

6M. Debug Logging

    [AI] Добавить подробное логирование в каждый компонент:
        AdapriveBehavior: текущий контекст, T_final, Score для каждого состояния, выбранное состояние
        Формат: [AdapriveBehavior] Enemy=%s | T=%.3f C=%.3f | Idle=%.2f Chase=%.2f Attack=%.2f Retreat=%.2f Flank=%.2f | → %s
        Вывод весов после каждого обновления
        Вывод доминантного паттерна

    Git commit: feat(adaptive): add comprehensive debug logging for all subsystems

Суммарная валидация Stage 6:

    Все подсистемы компилируются и создаются в BeginPlay
    Context evaluator возвращает нормализованные данные
    Threat calculator выдаёт T_final ∈ [0, 1]
    Utility curves дают разные значения для разных T
    State resolver корректно выбирает состояние
    Динамические веса меняются при получении/нанесении урона
    Память затухает со временем
    Паттерн распознаётся после 5-6 повторений
    Вся цепочка работает: Контекст → Угроза → Решение → FSM-переход
    AdapriveBehavior модуль компилируется независимо от конкретного врага (зависит только от интерфейсов)

STAGE 7: Адаптивный враг (демонстрационный)

Цель: AAdaptiveEnemy — главный демонстрационный враг, объединяющий FSM + адаптивный модуль.
Действия:

    [AI] Создать AAdaptiveEnemy (наследник ABaseEnemy):
        Добавляет UFSMComponent + UAdaptiveBehaviorComponent
        В BeginPlay: регистрирует 5 состояний в FSM, инициализирует адаптивный модуль
        Сам враг не содержит логики поведения — только связывает компоненты
        Визуально отличается от простых врагов (другой спрайт/цвет)

C++

void AAdaptiveEnemy::BeginPlay()
{
    Super::BeginPlay();

    // Register FSM states
    FSMComp->RegisterState(EFSMStateType::Idle,    NewObject<UFSMState_Idle>(this));
    FSMComp->RegisterState(EFSMStateType::Chase,   NewObject<UFSMState_Chase>(this));
    FSMComp->RegisterState(EFSMStateType::Attack,  NewObject<UFSMState_Attack>(this));
    FSMComp->RegisterState(EFSMStateType::Retreat, NewObject<UFSMState_Retreat>(this));
    FSMComp->RegisterState(EFSMStateType::Flank,   NewObject<UFSMState_Flank>(this));

    // Start in Idle
    FSMComp->TransitionTo(EFSMStateType::Idle);

    // Subscribe to player action tracker
    if (ADepthrunCharacter* Player = /* find player */)
    {
        Player->GetActionTracker()->OnPlayerAction.AddDynamic(
            AdaptiveComp, &UAdaptiveBehaviorComponent::HandlePlayerAction);
    }
}

    [AI] Создать UDebugAdaptiveWidget — overlay виджет, показывающий в реальном времени:
        Текущее состояние FSM
        T_final числом и полоской
        Confidence
        Текущие веса (гистограмма)
        Распознанный паттерн
        Утилиты каждого состояния
        Этот виджет критичен для защиты диплома — комиссия видит работу модуля

    [HUMAN] Назначить уникальный спрайт адаптивному врагу

    [HUMAN] Разместить на уровне: игрок + адаптивный враг

    [HUMAN] Протестировать полный цикл:
        Начать бой → враг в Idle → обнаружил → Chase → подбежал → Attack
        Получил урон → T растёт → Retreat
        Игрок дашит + бьёт повторно → паттерн распознаётся → Flank
        Веса меняются

Валидация:

    Враг демонстрирует разные состояния в зависимости от ситуации
    Debug widget показывает все внутренние данные
    Поведение визуально меняется в процессе боя
    Git commit: feat(enemy): implement adaptive enemy integrating FSM and adaptive module
    Git commit: feat(ui): implement debug overlay widget for adaptive behavior visualization

STAGE 8: Процедурная генерация комнат

Цель: Roguelike-структура с комнатами, переходами, спаунингом врагов.
8A. Room Base

    [AI] Создать ARoomBase (AActor):
        UBoxComponent — границы комнаты
        TArray<FVector> SpawnPoints — точки спауна врагов
        TArray<FVector> ExitPoints — точки дверей
        ERoomType { Combat, Rest, Boss, Start }
        void ActivateRoom() — спаунить врагов, закрыть двери
        void DeactivateRoom() — открыть двери после очистки

    [AI] Создать URoomTemplate (UDataAsset):
        Tilemap reference
        Список врагов для спауна
        Размер комнаты
        Difficulty rating

8B. Room Generator

    [AI] Создать URoomGeneratorSubsystem (UWorldSubsystem):
        Алгоритм: простой граф комнат (BSP или Random Walk)
        Генерация последовательности комнат
        Назначение типов комнат
        Размещение дверей/переходов
        Спаунинг tilemap для каждой комнаты

    [AI] Создать ARoomTransitionVolume — триггер перехода между комнатами

    [HUMAN] Создать 3-5 tilemap шаблонов комнат

    [HUMAN] Настроить коллизии тайлмапов

    [HUMAN] Протестировать переходы между комнатами

Валидация:

    Генерируется последовательность из 5+ комнат
    Враги спаунятся в комнатах
    Двери закрываются при входе, открываются после зачистки
    Git commit: feat(rooms): implement procedural room generation system

STAGE 9: UI система

Цель: HUD с HP, мини-информация, меню.
Действия:

    [AI] Создать UDepthrunHUD — основной виджет:
        HP bar игрока
        Текущее оружие
        Номер комнаты/этажа

    [AI] Создать UHealthBarWidget — переиспользуемый для игрока и врагов

    [AI] Создать UMainMenuWidget, UPauseMenuWidget, UGameOverWidget

    [HUMAN] Сверстать виджеты в UMG Editor

    [HUMAN] Назначить HUD в GameMode

Валидация:

    HP отображается и обновляется при уроне
    Меню работают
    Git commit: feat(ui): implement HUD, health bars, and menu widgets

STAGE 10: Save/Load через SQLite

Цель: Сохранение метапрогрессии между забегами.
Действия:

    [AI] Создать USQLiteManager:
        Обёртка над FSQLiteDatabase (SQLiteCore)
        bool OpenDatabase(FString Path)
        bool ExecuteQuery(FString SQL)
        bool InsertRow(...), TArray<FRow> SelectRows(...)

    [AI] Создать UDepthrunSaveSubsystem (UGameInstanceSubsystem):
        Таблицы: player_progress, run_history, adaptive_weights (опционально — для анализа)
        SaveRunResult(...) — после каждого забега
        LoadProgress(...) — при запуске
        SaveAdaptiveWeights(...) — для дипломной демонстрации: показать как веса эволюционируют

    [HUMAN] Проверить что SQLiteCore доступен, протестировать создание БД

    [HUMAN] Протестировать сохранение и загрузку

Валидация:

    БД создаётся в Saved/
    Данные сохраняются и загружаются корректно
    Git commit: feat(data): implement SQLite save/load system for metaprogression

STAGE 11: Полировка и подготовка к защите

Цель: Финальная сборка, устранение багов, подготовка демонстрации.
Действия:

    [AI] Ревью всего кода на предмет:
        Memory leaks (все UObject* под UPROPERTY)
        Missing null checks
        Thread safety (не актуально для single-thread, но проверить)
        Logging completeness

    [AI] Создать UDemonstrationSubsystem:
        Режим демонстрации для защиты
        Замедление времени для показа принятия решений
        Пошаговый режим: одна оценка за клик
        Экспорт данных в CSV для графиков в дипломе

    [HUMAN] Финальное тестирование полного цикла

    [HUMAN] Сборка standalone build

    [HUMAN] Подготовка скриншотов и видеоматериалов

Валидация:

    Игра запускается как standalone
    Демонстрационный режим работает
    Debug widget читаем
    Git commit: release: prepare diploma demonstration build v1.0

ЧАСТЬ 5: GIT-СТРАТЕГИЯ, ОШИБКИ, РЕКОМЕНДАЦИИ
Git Commit Strategy

Формат: <type>(<scope>): <description>

Типы:

    init — начальная настройка
    setup — конфигурация
    structure — структурные изменения
    feat — новая функциональность
    fix — исправление бага
    refactor — рефакторинг без изменения поведения
    docs — документация
    test — тесты
    release — сборки

Примеры:

text

init: create empty UE5 C++ project
setup: enable Paper2D, Niagara, SQLiteCore, EnhancedInput plugins
structure: create project folder hierarchy and base utility files
feat(player): implement character movement with Enhanced Input
feat(player): implement dash with cooldown and action tracking
feat(combat): implement ranged and melee weapons with projectiles
feat(enemy): implement base enemy with health component
feat(fsm): implement abstract FSMState base class
feat(fsm): implement FSM component with state registration and transitions
feat(fsm): implement Idle, Chase, Attack states
feat(fsm): implement Retreat and Flank states
feat(adaptive): define core types, structs, and config data asset
feat(adaptive): implement math utility functions (sigmoid, bell, decay)
feat(adaptive): implement Layer 1 - context evaluator
feat(adaptive): implement time-decay memory system
feat(adaptive): implement N-gram pattern recognition
feat(adaptive): implement Layer 2 - threat calculator with confidence
feat(adaptive): implement dynamic weight adjustment via feedback
feat(adaptive): implement utility curves for state evaluation
feat(adaptive): implement transition cost matrix and inertia
feat(adaptive): implement Layer 3 - state transition resolver
feat(adaptive): implement adaptive thresholds
feat(adaptive): implement AdaptiveBehaviorComponent orchestrator
feat(enemy): implement adaptive enemy integrating FSM and adaptive module
feat(ui): implement debug overlay for adaptive behavior visualization
feat(rooms): implement procedural room generation
feat(ui): implement HUD and menus
feat(data): implement SQLite save/load system
fix(adaptive): correct weight clamping boundary condition
refactor(fsm): extract common movement logic to base state
release: diploma demonstration build v1.0

Правило для дипломного авторства:

    Коммиты делать ежедневно (или при каждом значимом изменении)
    Каждый коммит — одно логическое изменение
    Никогда не коммитить всё одним «initial commit»
    В истории должно быть 30-50+ коммитов для доказательства поэтапной работы
    Использовать git log --oneline --graph для демонстрации на защите

Branches

text

main           — стабильная версия
develop        — основная ветка разработки
feature/fsm    — FSM модуль
feature/adaptive — адаптивный модуль
feature/rooms  — процедурная генерация

Типичные архитектурные ошибки, ослабляющие дипломную защиту
Ошибка	Почему ослабляет	Как избежать
Логика поведения внутри Enemy Actor	Нарушает разделение ответственности, модуль «растворяется» в игре	Enemy Actor только вызывает FSM. Вся логика — в FSM и Adaptive модулях
FSM на Blueprint	Не демонстрирует C++ навыки, комиссия посчитает это визуальным скриптингом	100% C++ для FSM и Adaptive
Behavior Tree	Встроенный механизм UE, нет кастомной реализации	Запрещено в ТЗ
Статические веса без обратной связи	Система не адаптивна, только реактивна	Обязательно: DynamicWeightManager
Нет визуализации работы модуля	Комиссия не увидит как модуль принимает решения	Debug widget обязателен
Один тип врага	Не показана переиспользуемость FSM	3 типа: melee, ranged, adaptive
Нет нормализации входов	Формула математически некорректна	Нормализация к [0,1] обязательна
Нет логирования	Невозможно отладить и продемонстрировать	Подробные UE_LOG для каждого этапа
Процедурная генерация раньше FSM	Ставит геймплей выше дипломного модуля	Порядок: FSM → Adaptive → Rooms
Нет Data Asset для конфигурации	Хардкод констант в коде — непрофессионально	UAdaptiveConfig как UDataAsset
Слишком частый Tick в адаптивном модуле	Тратит CPU, не нужен каждый кадр	Timer 0.2-0.5 сек
Огромный единый класс	Monolith, нарушает SRP	Разбить на UObject-подсистемы
Рекомендация по подготовке к защите

    Подготовить слайд с архитектурной диаграммой — показать 3-слойную модель
    Подготовить графики utility-кривых — для каждого состояния
    Подготовить видео боя с debug overlay — показать числа в реальном времени
    Подготовить CSV-экспорт данных — показать как веса менялись за 5 боёв
    Подготовить сравнение: простой враг vs адаптивный — наглядная разница
    Ключевая фраза для защиты: «Игровая среда — это полигон для демонстрации. Инженерный результат — программный модуль адаптивного поведения, реализующий 3-слойную модель оценки угроз с динамической корректировкой весов, временным затуханием памяти, распознаванием паттернов через N-грамм анализ и utility-based выбором состояний.»

Итоговый чеклист готовности к защите

    FSM полностью кастомный, без BehaviorTree
    Адаптивный модуль — отдельный архитектурный блок (не размазан по актору)
    Формула угрозы — нелинейная, с нормализацией, с кросс-терминами
    Динамические веса меняются в процессе боя (доказательство адаптации)
    Память с временным затуханием работает
    N-gram паттерны распознаются и влияют на поведение
    Utility curves для каждого состояния настроены и визуализированы
    Transition cost matrix и инерция предотвращают дёрганье
    Adaptive thresholds сдвигаются
    Confidence coefficient влияет на консервативность
    Debug widget показывает всё в реальном времени
    30+ коммитов в Git с осмысленными сообщениями
    CSV-экспорт данных для графиков в дипломе
    Standalone build работает
    Видеозапись демонстрации подготовлена
