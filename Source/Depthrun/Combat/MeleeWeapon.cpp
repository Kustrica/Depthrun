// Copyright Depthrun Project, 2026. All Rights Reserved.

#include "MeleeWeapon.h"
#include "Components/BoxComponent.h"
#include "Engine/DamageEvents.h"
#include "Core/DepthrunLogChannels.h"

AMeleeWeapon::AMeleeWeapon()
{
	HitZone = CreateDefaultSubobject<UBoxComponent>(TEXT("HitZone"));
	HitZone->SetupAttachment(RootComponent);
	HitZone->SetBoxExtent(FVector(HitZoneHalfExtent, 8.f, HitZoneHalfExtent));
	HitZone->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitZone->SetCollisionResponseToAllChannels(ECR_Overlap);
	HitZone->SetCollisionObjectType(ECC_WorldDynamic);
	HitZone->OnComponentBeginOverlap.AddDynamic(this, &AMeleeWeapon::OnHitZoneOverlap);
}

void AMeleeWeapon::Fire()
{
	if (!bCanFire) return;

	bCanFire = false;
	ActivateHitZone();

	// Hit zone stays active 0.2s, then deactivates. Full cooldown follows.
	GetWorldTimerManager().SetTimer(HitZoneTimer, this, &AMeleeWeapon::DeactivateHitZone, 0.2f, false);
	StartCooldown();

	UE_LOG(LogCombat, Log, TEXT("AMeleeWeapon::Fire — hit zone active for 0.2s"));
}

void AMeleeWeapon::ActivateHitZone()
{
	// Position hit zone in front of the owner based on attack direction
	if (IsValid(GetOwner()))
	{
		const FVector OwnerLocation = GetOwner()->GetActorLocation();
		// FireDirection is set by ADepthrunCharacter before calling Fire()
		const FVector Offset = FireDirection.GetSafeNormal() * (HitZoneHalfExtent + 12.f);
		SetActorLocation(OwnerLocation + Offset);
	}
	HitZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AMeleeWeapon::DeactivateHitZone()
{
	HitZone->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AMeleeWeapon::OnHitZoneOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                    bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor) || OtherActor == GetOwner()) return;

	UE_LOG(LogCombat, Log, TEXT("AMeleeWeapon::OnHitZoneOverlap → %s  damage=%.1f"),
		*OtherActor->GetName(), BaseDamage);

	OtherActor->TakeDamage(BaseDamage, FDamageEvent(), nullptr, GetOwner());
}
