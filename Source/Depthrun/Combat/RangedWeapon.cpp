// Copyright Depthrun Project, 2026. All Rights Reserved.

#include "RangedWeapon.h"
#include "BaseProjectile.h"
#include "Components/SphereComponent.h"
#include "Core/DepthrunLogChannels.h"
#include "Player/DepthrunCharacter.h"

ARangedWeapon::ARangedWeapon() {
  AttackCooldown = 0.5f;
  BaseDamage = 10.f;
  RicochetCount = 0;
  bPierceEnabled = false;
}

void ARangedWeapon::Fire() {
  if (!bCanFire || !ProjectileClass)
    return;

  bCanFire = false;
  StartCooldown();

  // Delay the actual spawn to match animation
  if (ShotDelay > 0.01f) {
    GetWorld()->GetTimerManager().SetTimer(
        ShotTimer, this, &ARangedWeapon::ActuallyFire, ShotDelay, false);
  } else {
    ActuallyFire();
  }
}

void ARangedWeapon::ActuallyFire() {
  const double RW_T0 = FPlatformTime::Seconds();
  UE_LOG(LogCombat, Log, TEXT("[RangedWeapon] %s ActuallyFire START"), *GetName());

  UWorld *World = GetWorld();
  if (!World)
    return;

  // Character that owns this weapon
  AActor *CharacterOwner = GetOwner();

  // Spawn ahead in FireDirection. Read MuzzleOffset from player character if available,
  // fallback to 40.f for enemy-owned weapons (large capsule clearance).
  float MuzzleOffsetDist = 40.f;
  if (ADepthrunCharacter* PlayerChar = Cast<ADepthrunCharacter>(CharacterOwner))
    MuzzleOffsetDist = PlayerChar->MuzzleOffset;

  const FVector SpawnLocation =
      (CharacterOwner ? CharacterOwner->GetActorLocation()
                      : GetActorLocation()) +
      FireDirection.GetSafeNormal() * MuzzleOffsetDist + FVector(0.f, 0.f, 10.0f);

  const FRotator SpawnRotation = FireDirection.GetSafeNormal().IsNearlyZero()
                                     ? FRotator::ZeroRotator
                                     : FireDirection.GetSafeNormal().Rotation();
  const FTransform SpawnTransform(SpawnRotation, SpawnLocation);

  // === SpawnActorDeferred: configure BEFORE BeginPlay & first overlap ===
  const double RW_T1 = FPlatformTime::Seconds();
  ABaseProjectile *Projectile = World->SpawnActorDeferred<ABaseProjectile>(
      ProjectileClass, SpawnTransform,
      this,                        // Owner = weapon
      Cast<APawn>(CharacterOwner), // Instigator = character
      ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
  const double RW_T2 = FPlatformTime::Seconds();
  UE_LOG(LogCombat, Log, TEXT("[RangedWeapon] SpawnActorDeferred %.4f ms"), (RW_T2 - RW_T1) * 1000.0);

  if (!IsValid(Projectile))
    return;

  if (CharacterOwner) {
    Projectile->CollisionSphere->IgnoreActorWhenMoving(CharacterOwner, true);

    // Commercial Fix: Explicitly ignore all primitives of the owner during
    // movement sweep. IgnoreActorWhenMoving sometimes fails for bSweep=true if
    // the components are Block-enabled.
    TArray<UPrimitiveComponent *> OwnerComponents;
    CharacterOwner->GetComponents<UPrimitiveComponent>(OwnerComponents);
    for (UPrimitiveComponent *Comp : OwnerComponents) {
      if (Comp) {
        Projectile->CollisionSphere->IgnoreComponentWhenMoving(Comp, true);
      }
    }
  }
  Projectile->CollisionSphere->IgnoreActorWhenMoving(this, true);

  Projectile->InitProjectile(FireDirection, BaseDamage, CharacterOwner,
                             ProjectileSpeed, bPierceEnabled, RicochetCount);
  Projectile->FinishSpawning(SpawnTransform);

  const double RW_T3 = FPlatformTime::Seconds();
  UE_LOG(LogCombat, Log, TEXT("[RangedWeapon] FinishSpawning %.4f ms | total %.4f ms"),
         (RW_T3 - RW_T2) * 1000.0, (RW_T3 - RW_T0) * 1000.0);
}

void ARangedWeapon::ResetEffects() {
  RicochetCount = 0;
  bPierceEnabled = false;
}
