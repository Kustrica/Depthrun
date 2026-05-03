// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChestActor.generated.h"

class UBoxComponent;
class UPaperSpriteComponent;
class UChestLootConfig;
class URunItemCollection;
class ADepthrunCharacter;

UCLASS()
class DEPTHRUN_API AChestActor : public AActor
{
    GENERATED_BODY()

public:
    AChestActor();

    /** Loot config asset. Assign DA_ChestLootConfig in Blueprint defaults. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest|Loot")
    TObjectPtr<UChestLootConfig> LootConfig;

    /** Item pool. Assign DA_RunItemCollection in Blueprint defaults. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest|Loot")
    TObjectPtr<URunItemCollection> ItemCollection;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UBoxComponent> CollisionBox;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UPaperSpriteComponent> SpriteComponent;

    UFUNCTION()
    void OnChestOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                        bool bFromSweep, const FHitResult& SweepResult);

private:
    bool bOpened = false;

    void DistributeLoot(ADepthrunCharacter* Player);
};
