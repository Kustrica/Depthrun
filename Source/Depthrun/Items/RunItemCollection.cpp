// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "RunItemCollection.h"

URunItemCollection::URunItemCollection()
{
	// Pre-fill all 7 run items with sensible defaults.
	// These values are editable in the DA_RunItemCollection asset in Editor.

	FRunItemData ArrowRicochet;
	ArrowRicochet.ItemName    = TEXT("Arrow Ricochet");
	ArrowRicochet.Effect      = ERunItemEffect::ArrowRicochet;
	ArrowRicochet.TargetWeapon = EItemTargetWeapon::Ranged;
	ArrowRicochet.RicochetCount = 2;

	FRunItemData ArrowPierce;
	ArrowPierce.ItemName    = TEXT("Arrow Pierce");
	ArrowPierce.Effect      = ERunItemEffect::ArrowPierce;
	ArrowPierce.TargetWeapon = EItemTargetWeapon::Ranged;

	FRunItemData ExtendedRange;
	ExtendedRange.ItemName    = TEXT("Extended Range");
	ExtendedRange.Effect      = ERunItemEffect::MeleeExtendedRange;
	ExtendedRange.TargetWeapon = EItemTargetWeapon::Melee;
	ExtendedRange.RangeMultiplier = 1.5f;

	FRunItemData DoubleSwing;
	DoubleSwing.ItemName    = TEXT("Double Swing");
	DoubleSwing.Effect      = ERunItemEffect::MeleeDoubleSwing;
	DoubleSwing.TargetWeapon = EItemTargetWeapon::Melee;

	FRunItemData BonusHP;
	BonusHP.ItemName    = TEXT("Bonus HP");
	BonusHP.Effect      = ERunItemEffect::BonusMaxHP;
	BonusHP.TargetWeapon = EItemTargetWeapon::Any;
	BonusHP.NumericValue = 25.f;

	FRunItemData BonusSpeed;
	BonusSpeed.ItemName    = TEXT("Bonus Speed");
	BonusSpeed.Effect      = ERunItemEffect::BonusMoveSpeed;
	BonusSpeed.TargetWeapon = EItemTargetWeapon::Any;
	BonusSpeed.NumericValue = 0.15f;

	FRunItemData BonusArrows;
	BonusArrows.ItemName    = TEXT("Bonus Arrows");
	BonusArrows.Effect      = ERunItemEffect::BonusProjectileCount;
	BonusArrows.TargetWeapon = EItemTargetWeapon::Any;
	BonusArrows.NumericValue = 1.f;

	Items = { ArrowRicochet, ArrowPierce, ExtendedRange, DoubleSwing, BonusHP, BonusSpeed, BonusArrows };
}

int32 URunItemCollection::FindIndexByName(const FString& Name) const
{
	const FString Lower = Name.ToLower();
	for (int32 i = 0; i < Items.Num(); ++i)
	{
		if (Items[i].ItemName.ToLower().Contains(Lower))
		{
			return i;
		}
	}
	return -1;
}

const FRunItemData* URunItemCollection::FindByName(const FString& Name) const
{
	const int32 Idx = FindIndexByName(Name);
	return Idx >= 0 ? &Items[Idx] : nullptr;
}
