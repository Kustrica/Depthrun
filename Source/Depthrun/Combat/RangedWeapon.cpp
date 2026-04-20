// Copyright Depthrun Project, 2026. All Rights Reserved.

#include "RangedWeapon.h"
#include "BaseProjectile.h"
#include "Core/DepthrunLogChannels.h"
#include "Components/SphereComponent.h"

ARangedWeapon::ARangedWeapon()
{
	AttackCooldown = 0.5f;
	BaseDamage = 10.f;
	RicochetCount = 0;
	bPierceEnabled = false;
}

void ARangedWeapon::Fire()
{
	if (!bCanFire || !ProjectileClass) return;

	bCanFire = false;
	StartCooldown();

	// Delay the actual spawn to match animation
	if (ShotDelay > 0.01f)
	{
		GetWorld()->GetTimerManager().SetTimer(ShotTimer, this, &ARangedWeapon::ActuallyFire, ShotDelay, false);
	}
	else
	{
		ActuallyFire();
	}
}

void ARangedWeapon::ActuallyFire()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// Character that owns this weapon
	AActor* CharacterOwner = GetOwner();

	// Spawn slightly ahead in FireDirection to clear the character's capsule.
	// ADDED: Z+2.0f offset to prevent Z-fighting or floor-level invisibility.
	const FVector SpawnLocation = (CharacterOwner ? CharacterOwner->GetActorLocation() : GetActorLocation())
	                              + FireDirection.GetSafeNormal() * 30.f
	                              + FVector(0.f, 0.f, 2.0f);

	const FRotator SpawnRotation = FireDirection.GetSafeNormal().IsNearlyZero()
	                               ? FRotator::ZeroRotator
	                               : FireDirection.GetSafeNormal().Rotation();
	const FTransform SpawnTransform(SpawnRotation, SpawnLocation);

	// === SpawnActorDeferred: configure BEFORE BeginPlay & first overlap ===
	ABaseProjectile* Projectile = World->SpawnActorDeferred<ABaseProjectile>(
		ProjectileClass,
		SpawnTransform,
		this,                                           // Owner = weapon
		Cast<APawn>(CharacterOwner),                    // Instigator = character
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn
	);

	if (!IsValid(Projectile)) return;

	if (CharacterOwner)
	{
		Projectile->CollisionSphere->IgnoreActorWhenMoving(CharacterOwner, true);
	}
	Projectile->CollisionSphere->IgnoreActorWhenMoving(this, true);

	Projectile->InitProjectile(FireDirection, BaseDamage, CharacterOwner, bPierceEnabled, RicochetCount);
	Projectile->FinishSpawning(SpawnTransform);
}

void ARangedWeapon::ResetEffects()
{
	RicochetCount = 0;
	bPierceEnabled = false;
}
