// Copyright Depthrun Project, 2026. All Rights Reserved.

#include "RangedWeapon.h"
#include "BaseProjectile.h"
#include "Components/SphereComponent.h"
#include "Core/DepthrunLogChannels.h"

ARangedWeapon::ARangedWeapon() {
  AttackCooldown = 0.5f;
  BaseDamage = 10.f;
  RicochetCount = 0;
  bPierceEnabled = false;
}

void ARangedWeapon::Fire() {
  UE_LOG(LogCombat, Log,
         TEXT("[RangedWeapon] Fire called on %s. ProjectileClass=%s"),
         *GetNameSafe(GetOwner()),
         ProjectileClass ? *ProjectileClass->GetName() : TEXT("NULL"));

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
  UWorld *World = GetWorld();
  if (!World)
    return;

  // Character that owns this weapon
  AActor *CharacterOwner = GetOwner();

  // Spawn slightly ahead in FireDirection to clear the character's capsule.
  // ADDED: Z+10.0f offset to prevent Z-fighting or floor-level invisibility.
  // Commercial Fix: Increased offset to 60.f to ensure large enemy capsules
  // don't block the spawn sweep.
  const FVector SpawnLocation =
      (CharacterOwner ? CharacterOwner->GetActorLocation()
                      : GetActorLocation()) +
      FireDirection.GetSafeNormal() * 60.f + FVector(0.f, 0.f, 10.0f);

  const FRotator SpawnRotation = FireDirection.GetSafeNormal().IsNearlyZero()
                                     ? FRotator::ZeroRotator
                                     : FireDirection.GetSafeNormal().Rotation();
  const FTransform SpawnTransform(SpawnRotation, SpawnLocation);

  // === SpawnActorDeferred: configure BEFORE BeginPlay & first overlap ===
  ABaseProjectile *Projectile = World->SpawnActorDeferred<ABaseProjectile>(
      ProjectileClass, SpawnTransform,
      this,                        // Owner = weapon
      Cast<APawn>(CharacterOwner), // Instigator = character
      ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

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
}

void ARangedWeapon::ResetEffects() {
  RicochetCount = 0;
  bPierceEnabled = false;
}
