// Copyright Depthrun Project, 2026. All Rights Reserved.

#include "PlayerCombatComponent.h"
#include "Combat/BaseWeapon.h"
#include "Core/DepthrunLogChannels.h"

UPlayerCombatComponent::UPlayerCombatComponent() {
  PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerCombatComponent::Attack() {
  if (!IsValid(CurrentWeapon)) {
    UE_LOG(LogCombat, Verbose,
           TEXT("UPlayerCombatComponent::Attack — no weapon equipped"));
    return;
  }

  CurrentWeapon->Fire();
  UE_LOG(LogCombat, Log, TEXT("UPlayerCombatComponent::Attack — fired"));
}

void UPlayerCombatComponent::EquipWeapon(ABaseWeapon *NewWeapon) {
  if (IsValid(CurrentWeapon)) {
    CurrentWeapon->Destroy();
  }
  CurrentWeapon = NewWeapon;
  UE_LOG(LogCombat, Log,
         TEXT("UPlayerCombatComponent::EquipWeapon — equipped %s"),
         NewWeapon ? *NewWeapon->GetName() : TEXT("null"));
}
