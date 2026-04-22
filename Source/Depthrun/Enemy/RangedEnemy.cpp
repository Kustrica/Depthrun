// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "Enemy/RangedEnemy.h"
#include "Combat/BaseProjectile.h"
#include "Combat/RangedWeapon.h"
#include "Components/SphereComponent.h"
#include "Core/DepthrunLogChannels.h"
#include "Kismet/GameplayStatics.h"

ARangedEnemy::ARangedEnemy() {
  EnemyType = EEnemyType::Ranged;

  // Ranged enemies prefer to keep distance.
  AttackRange = 450.f;
  DetectionRange = 700.f;
  AttackCooldown = 2.0f;
  MoveSpeed = 250.f;
}

void ARangedEnemy::PerformMeleeAttack() {
  // ── Commercial Fix: If we have a dedicated weapon actor, use it (same as
  // player)
  if (IsValid(SpawnedWeapon)) {
    ACharacter *Player = UGameplayStatics::GetPlayerCharacter(this, 0);
    if (!IsValid(Player))
      return;

    const FVector FireDir =
        (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
    SpawnedWeapon->SetFireDirection(FireDir);
    SpawnedWeapon->Fire();
    return;
  }

  // Fallback to manual spawn if no weapon actor is assigned in BP
  if (!ProjectileClass)
    return;

  if (ShotDelay > 0.01f) {
    FTimerHandle Timer;
    GetWorldTimerManager().SetTimer(Timer, this, &ARangedEnemy::ActuallyFire,
                                    ShotDelay, false);
  } else {
    ActuallyFire();
  }
}

void ARangedEnemy::ActuallyFire() {
  ACharacter *Player = UGameplayStatics::GetPlayerCharacter(this, 0);
  if (!IsValid(Player))
    return;

  const FVector EnemyLoc = GetActorLocation();
  const FVector PlayerLoc = Player->GetActorLocation();
  const FVector FireDir = (PlayerLoc - EnemyLoc).GetSafeNormal2D();

  const FVector SpawnLoc =
      EnemyLoc + (FireDir * MuzzleOffset) + FVector(0.f, 0.f, 10.0f);
  const FTransform SpawnTransform = FTransform(FireDir.Rotation(), SpawnLoc);

  UWorld *World = GetWorld();
  if (!World)
    return;

  ABaseProjectile *Projectile = World->SpawnActorDeferred<ABaseProjectile>(
      ProjectileClass, SpawnTransform, this, Cast<APawn>(this),
      ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

  if (!IsValid(Projectile))
    return;

  Projectile->CollisionSphere->IgnoreActorWhenMoving(this, true);
  Projectile->InitProjectile(FireDir, AttackDamage, this, ProjectileSpeed);
  Projectile->FinishSpawning(SpawnTransform);
}
