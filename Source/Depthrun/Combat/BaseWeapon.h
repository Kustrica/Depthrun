// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseWeapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Melee  UMETA(DisplayName = "Melee"),
	Ranged UMETA(DisplayName = "Ranged")
};

/**
 * ABaseWeapon
 * Abstract base for all weapons. Subclasses implement Fire().
 * Fire direction is set externally by the owner (ADepthrunCharacter)
 * via SetFireDirection() — no dependency on character type here.
 */
UCLASS(Abstract)
class DEPTHRUN_API ABaseWeapon : public AActor
{
	GENERATED_BODY()

public:
	ABaseWeapon();

	/** Subclasses implement the actual fire/swing logic. */
	virtual void Fire() PURE_VIRTUAL(ABaseWeapon::Fire, );

	virtual float       GetDamage()     const;
	virtual EWeaponType GetWeaponType() const PURE_VIRTUAL(ABaseWeapon::GetWeaponType, return EWeaponType::Melee;);

	/** Set before calling Fire(). Owner provides world-space direction. */
	void SetFireDirection(const FVector& Direction) { FireDirection = Direction; }

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float BaseDamage = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float AttackCooldown = 0.5f;

protected:
	bool    bCanFire      = true;
	FVector FireDirection = FVector(1.f, 0.f, 0.f);
	FTimerHandle CooldownTimer;

	/** Call this at the end of Fire() in subclasses to start the attack cooldown. */
	void StartCooldown();

private:
	void OnCooldownEnd();
};
