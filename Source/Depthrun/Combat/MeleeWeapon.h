// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "BaseWeapon.h"
#include "MeleeWeapon.generated.h"

class UBoxComponent;

/**
 * AMeleeWeapon
 * Activates a hit zone (UBoxComponent) for 0.2s on Fire().
 * Any overlapping enemy takes BaseDamage.
 */
UCLASS()
class DEPTHRUN_API AMeleeWeapon : public ABaseWeapon
{
	GENERATED_BODY()

public:
	AMeleeWeapon();

	virtual void Fire() override;
	virtual EWeaponType GetWeaponType() const override { return EWeaponType::Melee; }

	/** Size of the melee hit zone in front of the character. */
	UPROPERTY(EditAnywhere, Category = "Weapon|Melee")
	float HitZoneHalfExtent = 40.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Melee")
	TObjectPtr<UBoxComponent> HitZone;

protected:
	UFUNCTION()
	void OnHitZoneOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	                      bool bFromSweep, const FHitResult& SweepResult);

private:
	void ActivateHitZone();
	void DeactivateHitZone();
	FTimerHandle HitZoneTimer;
};
