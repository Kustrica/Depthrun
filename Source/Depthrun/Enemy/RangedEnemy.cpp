// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "Enemy/RangedEnemy.h"
#include "Combat/BaseProjectile.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Core/DepthrunLogChannels.h"

ARangedEnemy::ARangedEnemy()
{
	EnemyType = EEnemyType::Ranged;

	// Ranged enemies prefer to keep distance.
	AttackRange    = 450.f;
	DetectionRange = 700.f;
	AttackCooldown = 2.0f;
	MoveSpeed      = 250.f;
}

void ARangedEnemy::PerformMeleeAttack()
{
	// For ranged enemies this fires a projectile, NOT a melee hit.
	if (!ProjectileClass) return;

	// Use ShotDelay if configured to sync with animation
	if (ShotDelay > 0.01f)
	{
		FTimerHandle Timer;
		GetWorldTimerManager().SetTimer(Timer, this, &ARangedEnemy::ActuallyFire, ShotDelay, false);
	}
	else
	{
		ActuallyFire();
	}
}

void ARangedEnemy::ActuallyFire()
{
	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(this, 0);
	if (!IsValid(Player)) return;

	const FVector EnemyLoc  = GetActorLocation();
	const FVector PlayerLoc = Player->GetActorLocation();
	const FVector FireDir   = (PlayerLoc - EnemyLoc).GetSafeNormal();

	// Spawn slightly ahead + Z-offset for visibility
	const FVector    SpawnLoc       = EnemyLoc + (FireDir * MuzzleOffset) + FVector(0.f, 0.f, 2.0f);
	const FTransform SpawnTransform = FTransform(FireDir.Rotation(), SpawnLoc);

	UWorld* World = GetWorld();
	if (!World) return;

	ABaseProjectile* Projectile = World->SpawnActorDeferred<ABaseProjectile>(
		ProjectileClass,
		SpawnTransform,
		this,               // Owner = this enemy
		Cast<APawn>(this),  // Instigator = this enemy pawn
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn
	);

	if (!IsValid(Projectile)) return;

	// Prevent projectile from immediately overlapping with our own capsule.
	Projectile->CollisionSphere->IgnoreActorWhenMoving(this, true);

	// AttackDamage from BaseEnemy config, ShooterActor = this (OnOverlap guard uses it).
	Projectile->InitProjectile(FireDir, AttackDamage, this);
	Projectile->FinishSpawning(SpawnTransform);

	UE_LOG(LogCombat, Log,
		TEXT("[RangedEnemy] %s → fired at player dir=(%.2f,%.2f) dmg=%.1f"),
		*GetName(), FireDir.X, FireDir.Y, AttackDamage);
}
