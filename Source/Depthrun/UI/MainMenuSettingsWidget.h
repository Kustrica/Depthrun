// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UI/SettingsWidget.h"
#include "MainMenuSettingsWidget.generated.h"

class UImage;

/**
 * UMainMenuSettingsWidget
 * Settings screen opened from Main Menu.
 * Inherits all volume logic from USettingsWidget.
 * Adds an optional BackgroundImage so the black void behind is covered.
 * Assign WBP_MainMenuSettings as SettingsWidgetClass in WBP_MainMenu.
 */
UCLASS()
class DEPTHRUN_API UMainMenuSettingsWidget : public USettingsWidget
{
	GENERATED_BODY()

public:
	/** Optional full-screen background image. Set its brush in the Blueprint. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> BackgroundImage;
};
