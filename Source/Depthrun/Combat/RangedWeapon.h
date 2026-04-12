// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "BaseWeapon.h"
#include "RangedWeapon.generated.h"

class ABaseProjectile;

/**
 * ARangedWeapon
 * Spawns ABaseProjectile in FireDirection on Fire(). 
 * Direction is set by ADepthrunCharacter before calling Attack().
 */
UCLASS()
class DEPTHRUN_API ARangedWeapon : public ABaseWeapon
{
	GENERATED_BODY()

public:
	ARangedWeapon();

	virtual void Fire() override;
	virtual EWeaponType GetWeaponType() const override { return EWeaponType::Ranged; }

	/** Projectile blueprint class — assign in Editor (Blueprint subclass of ABaseProjectile). */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Ranged")
	TSubclassOf<ABaseProjectile> ProjectileClass;
};
