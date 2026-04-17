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

void ABaseProjectile::InitProjectile(const FVector& Direction, float Damage, AActor* Shooter, bool bPierce, int32 InRicochetCount)
{
	DamageAmount = Damage;
	ShooterActor = Shooter;
	bPierceEnabled = bPierce;
	RicochetCount = InRicochetCount;

	if (ProjectileMovement)
	{
		ProjectileMovement->Velocity = Direction.GetSafeNormal() * ProjectileSpeed;
		
		// If ricochet is enabled, we need to allow bounce
		if (RicochetCount > 0)
		{
			ProjectileMovement->bShouldBounce = true;
			ProjectileMovement->Bounciness = 1.0f;
			ProjectileMovement->Friction = 0.0f;
		}
	}
}

void ABaseProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
                             UPrimitiveComponent* OtherComp, FVector NormalImpulse,
                             const FHitResult& Hit)
{
	if (RicochetCount > 0)
	{
		RicochetCount--;
		UE_LOG(LogCombat, Verbose, TEXT("Projectile ricocheted! Left: %d"), RicochetCount);
		// If this was the last ricochet, disable further bouncing
		if (RicochetCount <= 0 && ProjectileMovement)
		{
			ProjectileMovement->bShouldBounce = false;
		}
		return;
	}

	// Always treat wall hits as destruction if no ricochets left
	Destroy();
}

void ABaseProjectile::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                 bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor) || OtherActor == ShooterActor || HitActors.Contains(OtherActor)) return;

	UE_LOG(LogCombat, Log, TEXT("ABaseProjectile::OnOverlap → %s"), *OtherActor->GetName());

	OtherActor->TakeDamage(DamageAmount, FDamageEvent(), nullptr, ShooterActor);
	HitActors.Add(OtherActor);

	if (!bPierceEnabled)
	{
		Destroy();
	}
}
