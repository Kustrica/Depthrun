// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUDOverlayWidget.generated.h"

class ADepthrunCharacter;

/**
 * UHUDOverlayWidget
 * In-game HUD overlay. Displays: HP, Diamonds, Potions, Active Weapon Slot, Room Number.
 * Layout is done in UMG Editor (WBP_HUDOverlay).
 * Binds to PlayerEconomy delegates on NativeConstruct.
 * Implementation: Stage 9H.
 */
UCLASS()
class DEPTHRUN_API UHUDOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ─── Called from C++ / Blueprint to push data ───────────────────────────

	/** Update HP bar and text. */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetHP(float Current, float Max);

	/** Update diamond counter. */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetDiamonds(int32 Amount);

	/** Update potion counter. */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetPotions(int32 Amount);

	/** Update room counter text ("Room X / Y"). */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetRoomInfo(int32 Current, int32 Total);

	/** Set active weapon slot index (0 = sword, 1 = bow). */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetActiveWeaponSlot(int32 SlotIndex);

	// ─── Pulled from character on tick or delegate ──────────────────────────

	/** Refresh all values from the player character. */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void RefreshFromPlayer();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// ─── Blueprint Implementable Events (update visual in WBP) ──────────────

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void OnHPUpdated(float Percent, float Current, float Max);

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void OnDiamondsUpdated(int32 Amount);

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void OnPotionsUpdated(int32 Amount);

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void OnRoomInfoUpdated(int32 Current, int32 Total);

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void OnWeaponSlotUpdated(int32 SlotIndex);

private:
	void BindToPlayerDelegates(ADepthrunCharacter* Player);
	void UnbindFromPlayerDelegates();

	UFUNCTION()
	void SetDiamondsFromDelegate(int32 Old, int32 New);

	UFUNCTION()
	void SetPotionsFromDelegate(int32 Old, int32 New);

	UPROPERTY()
	TObjectPtr<ADepthrunCharacter> CachedPlayer;
};
