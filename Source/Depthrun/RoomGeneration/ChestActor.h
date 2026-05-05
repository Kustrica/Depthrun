// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Texture2D.h"
#include "ChestActor.generated.h"

class UBoxComponent;
class UPaperSpriteComponent;
class UChestLootConfig;
class URunItemCollection;
class ADepthrunCharacter;

/** Payload broadcast after chest is opened — consumed by HUD to show reward popup. */
USTRUCT(BlueprintType)
struct FChestRewardPayload
{
	GENERATED_BODY()

	UPROPERTY() int32 Diamonds = 0;
	UPROPERTY() int32 Potions  = 0;
	UPROPERTY() FString ItemName;
	UPROPERTY() TObjectPtr<UTexture2D> DiamondIcon;
	UPROPERTY() TObjectPtr<UTexture2D> PotionIcon;
	UPROPERTY() TObjectPtr<UTexture2D> ItemIcon;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnChestOpened, const FChestRewardPayload&);

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

    /** Broadcast after loot is distributed. HUD subscribes to spawn reward popup. */
    FOnChestOpened OnChestOpened;

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

    /** Delay before collision is active — prevents instant open when spawned under player. */
    UPROPERTY(EditDefaultsOnly, Category = "Chest|Loot", meta = (ClampMin="0.0", ClampMax="2.0"))
    float ActivationDelay = 0.35f;

    FTimerHandle TimerHandle_Activate;
    void EnableOverlap();

    void DistributeLoot(ADepthrunCharacter* Player);
};
