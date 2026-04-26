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

    /** Настройка спрайта при спавне. */
    void InitializeDoor(class UPaperSprite* DoorSprite);

    UFUNCTION(BlueprintCallable, Category = "Door")
    void OpenDoor();

    UFUNCTION(BlueprintCallable, Category = "Door")
    void CloseDoor();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Door")
    bool bIsOpen = false;
};
