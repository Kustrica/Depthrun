// Copyright Depthrun Project, 2026. All Rights Reserved.

#include "MeleeWeapon.h"
#include "Components/BoxComponent.h"
#include "Core/DepthrunLogChannels.h"
#include "Engine/DamageEvents.h"

AMeleeWeapon::AMeleeWeapon() {
  HitZone = CreateDefaultSubobject<UBoxComponent>(TEXT("HitZone"));
  HitZone->SetupAttachment(RootComponent);
  HitZone->SetBoxExtent(
      FVector(HitZoneHalfExtent, HitZoneWidth, HitZoneThickness));
  HitZone->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  HitZone->SetCollisionResponseToAllChannels(ECR_Overlap);
  HitZone->SetCollisionObjectType(ECC_WorldDynamic);
  HitZone->SetGenerateOverlapEvents(true); // Explicitly enable
  HitZone->OnComponentBeginOverlap.AddDynamic(this,
                                              &AMeleeWeapon::OnHitZoneOverlap);

  AttackCooldown = 0.4f;
  BaseDamage = 35.f; // Increased default damage
  RangeMultiplier = 1.0f;
  WidthMultiplier = 1.0f;
  bDoubleSwing = false;
  bIsSecondSwing = false;
  bOmniSwing = false;
}

void AMeleeWeapon::ResetEffects() {
  RangeMultiplier = 1.0f;
  WidthMultiplier = 1.0f;
  ThicknessMultiplier = 1.0f;
  bDoubleSwing = false;
  bIsSecondSwing = false;
  bOmniSwing = false;
}

void AMeleeWeapon::Fire() {
  if (!bCanFire)
    return;

  bCanFire = false;
  ActivateHitZone();

  if (bDoubleSwing && !bIsSecondSwing) {
    GetWorldTimerManager().SetTimer(
        DoubleSwingTimer, this, &AMeleeWeapon::PerformDoubleSwing, 0.2f, false);
  } else {
    // Hit zone stays active 0.2s, then deactivates. Full cooldown follows.
    GetWorldTimerManager().SetTimer(
        HitZoneTimer, this, &AMeleeWeapon::DeactivateHitZone, 0.2f, false);
    StartCooldown();

    UE_LOG(LogCombat, Log,
           TEXT("AMeleeWeapon::Fire - hit zone active for 0.2s"));
  }
}

void AMeleeWeapon::PerformDoubleSwing() {
  bIsSecondSwing = true;
  ActivateHitZone();

  // Set a timer to deactivate the second swing's zone
  GetWorldTimerManager().SetTimer(
      HitZoneTimer, this, &AMeleeWeapon::DeactivateHitZone, 0.2f, false);
  bIsSecondSwing = false;
}

void AMeleeWeapon::ActivateHitZone() {
  const float ScaledExtent = HitZoneHalfExtent * RangeMultiplier;
  const float ScaledWidth = HitZoneWidth * WidthMultiplier;
  const float ScaledThickness = HitZoneThickness * ThicknessMultiplier;
  const float OmniExtent = FMath::Max(ScaledExtent, ScaledWidth);
  const FVector HitExtent = bOmniSwing
                                ? FVector(OmniExtent, OmniExtent, ScaledThickness)
                                : FVector(ScaledExtent, ScaledWidth, ScaledThickness);

  if (HitZone) {
    // 1. Set size first
    HitZone->SetBoxExtent(HitExtent);
    // 2. Enable collision BEFORE moving so Sweep/UpdateOverlaps can work
    HitZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    // 3. Ensure we overlap Pawns specifically
    HitZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
  }

  if (IsValid(GetOwner())) {
    const FVector OwnerLocation = GetOwner()->GetActorLocation();
    const FRotator FireRot = bOmniSwing ? FRotator::ZeroRotator : FireDirection.Rotation();

    // 4. Move and Rotate with Sweep=true to force overlap checks along the path
    FHitResult SweepHit;
    const FVector Offset = bOmniSwing ? FVector::ZeroVector
                                      : FireDirection.GetSafeNormal() * ScaledExtent;
    SetActorLocationAndRotation(OwnerLocation + Offset, FireRot, true,
                                &SweepHit);
  }

  if (HitZone) {
    // 5. Force update overlaps in case we are already inside an actor
    HitZone->UpdateOverlaps();
  }

  UE_LOG(LogCombat, Log,
         TEXT("AMeleeWeapon::ActivateHitZone - Zone active at %s, rotation %s"),
         *GetActorLocation().ToString(), *GetActorRotation().ToString());
}

void AMeleeWeapon::DeactivateHitZone() {
  HitZone->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

#include "Enemy/BaseEnemy.h"

void AMeleeWeapon::OnHitZoneOverlap(UPrimitiveComponent *OverlappedComp,
                                    AActor *OtherActor,
                                    UPrimitiveComponent *OtherComp,
                                    int32 OtherBodyIndex, bool bFromSweep,
                                    const FHitResult &SweepResult) {
  if (!IsValid(OtherActor) || OtherActor == GetOwner())
    return;

  // Friendly Fire Prevention: Enemies shouldn't hit other enemies
  if (GetOwner() && GetOwner()->IsA(ABaseEnemy::StaticClass()) && OtherActor->IsA(ABaseEnemy::StaticClass()))
    return;

  OtherActor->TakeDamage(BaseDamage, FDamageEvent(), nullptr, GetOwner());
}
