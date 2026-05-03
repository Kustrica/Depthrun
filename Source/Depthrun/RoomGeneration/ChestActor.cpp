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
    CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AChestActor::OnChestOverlap);
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

    // ── 1. Diamonds ──────────────────────────────────────────────────────────
    const int32 Diamonds = LootConfig ? LootConfig->RollDiamonds() : FMath::RandRange(5, 15);
    if (Economy)
    {
        Economy->AddDiamonds(Diamonds);
    }
    UE_LOG(LogDepthrunLoot, Log, TEXT("[Chest] Opened — Diamonds: +%d"), Diamonds);

    // ── 2. Potions ───────────────────────────────────────────────────────────
    const int32 Potions = LootConfig ? LootConfig->RollPotions() : 1;
    if (Economy && Potions > 0)
    {
        Economy->AddPotions(Potions);
    }
    UE_LOG(LogDepthrunLoot, Log, TEXT("[Chest] Opened — Potions: +%d"), Potions);

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
                // Apply weapon effects immediately
                if (ABaseWeapon* W1 = Player->GetWeaponSlot(1)) Inventory->ApplyToWeapon(W1);
                if (ABaseWeapon* W2 = Player->GetWeaponSlot(2)) Inventory->ApplyToWeapon(W2);
                Inventory->ApplyToCharacter(Player);
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
}
