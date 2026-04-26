// Copyright Depthrun Project, 2026. All Rights Reserved.

#include "BaseProjectile.h"
#include "Components/SphereComponent.h"
#include "Core/DepthrunLogChannels.h"
#include "Engine/DamageEvents.h"
#include "PaperSpriteComponent.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

ABaseProjectile::ABaseProjectile() {
  PrimaryActorTick.bCanEverTick = true; // Manual Tick enabled for movement

  CollisionSphere =
      CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
  CollisionSphere->SetSphereRadius(10.f);
  CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
  CollisionSphere->SetCollisionObjectType(ECC_WorldDynamic);
  CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
  CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
  CollisionSphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
  CollisionSphere->SetGenerateOverlapEvents(true);
  RootComponent = CollisionSphere;

  SpriteComponent =
      CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("Sprite"));
  SpriteComponent->SetupAttachment(RootComponent);
  SpriteComponent->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
  SpriteComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

  // PMC removed to fix 1.5s lag/freezes and physics conflicts in 2D
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────────────────────

void ABaseProjectile::BeginPlay() {
  Super::BeginPlay();

  CollisionSphere->OnComponentBeginOverlap.AddDynamic(
      this, &ABaseProjectile::OnOverlap);

  SetLifeSpan(ProjectileLifeSpan);
}

// ─────────────────────────────────────────────────────────────────────────────
// Tick — Manual Movement (Lag-free and 100% predictable)
// ─────────────────────────────────────────────────────────────────────────────

void ABaseProjectile::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  if (LaunchDirection.IsNearlyZero()) return;

  const FVector DeltaLocation = LaunchDirection * ProjectileSpeed * DeltaTime;
  
  FHitResult Hit;
  AddActorWorldOffset(DeltaLocation, true, &Hit);

  // If we hit something static (wall), destroy the projectile
  if (Hit.bBlockingHit)
  {
      Destroy();
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// InitProjectile — called by ARangedWeapon between SpawnActorDeferred /
// FinishSpawning
// ─────────────────────────────────────────────────────────────────────────────

void ABaseProjectile::InitProjectile(const FVector &Direction, float Damage,
                                     AActor *Shooter, float InSpeed,
                                     bool bPierce, int32 InRicochetCount) {
  LaunchDirection = Direction.GetSafeNormal();
  ProjectileSpeed = InSpeed;
  DamageAmount = Damage;
  ShooterActor = Shooter;
  bPierceEnabled = bPierce;
  RicochetCount = InRicochetCount;

  // Manual rotation of the actor to face movement direction
  if (!LaunchDirection.IsNearlyZero())
  {
      SetActorRotation(LaunchDirection.Rotation());
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// OnOverlap — damage on enemy contact
// ─────────────────────────────────────────────────────────────────────────────

#include "Enemy/BaseEnemy.h"

void ABaseProjectile::OnOverlap(UPrimitiveComponent *OverlappedComp,
                                 AActor *OtherActor,
                                 UPrimitiveComponent *OtherComp,
                                 int32 OtherBodyIndex, bool bFromSweep,
                                 const FHitResult &SweepResult) {
  if (!IsValid(OtherActor))
    return;
  if (OtherActor == this)
    return;
  if (OtherActor == ShooterActor)
    return;
  if (OtherActor == GetOwner())
    return; // skip weapon actor
  if (HitActors.Contains(OtherActor))
    return; // already hit (pierce mode guard)

  // Friendly Fire Prevention: Enemies shouldn't hit other enemies
  if (ShooterActor && ShooterActor->IsA(ABaseEnemy::StaticClass()) && OtherActor->IsA(ABaseEnemy::StaticClass()))
    return;

  OtherActor->TakeDamage(DamageAmount, FDamageEvent(), nullptr, ShooterActor);
  HitActors.Add(OtherActor);

  if (!bPierceEnabled) {
    Destroy();
  }
}
