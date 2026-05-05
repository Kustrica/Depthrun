// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SettingsWidget.generated.h"

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

public:
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
