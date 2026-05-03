// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "BaseProjectile.generated.h"

class USphereComponent;
class UPaperSpriteComponent;

/**
 * ABaseProjectile
 * Spawned by ARangedWeapon. Moves manually via Tick (no
 * ProjectileMovementComponent). Detects enemy overlaps for damage, stops at
 * WorldStatic (walls). PMC was removed because it fought Blueprint physics
 * settings and caused 1.5s freezes.
 */
UCLASS()
class DEPTHRUN_API ABaseProjectile : public AActor {
  GENERATED_BODY()

public:
  ABaseProjectile();

  /** Called by ARangedWeapon immediately after SpawnActorDeferred. */
  void InitProjectile(const FVector &Direction, float Damage, AActor *Shooter,
                      float InSpeed = 800.f, bool bPierce = false,
                      int32 InRicochetCount = 0);

protected:
  virtual void BeginPlay() override;
  virtual void Tick(float DeltaTime) override;

  /** Fires when the projectile overlaps a Pawn (enemies). */
  UFUNCTION()
  void OnOverlap(UPrimitiveComponent *OverlappedComp, AActor *OtherActor,
                 UPrimitiveComponent *OtherComp, int32 OtherBodyIndex,
                 bool bFromSweep, const FHitResult &SweepResult);

public:
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
  TObjectPtr<USphereComponent> CollisionSphere;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
  TObjectPtr<UPaperSpriteComponent> SpriteComponent;

  /** Projectile speed in UU/s. Set in Blueprint for per-weapon tuning. */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
  float ProjectileSpeed = 800.f;

  /** Projectile lifetime in seconds before auto-destroy. */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
  float ProjectileLifeSpan = 3.f;

private:
  /** World-space normalized direction. Set by InitProjectile. */
  FVector LaunchDirection = FVector::ZeroVector;

  float DamageAmount = 10.f;
  bool bPierceEnabled = false;
  int32 RicochetCount = 0;

  /** Reference to the character that fired this projectile. */
  UPROPERTY()
  TObjectPtr<AActor> ShooterActor;

  /** Already-hit actors (prevents multi-hit on the same target in pierce mode).
   */
  UPROPERTY()
  TArray<TObjectPtr<AActor>> HitActors;

};
