// Copyright Depthrun Project, 2026. All Rights Reserved.

#include "RangedWeapon.h"
#include "BaseProjectile.h"
#include "Core/DepthrunLogChannels.h"

ARangedWeapon::ARangedWeapon()
{
}

void ARangedWeapon::Fire()
{
	if (!bCanFire || !ProjectileClass) return;

	bCanFire = false;

	if (!GetWorld()) return;

	FActorSpawnParameters Params;
	Params.Instigator = Cast<APawn>(GetOwner());
	Params.Owner      = this;

	const FVector SpawnLocation = GetOwner() ? GetOwner()->GetActorLocation() : GetActorLocation();

	ABaseProjectile* Projectile = GetWorld()->SpawnActor<ABaseProjectile>(
		ProjectileClass,
		SpawnLocation,
		FRotator::ZeroRotator,
		Params);

	if (IsValid(Projectile))
	{
		// FireDirection was set by ADepthrunCharacter before calling Attack()
		Projectile->InitProjectile(FireDirection, BaseDamage, GetOwner());
		UE_LOG(LogCombat, Log, TEXT("ARangedWeapon::Fire — projectile spawned dir=(%.1f,%.1f,%.1f)"),
			FireDirection.X, FireDirection.Y, FireDirection.Z);
	}

	StartCooldown();
}
