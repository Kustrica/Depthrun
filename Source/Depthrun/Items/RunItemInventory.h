// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/RunItemTypes.h"
#include "RunItemInventory.generated.h"

class URunItemConfig;
class ABaseWeapon;

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

	/** Add an item to the run inventory. Returns false if already has it. */
	UFUNCTION(BlueprintCallable, Category = "Items")
	bool AddItem(URunItemConfig* Config);

	/** Remove all items (call at run start). */
	UFUNCTION(BlueprintCallable, Category = "Items")
	void ClearItems();

	/** Apply all relevant items to a weapon when it becomes active. */
	UFUNCTION(BlueprintCallable, Category = "Items")
	void ApplyToWeapon(ABaseWeapon* Weapon) const;

	/** Returns true if any item with this effect is present. */
	UFUNCTION(BlueprintPure, Category = "Items")
	bool HasEffect(ERunItemEffect Effect) const;

	/** All items currently held in this run. */
	UFUNCTION(BlueprintPure, Category = "Items")
	const TArray<URunItemConfig*>& GetItems() const { return Items; }

	/** Max items allowed per run. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Items")
	int32 MaxItems = 6;

	/** Broadcast when an item is added. Subscribed by HubWidget. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemAdded, URunItemConfig*, Config);
	UPROPERTY(BlueprintAssignable, Category = "Items")
	FOnItemAdded OnItemAdded;

private:
	UPROPERTY()
	TArray<TObjectPtr<URunItemConfig>> Items;
};
