// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/RunItemCollection.h"
#include "RunItemInventory.generated.h"

class ABaseWeapon;
class ADepthrunCharacter;

/**
 * URunItemInventory
 * Per-run item inventory carried by ADepthrunCharacter.
 * Items are run-scoped: cleared when a new run starts.
 *
 * Effect dispatch (Stage 3G implementation):
 *   ApplyToWeapon(ABaseWeapon*) iterates Items, checks TargetWeapon,
 *   calls the appropriate weapon method for each effect.
 *
 * Supported effects:
 *   MeleeExtendedRange  → AMeleeWeapon::SetRangeMultiplier(float)
 *   MeleeDoubleSwing    → AMeleeWeapon::EnableDoubleSwing(true)
 *   ArrowRicochet       → ARangedWeapon::SetRicochetCount(int32)
 *   ArrowPierce         → ARangedWeapon::EnablePierce(true)
 */
UCLASS(ClassGroup = (Depthrun), meta = (BlueprintSpawnableComponent))
class DEPTHRUN_API URunItemInventory : public UActorComponent
{
	GENERATED_BODY()

public:
	URunItemInventory();

	// ─── Item management ─────────────────────────────────────────────────────

	/** Add an item to the run inventory. Returns false if inventory is full. */
	bool AddItem(const FRunItemData& Data);

	/** Remove all items (call at run start). */
	UFUNCTION(BlueprintCallable, Category = "Items")
	void ClearItems();

	/** Apply all relevant items to a weapon when it becomes active. */
	UFUNCTION(BlueprintCallable, Category = "Items")
	void ApplyToWeapon(ABaseWeapon* Weapon) const;

	/** Apply stat-boost items (BonusMaxHP, BonusMoveSpeed, BonusProjectileCount) to character. */
	UFUNCTION(BlueprintCallable, Category = "Items")
	void ApplyToCharacter(ADepthrunCharacter* Character) const;

	/** Returns true if any item with this effect is present. */
	UFUNCTION(BlueprintPure, Category = "Items")
	bool HasEffect(ERunItemEffect Effect) const;

	/** All items currently held in this run. */
	const TArray<FRunItemData>& GetItems() const { return Items; }

	/** Max items allowed per run. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Items")
	int32 MaxItems = 6;

private:
	TArray<FRunItemData> Items;
};
