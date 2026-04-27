// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DoorActor.generated.h"

UCLASS()
class DEPTHRUN_API ADoorActor : public AActor
{
    GENERATED_BODY()

public:
    ADoorActor();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<class UBoxComponent> CollisionBox;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<class UPaperSpriteComponent> SpriteComponent;

    /** Initializes sprite and collision shape for the spawned door.
     *  Collision is ALWAYS handled by CollisionBox (thin Z-extent); sprite is visual-only. */
    void InitializeDoor(class UPaperSprite* DoorSprite, bool bVerticalDoor, const FRotator& SpriteRotation, float VisualScale);

    UFUNCTION(BlueprintCallable, Category = "Door")
    void OpenDoor();

    UFUNCTION(BlueprintCallable, Category = "Door")
    void CloseDoor();

protected:
    void SetDoorCollisionEnabled(bool bEnabled);

    UPROPERTY(BlueprintReadOnly, Category = "Door")
    bool bIsOpen = false;
};
