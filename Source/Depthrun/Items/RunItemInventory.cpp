// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "RunItemInventory.h"
#include "Combat/BaseWeapon.h"
#include "Combat/MeleeWeapon.h"
#include "Combat/RangedWeapon.h"
#include "Player/DepthrunCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Core/DepthrunLogChannels.h"

URunItemInventory::URunItemInventory()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool URunItemInventory::AddItem(const FRunItemData& Data)
{
	if (Items.Num() >= MaxItems)
	{
		UE_LOG(LogDepthrun, Warning, TEXT("[Items] Inventory full (%d/%d)"), Items.Num(), MaxItems);
		return false;
	}
	Items.Add(Data);
	UE_LOG(LogDepthrun, Log, TEXT("[Items] Added: %s"), *Data.ItemName);
	return true;
}

void URunItemInventory::ClearItems()
{
	Items.Reset();
	UE_LOG(LogDepthrun, Log, TEXT("[Items] Inventory cleared"));
}

void URunItemInventory::ApplyToWeapon(ABaseWeapon* Weapon) const
{
	if (!Weapon) return;

	Weapon->ResetBaseDamage();
	if (AMeleeWeapon* Melee = Cast<AMeleeWeapon>(Weapon)) Melee->ResetEffects();
	else if (ARangedWeapon* Ranged = Cast<ARangedWeapon>(Weapon)) Ranged->ResetEffects();

	for (const FRunItemData& Item : Items)
	{
		const bool bMatchingType =
			Item.TargetWeapon == EItemTargetWeapon::Any ||
			(Weapon->GetWeaponType() == EWeaponType::Melee  && Item.TargetWeapon == EItemTargetWeapon::Melee) ||
			(Weapon->GetWeaponType() == EWeaponType::Ranged && Item.TargetWeapon == EItemTargetWeapon::Ranged);

		if (!bMatchingType) continue;

		switch (Item.Effect)
		{
		case ERunItemEffect::FlatDamage:
			Weapon->BaseDamage += 5.f;
			break;
		case ERunItemEffect::MeleeExtendedRange:
			if (AMeleeWeapon* Melee = Cast<AMeleeWeapon>(Weapon))
				Melee->SetRangeMultiplier(Melee->GetRangeMultiplier() + (Item.RangeMultiplier - 1.0f));
			break;
		case ERunItemEffect::MeleeDoubleSwing:
			if (AMeleeWeapon* Melee = Cast<AMeleeWeapon>(Weapon))
				Melee->SetDoubleSwing(true);
			break;
		case ERunItemEffect::ArrowRicochet:
			if (ARangedWeapon* Ranged = Cast<ARangedWeapon>(Weapon))
				Ranged->SetRicochetCount(Ranged->GetRicochetCount() + Item.RicochetCount);
			break;
		case ERunItemEffect::ArrowPierce:
			if (ARangedWeapon* Ranged = Cast<ARangedWeapon>(Weapon))
				Ranged->SetPierceEnabled(true);
			break;
		default: break;
		}

		UE_LOG(LogDepthrun, Verbose, TEXT("[Items] Applied: %s to %s"), *Item.ItemName, *Weapon->GetName());
	}
}

void URunItemInventory::ApplyToCharacter(ADepthrunCharacter* Character) const
{
	if (!Character) return;

	for (const FRunItemData& Item : Items)
	{
		switch (Item.Effect)
		{
		case ERunItemEffect::BonusMaxHP:
			Character->MaxHP += Item.NumericValue;
			Character->CurrentHP = FMath::Clamp(Character->CurrentHP, 0.f, Character->MaxHP);
			UE_LOG(LogDepthrun, Log, TEXT("[Items] BonusMaxHP +%.0f → MaxHP=%.0f"), Item.NumericValue, Character->MaxHP);
			break;
		case ERunItemEffect::BonusMoveSpeed:
			if (UCharacterMovementComponent* MC = Character->GetCharacterMovement())
			{
				const float Bonus = MC->MaxFlySpeed * Item.NumericValue;
				MC->MaxFlySpeed  += Bonus;
				MC->MaxWalkSpeed += Bonus;
				UE_LOG(LogDepthrun, Log, TEXT("[Items] BonusMoveSpeed +%.0f%% → MaxFlySpeed=%.0f"),
					Item.NumericValue * 100.f, MC->MaxFlySpeed);
			}
			break;
		case ERunItemEffect::BonusProjectileCount:
			Character->BaseProjectileCount = FMath::Clamp(
				Character->BaseProjectileCount + FMath::RoundToInt(Item.NumericValue), 1, 5);
			UE_LOG(LogDepthrun, Log, TEXT("[Items] BonusProjectileCount +%d → Count=%d"),
				FMath::RoundToInt(Item.NumericValue), Character->BaseProjectileCount);
			break;
		default: break;
		}
	}
}

bool URunItemInventory::HasEffect(ERunItemEffect Effect) const
{
	for (const FRunItemData& Item : Items)
	{
		if (Item.Effect == Effect) return true;
	}
	return false;
}
