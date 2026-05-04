// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "DepthrunHUD.generated.h"

class UHealthBarWidget;
class UHUDOverlayWidget;

/**
 * ADepthrunHUD
 * Main HUD actor. Creates and manages in-world widgets.
 * Widget layout is done in UMG Editor (Blueprint child or WBP).
 * Implementation: Stage 9.
 */
UCLASS()
class DEPTHRUN_API ADepthrunHUD : public AHUD
{
	GENERATED_BODY()

public:
	ADepthrunHUD();

protected:
	virtual void BeginPlay() override;

public:
	/** Called by damage system to refresh HP display. */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdatePlayerHP(float Current, float Max);

	/** Toggle the debug adaptive overlay visible/hidden. */
	UFUNCTION(BlueprintCallable, Category = "HUD|Debug")
	void ToggleAdaptiveDebugWidget();

	/** Returns the main HUD overlay widget (WBP_HUDOverlay). Use to call Set Active Weapon Slot etc. */
	UFUNCTION(BlueprintPure, Category = "HUD")
	UHUDOverlayWidget* GetHUDOverlay() const;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "HUD|Classes")
	TSubclassOf<UUserWidget> MainWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "HUD|Classes")
	TSubclassOf<UUserWidget> DebugAdaptiveWidgetClass;

private:
	UPROPERTY()
	TObjectPtr<UUserWidget> MainWidget;

	UPROPERTY()
	TObjectPtr<UUserWidget> DebugWidget;

	bool bDebugWidgetVisible = false;
};
