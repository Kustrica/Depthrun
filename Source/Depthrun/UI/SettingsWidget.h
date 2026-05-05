// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InputCoreTypes.h"
#include "SettingsWidget.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnSettingsClosed);

class UButton;
class USlider;
class UTextBlock;

/**
 * USettingsWidget
 * Volume slider + Back button. Works standalone or pushed over PauseMenu.
 */
UCLASS()
class DEPTHRUN_API USettingsWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

public:
	/** Fired when the widget removes itself (Back button or Esc/P). */
	FOnSettingsClosed OnSettingsClosed;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> VolumeSlider;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> VolumeValueText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BackBtn;

private:
	UFUNCTION()
	void OnVolumeChanged(float Value);

	UFUNCTION()
	void OnBackClicked();
};
