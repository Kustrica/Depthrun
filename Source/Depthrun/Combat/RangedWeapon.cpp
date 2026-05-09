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
  UE_LOG(LogCombat, Log, TEXT("[RangedWeapon] %s ActuallyFire START (ShotsPerFire=%d)"), *GetName(), ShotsPerFire);

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

  const FVector BaseSpawnLocation =
      (CharacterOwner ? CharacterOwner->GetActorLocation()
                      : GetActorLocation()) +
      FireDirection.GetSafeNormal() * MuzzleOffsetDist + FVector(0.f, 0.f, 10.0f);

  const FRotator BaseRotation = FireDirection.GetSafeNormal().IsNearlyZero()
                                     ? FRotator::ZeroRotator
                                     : FireDirection.GetSafeNormal().Rotation();

  // === Multishot: spawn multiple projectiles with angular spread ===
  const int32 NumShots = FMath::Clamp(ShotsPerFire, 1, 10); // safety cap
  const float SpreadStep = 14.0f; // degrees between shots (±7° for 2 shots, etc.)
  const float StartAngle = -SpreadStep * (NumShots - 1) * 0.5f; // center the spread

  for (int32 i = 0; i < NumShots; ++i)
  {
    const float AngleOffset = StartAngle + (i * SpreadStep);
    const FVector SpreadDirection = FireDirection.GetSafeNormal().RotateAngleAxis(AngleOffset, FVector(0.f, 0.f, 1.f));
    const FVector SpawnLocation = BaseSpawnLocation;
    const FRotator SpawnRotation = SpreadDirection.IsNearlyZero()
                                       ? FRotator::ZeroRotator
                                       : SpreadDirection.Rotation();
    const FTransform SpawnTransform(SpawnRotation, SpawnLocation);

    // === SpawnActorDeferred: configure BEFORE BeginPlay & first overlap ===
    const double RW_T1 = FPlatformTime::Seconds();
    ABaseProjectile *Projectile = World->SpawnActorDeferred<ABaseProjectile>(
        ProjectileClass, SpawnTransform,
        this,                        // Owner = weapon
        Cast<APawn>(CharacterOwner), // Instigator = character
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
    const double RW_T2 = FPlatformTime::Seconds();
    UE_LOG(LogCombat, Verbose, TEXT("[RangedWeapon] SpawnActorDeferred %.4f ms"), (RW_T2 - RW_T1) * 1000.0);

    if (!IsValid(Projectile))
      continue;

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

    Projectile->InitProjectile(SpreadDirection, BaseDamage, CharacterOwner,
                               ProjectileSpeed, bPierceEnabled, RicochetCount);
    Projectile->FinishSpawning(SpawnTransform);
  }

  const double RW_T3 = FPlatformTime::Seconds();
  UE_LOG(LogCombat, Log, TEXT("[RangedWeapon] FinishSpawning %.4f ms | total %.4f ms (spawned %d projectiles)"),
         (RW_T3 - RW_T0) * 1000.0, (RW_T3 - RW_T0) * 1000.0, NumShots);
}

void ARangedWeapon::ResetEffects() {
  RicochetCount = 0;
  bPierceEnabled = false;
  // Do NOT reset ShotsPerFire — hub upgrades (BaseShotsPerFire) must persist
  // across item re-applies. Items add on top of the hub base, so we only
  // reset item-specific effects here.
}
