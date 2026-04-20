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
}

void UPlayerCombatComponent::EquipWeapon(ABaseWeapon *NewWeapon) {
  // Commercial Fix: Do NOT destroy weapons when switching, as they are pre-spawned and persistent in our system.
  CurrentWeapon = NewWeapon;
  UE_LOG(LogCombat, Log,
         TEXT("UPlayerCombatComponent::EquipWeapon — equipped %s"),
         NewWeapon ? *NewWeapon->GetName() : TEXT("null"));
}
