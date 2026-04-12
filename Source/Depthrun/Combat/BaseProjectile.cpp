// Copyright Depthrun Project, 2026. All Rights Reserved.

#include "BaseProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "PaperSpriteComponent.h"
#include "Engine/DamageEvents.h"
#include "Core/DepthrunLogChannels.h"

ABaseProjectile::ABaseProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->SetSphereRadius(8.f);

	// Use BlockAllDynamic — always exists in UE. We generate Hit events + Overlap events.
	CollisionSphere->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetGenerateOverlapEvents(true);
	RootComponent = CollisionSphere;

	SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("Sprite"));
	SpriteComponent->SetupAttachment(RootComponent);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed            = ProjectileSpeed;
	ProjectileMovement->MaxSpeed                = ProjectileSpeed;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale   = 0.f; // top-down: no gravity
}

void ABaseProjectile::BeginPlay()
{
	Super::BeginPlay();
	// OnComponentHit fires when the collision profile generates blocking hits
	CollisionSphere->OnComponentHit.AddDynamic(this, &ABaseProjectile::OnHit);
	// OnComponentBeginOverlap fires even without a blocking hit (fallback)
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &ABaseProjectile::OnOverlap);
	SetLifeSpan(ProjectileLifeSpan);
}

void ABaseProjectile::InitProjectile(const FVector& Direction, float Damage, AActor* Shooter)
{
	DamageAmount  = Damage;
	ShooterActor  = Shooter;
	ProjectileMovement->Velocity = Direction.GetSafeNormal() * ProjectileSpeed;
}

void ABaseProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
                            UPrimitiveComponent* OtherComp, FVector NormalImpulse,
                            const FHitResult& Hit)
{
	if (!IsValid(OtherActor) || OtherActor == ShooterActor)
	{
		return;
	}

	UE_LOG(LogCombat, Log, TEXT("ABaseProjectile::OnHit → %s  damage=%.1f"),
		*OtherActor->GetName(), DamageAmount);

	OtherActor->TakeDamage(DamageAmount, FDamageEvent(), nullptr, ShooterActor);
	Destroy();
}

void ABaseProjectile::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                bool bFromSweep, const FHitResult& SweepResult)
{
	// Fallback path: used when BlockAllDynamic doesn't produce a Hit event
	// Guard against self-overlap and shooter
	if (!IsValid(OtherActor) || OtherActor == ShooterActor || OtherActor == this)
	{
		return;
	}

	UE_LOG(LogCombat, Log, TEXT("ABaseProjectile::OnOverlap → %s  damage=%.1f"),
		*OtherActor->GetName(), DamageAmount);

	OtherActor->TakeDamage(DamageAmount, FDamageEvent(), nullptr, ShooterActor);
	Destroy();
}
