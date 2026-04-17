// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoomTransitionVolume.generated.h"

class URoomGeneratorSubsystem;
class ARoomBase;
class UBoxComponent;

/**
 * ARoomTransitionVolume
 * Trigger volume placed at room exits.
 * When the player overlaps it, notifies URoomGeneratorSubsystem
 * to transition to the next room.
 * Implementation: Stage 8B.
 */
UCLASS()
class DEPTHRUN_API ARoomTransitionVolume : public AActor
{
	GENERATED_BODY()

public:
	ARoomTransitionVolume();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

public:
	/** The room this volume belongs to. Set by RoomGeneratorSubsystem on spawn. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Room")
	TObjectPtr<ARoomBase> OwnerRoom;

	/** Exit index within OwnerRoom's ExitPoints array. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Room")
	int32 ExitIndex = 0;

private:
	UPROPERTY()
	TObjectPtr<UBoxComponent> TriggerBox;
};
