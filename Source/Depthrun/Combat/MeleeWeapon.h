// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "BaseWeapon.h"
#include "CoreMinimal.h"
#include "MeleeWeapon.generated.h"

class UBoxComponent;

/**
 * AMeleeWeapon
 * Activates a hit zone (UBoxComponent) for 0.2s on Fire().
 * Any overlapping enemy takes BaseDamage.
 */
UCLASS()
class DEPTHRUN_API AMeleeWeapon : public ABaseWeapon {
  GENERATED_BODY()

public:
  AMeleeWeapon();

  virtual void Fire() override;
  virtual EWeaponType GetWeaponType() const override {
    return EWeaponType::Melee;
  }

  /** Resets all item-applied modifiers to defaults. */
  void ResetEffects();

  void SetRangeMultiplier(float NewMultiplier) {
    RangeMultiplier = NewMultiplier;
  }
  float GetRangeMultiplier() const { return RangeMultiplier; }
  void SetDoubleSwing(bool bEnabled) { bDoubleSwing = bEnabled; }

  /** Reach of the weapon (distance forward). */
  UPROPERTY(EditAnywhere, Category = "Weapon|Melee")
  float HitZoneHalfExtent = 60.f;

  /** Width of the weapon swing (distance sideways). */
  UPROPERTY(EditAnywhere, Category = "Weapon|Melee")
  float HitZoneWidth = 80.f;

  /** Z-thickness of the hit zone to ensure it hits enemies in 2D. */
  UPROPERTY(EditAnywhere, Category = "Weapon|Melee")
  float HitZoneThickness = 32.f;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Melee")
  TObjectPtr<UBoxComponent> HitZone;

protected:
  UFUNCTION()
  void OnHitZoneOverlap(UPrimitiveComponent *OverlappedComp, AActor *OtherActor,
                        UPrimitiveComponent *OtherComp, int32 OtherBodyIndex,
                        bool bFromSweep, const FHitResult &SweepResult);

private:
  void ActivateHitZone();
  void DeactivateHitZone();

  /** Logic for second swing if DoubleSwing is enabled. */
  void PerformDoubleSwing();

  FTimerHandle HitZoneTimer;
  FTimerHandle DoubleSwingTimer;

  float RangeMultiplier = 1.0f;
  bool bDoubleSwing = false;
  bool bIsSecondSwing = false;
};
