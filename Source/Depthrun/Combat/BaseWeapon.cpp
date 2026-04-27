// Copyright Depthrun Project, 2026. All Rights Reserved.

#include "BaseWeapon.h"
#include "Components/SceneComponent.h"
#include "Core/DepthrunLogChannels.h"

ABaseWeapon::ABaseWeapon()
{
	PrimaryActorTick.bCanEverTick = false;

	// Weapons need an explicit RootComponent so child components
	// (HitZone, Sprite) have a valid attachment target.
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
}

void ABaseWeapon::BeginPlay()
{
	Super::BeginPlay();
	InitialBaseDamage = BaseDamage;
}

float ABaseWeapon::GetDamage() const
{
	return BaseDamage;
}

void ABaseWeapon::ResetBaseDamage()
{
	BaseDamage = InitialBaseDamage;
}

void ABaseWeapon::StartCooldown()
{
	GetWorldTimerManager().SetTimer(CooldownTimer, this, &ABaseWeapon::OnCooldownEnd, AttackCooldown, false);
}

void ABaseWeapon::OnCooldownEnd()
{
	bCanFire = true;
}
