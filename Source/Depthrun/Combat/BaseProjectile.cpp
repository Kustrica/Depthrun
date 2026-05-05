// Copyright Depthrun Project, 2026. All Rights Reserved.

#include "BaseProjectile.h"
#include "Components/SphereComponent.h"
#include "Core/DepthrunLogChannels.h"
#include "Engine/DamageEvents.h"
#include "PaperSpriteComponent.h"
#include "Enemy/BaseEnemy.h"
#include "Kismet/GameplayStatics.h"

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
    return;
  if (HitActors.Contains(OtherActor))
    return;

  // Friendly Fire Prevention: Enemies shouldn't hit other enemies
  if (ShooterActor && ShooterActor->IsA(ABaseEnemy::StaticClass()) && OtherActor->IsA(ABaseEnemy::StaticClass()))
    return;

  OtherActor->TakeDamage(DamageAmount, FDamageEvent(), nullptr, ShooterActor);
  HitActors.Add(OtherActor);

  // ── Ricochet: redirect to nearest other enemy ────────────────────────────
  if (RicochetCount > 0 && OtherActor->IsA(ABaseEnemy::StaticClass()))
  {
    TryRicochet(OtherActor);
    return; // don't destroy yet — TryRicochet decides
  }

  if (!bPierceEnabled) {
    Destroy();
  }
}

void ABaseProjectile::TryRicochet(AActor* JustHitEnemy)
{
  UWorld* World = GetWorld();
  if (!World) { Destroy(); return; }

  // Gather all living enemies within search radius
  TArray<AActor*> AllEnemies;
  UGameplayStatics::GetAllActorsOfClass(World, ABaseEnemy::StaticClass(), AllEnemies);

  AActor* BestTarget = nullptr;
  float BestDistSq = FLT_MAX;
  const FVector MyPos = GetActorLocation();

  for (AActor* Enemy : AllEnemies)
  {
    if (!IsValid(Enemy)) continue;
    if (Enemy == JustHitEnemy) continue;
    if (HitActors.Contains(Enemy)) continue; // already bounced through this one

    const float DistSq = FVector::DistSquared(MyPos, Enemy->GetActorLocation());
    if (DistSq < BestDistSq && FMath::Sqrt(DistSq) <= RicochetSearchRadius)
    {
      BestDistSq = DistSq;
      BestTarget = Enemy;
    }
  }

  if (!BestTarget)
  {
    // No valid target in range — projectile dies
    Destroy();
    return;
  }

  --RicochetCount;
  const FVector NewDir = (BestTarget->GetActorLocation() - MyPos).GetSafeNormal();
  LaunchDirection = NewDir;
  SetActorRotation(NewDir.Rotation());

  UE_LOG(LogCombat, Log, TEXT("[Projectile] Ricochet -> %s (remaining: %d)"),
    *BestTarget->GetName(), RicochetCount);
}
