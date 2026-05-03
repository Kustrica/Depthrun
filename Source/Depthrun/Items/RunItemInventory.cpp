// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "RunItemInventory.h"
#include "RunItemConfig.h"
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

bool URunItemInventory::AddItem(URunItemConfig* Config)
{
	if (!Config) return false;
	if (Items.Num() >= MaxItems)
	{
		UE_LOG(LogDepthrun, Warning, TEXT("[Items] Inventory full (%d/%d)"), Items.Num(), MaxItems);
		return false;
	}
	Items.Add(Config);
	OnItemAdded.Broadcast(Config);
	UE_LOG(LogDepthrun, Log, TEXT("[Items] Added: %s"), *Config->ItemName.ToString());
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

	// Reset weapon to base state before applying modifiers
	Weapon->ResetBaseDamage();
	if (AMeleeWeapon* Melee = Cast<AMeleeWeapon>(Weapon)) Melee->ResetEffects();
	else if (ARangedWeapon* Ranged = Cast<ARangedWeapon>(Weapon)) Ranged->ResetEffects();

	for (const URunItemConfig* Item : Items)
	{
		if (!Item) continue;

		// Check if item target matches current weapon type
		bool bMatchingType = (Item->TargetWeapon == EItemTargetWeapon::Any);
		if (Weapon->GetWeaponType() == EWeaponType::Melee && Item->TargetWeapon == EItemTargetWeapon::Melee) bMatchingType = true;
		if (Weapon->GetWeaponType() == EWeaponType::Ranged && Item->TargetWeapon == EItemTargetWeapon::Ranged) bMatchingType = true;

		if (!bMatchingType) continue;

		// Apply effect
		switch (Item->Effect)
		{
		case ERunItemEffect::FlatDamage:
			Weapon->BaseDamage += 5.f; // Example: hardcoded 5 flat dmg, or we could add field to Config
			break;
		case ERunItemEffect::MeleeExtendedRange:
			if (AMeleeWeapon* Melee = Cast<AMeleeWeapon>(Weapon))
				Melee->SetRangeMultiplier(Melee->GetRangeMultiplier() + (Item->RangeMultiplier - 1.0f));
			break;
		case ERunItemEffect::MeleeDoubleSwing:
			if (AMeleeWeapon* Melee = Cast<AMeleeWeapon>(Weapon))
				Melee->SetDoubleSwing(true);
			break;
		case ERunItemEffect::ArrowRicochet:
			if (ARangedWeapon* Ranged = Cast<ARangedWeapon>(Weapon))
				Ranged->SetRicochetCount(Ranged->GetRicochetCount() + Item->RicochetCount);
			break;
		case ERunItemEffect::ArrowPierce:
			if (ARangedWeapon* Ranged = Cast<ARangedWeapon>(Weapon))
				Ranged->SetPierceEnabled(true);
			break;
		default: break;
		}

		UE_LOG(LogDepthrun, Verbose, TEXT("[Items] Applied: %s to %s"),
			*Item->ItemName.ToString(), *Weapon->GetName());
	}
}

void URunItemInventory::ApplyToCharacter(ADepthrunCharacter* Character) const
{
	if (!Character) return;

	for (const URunItemConfig* Item : Items)
	{
		if (!Item) continue;

		switch (Item->Effect)
		{
		case ERunItemEffect::BonusMaxHP:
			Character->MaxHP += Item->NumericValue;
			Character->CurrentHP = FMath::Clamp(Character->CurrentHP, 0.f, Character->MaxHP);
			UE_LOG(LogDepthrun, Log, TEXT("[Items] BonusMaxHP +%.0f → MaxHP=%.0f"),
				Item->NumericValue, Character->MaxHP);
			break;
		case ERunItemEffect::BonusMoveSpeed:
			if (UCharacterMovementComponent* MC = Character->GetCharacterMovement())
			{
				const float Bonus = MC->MaxFlySpeed * Item->NumericValue;
				MC->MaxFlySpeed += Bonus;
				MC->MaxWalkSpeed += Bonus;
				UE_LOG(LogDepthrun, Log, TEXT("[Items] BonusMoveSpeed +%.0f (%.0f%%) → MaxFlySpeed=%.0f"),
					Bonus, Item->NumericValue * 100.f, MC->MaxFlySpeed);
			}
			break;
		case ERunItemEffect::BonusProjectileCount:
			Character->BaseProjectileCount += FMath::RoundToInt(Item->NumericValue);
			Character->BaseProjectileCount = FMath::Clamp(Character->BaseProjectileCount, 1, 5);
			UE_LOG(LogDepthrun, Log, TEXT("[Items] BonusProjectileCount +%d → Count=%d"),
				FMath::RoundToInt(Item->NumericValue), Character->BaseProjectileCount);
			break;
		default: break;
		}
	}
}

bool URunItemInventory::HasEffect(ERunItemEffect Effect) const
{
	for (const URunItemConfig* Item : Items)
	{
		if (Item && Item->Effect == Effect) return true;
	}
	return false;
}
