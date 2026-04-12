// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class UPaperSpriteComponent;

/**
 * ABaseProjectile
 * Spawned by ARangedWeapon. Flies in given direction, deals damage on hit, then destroys itself.
 * Uses UProjectileMovementComponent with zero gravity (top-down 2D).
 */
UCLASS()
class DEPTHRUN_API ABaseProjectile : public AActor
{
	GENERATED_BODY()

public:
	ABaseProjectile();

	/** Called by ARangedWeapon immediately after spawning. */
	void InitProjectile(const FVector& Direction, float Damage, AActor* Shooter);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
	           UPrimitiveComponent* OtherComp, FVector NormalImpulse,
	           const FHitResult& Hit);

	/** Fallback: fires when the projectile overlaps another actor (no blocking hit needed). */
	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	               UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	               bool bFromSweep, const FHitResult& SweepResult);

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<UPaperSpriteComponent> SpriteComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	float ProjectileSpeed = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	float ProjectileLifeSpan = 3.f;

private:
	float DamageAmount = 10.f;

	UPROPERTY()
	TObjectPtr<AActor> ShooterActor;
};
