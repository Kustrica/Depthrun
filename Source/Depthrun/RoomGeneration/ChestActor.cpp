// Copyright Depthrun Project, 2026. All Rights Reserved.

#include "ChestActor.h"
#include "Combat/ChestLootConfig.h"
#include "Items/RunItemCollection.h"
#include "Items/RunItemInventory.h"
#include "Player/DepthrunCharacter.h"
#include "Player/PlayerEconomy.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"
#include "PaperSpriteComponent.h"
#include "Core/DepthrunLogChannels.h"
#include "Math/UnrealMathUtility.h"

AChestActor::AChestActor()
{
    PrimaryActorTick.bCanEverTick = false;

    CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
    RootComponent = CollisionBox;
    CollisionBox->SetBoxExtent(FVector(8.f, 8.f, 20.f));
    CollisionBox->SetCollisionProfileName(TEXT("OverlapAll"));
    CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollisionBox->SetGenerateOverlapEvents(true);

    SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("SpriteComponent"));
    SpriteComponent->SetupAttachment(RootComponent);
    SpriteComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SpriteComponent->SetTranslucentSortPriority(20);
}

void AChestActor::BeginPlay()
{
    Super::BeginPlay();

    // Disable overlap detection initially; enable after a short delay.
    // This prevents the chest from being instantly opened when it spawns
    // directly on top of the player (e.g., room clears while player stands there).
    CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AChestActor::OnChestOverlap);

    GetWorldTimerManager().SetTimer(TimerHandle_Activate, this,
        &AChestActor::EnableOverlap, ActivationDelay, false);
}

void AChestActor::EnableOverlap()
{
    CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

    // Manually check if the player is already standing inside the box
    // (BeginOverlap won't fire for actors that were overlapping before collision was enabled)
    TArray<AActor*> Overlapping;
    CollisionBox->GetOverlappingActors(Overlapping, ADepthrunCharacter::StaticClass());
    for (AActor* Actor : Overlapping)
    {
        if (ADepthrunCharacter* Player = Cast<ADepthrunCharacter>(Actor))
        {
            if (Player->IsPlayerControlled() && !bOpened)
            {
                bOpened = true;
                DistributeLoot(Player);
                return;
            }
        }
    }
}

void AChestActor::OnChestOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                  bool bFromSweep, const FHitResult& SweepResult)
{
    if (bOpened || !OtherActor) return;

    ADepthrunCharacter* Player = Cast<ADepthrunCharacter>(OtherActor);
    if (!Player || !Player->IsPlayerControlled()) return;

    bOpened = true;
    DistributeLoot(Player);
}

void AChestActor::DistributeLoot(ADepthrunCharacter* Player)
{
    if (!Player) return;

    UPlayerEconomy* Economy = Player->FindComponentByClass<UPlayerEconomy>();
    URunItemInventory* Inventory = Player->FindComponentByClass<URunItemInventory>();

    FChestRewardPayload Payload;
    Payload.DiamondIcon = LootConfig ? LootConfig->DiamondIcon : nullptr;
    Payload.PotionIcon  = LootConfig ? LootConfig->PotionIcon  : nullptr;

    // ── 1. Diamonds ──────────────────────────────────────────────────────────
    Payload.Diamonds = LootConfig ? LootConfig->RollDiamonds() : FMath::RandRange(5, 15);
    if (Economy) Economy->AddDiamonds(Payload.Diamonds);
    UE_LOG(LogDepthrunLoot, Log, TEXT("[Chest] Opened — Diamonds: +%d"), Payload.Diamonds);

    // ── 2. Potions ───────────────────────────────────────────────────────────
    Payload.Potions = LootConfig ? LootConfig->RollPotions() : 1;
    if (Economy && Payload.Potions > 0) Economy->AddPotions(Payload.Potions);
    UE_LOG(LogDepthrunLoot, Log, TEXT("[Chest] Opened — Potions: +%d"), Payload.Potions);

    // ── 3. Run Item (from pool) ───────────────────────────────────────────────
    const float ItemChance = LootConfig ? LootConfig->ItemDropChance : 0.7f;
    if (ItemCollection && ItemCollection->Items.Num() > 0 && FMath::FRand() <= ItemChance)
    {
        const int32 Idx = FMath::RandRange(0, ItemCollection->Items.Num() - 1);
        const FRunItemData& Item = ItemCollection->Items[Idx];

        if (Inventory)
        {
            const bool bAdded = Inventory->AddItem(Item);
            if (bAdded)
            {
                if (ABaseWeapon* W1 = Player->GetWeaponSlot(1)) Inventory->ApplyToWeapon(W1);
                if (ABaseWeapon* W2 = Player->GetWeaponSlot(2)) Inventory->ApplyToWeapon(W2);
                Inventory->ApplyToCharacter(Player);
                Payload.ItemName = Item.ItemName;
                Payload.ItemIcon = Item.ItemIcon;
                UE_LOG(LogDepthrunLoot, Log, TEXT("[Chest] Item: %s"), *Item.ItemName);
            }
            else
            {
                UE_LOG(LogDepthrunLoot, Log, TEXT("[Chest] Item roll '%s' skipped — inventory full"), *Item.ItemName);
            }
        }
    }
    else
    {
        UE_LOG(LogDepthrunLoot, Log, TEXT("[Chest] Item: no drop this time (chance=%.0f%%)"), ItemChance * 100.f);
    }

    // ── 4. Broadcast for HUD reward popup ────────────────────────────────────
    OnChestOpened.Broadcast(Payload);
}
