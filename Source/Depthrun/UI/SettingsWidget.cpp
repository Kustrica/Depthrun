// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "UI/SettingsWidget.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"
#include "Core/DepthrunGameInstance.h"

void USettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (VolumeSlider)
	{
		VolumeSlider->OnValueChanged.AddDynamic(this, &USettingsWidget::OnVolumeChanged);

		// Restore saved volume from GameInstance
		if (UDepthrunGameInstance* GI = Cast<UDepthrunGameInstance>(GetGameInstance()))
		{
			VolumeSlider->SetValue(GI->MasterVolume);
			OnVolumeChanged(GI->MasterVolume);
		}
		else
		{
			VolumeSlider->SetValue(1.f);
			OnVolumeChanged(1.f);
		}
	}

	if (BackBtn) BackBtn->OnClicked.AddDynamic(this, &USettingsWidget::OnBackClicked);
}

void USettingsWidget::OnVolumeChanged(float Value)
{
	if (VolumeValueText)
		VolumeValueText->SetText(FText::FromString(FString::Printf(TEXT("%.0f%%"), Value * 100.f)));

	// Apply to SC_Master sound class via sound mix
	if (UDepthrunGameInstance* GI = Cast<UDepthrunGameInstance>(GetGameInstance()))
	{
		GI->MasterVolume = Value;
		if (GI->MasterSoundMix && GI->MasterSoundClass)
		{
			UGameplayStatics::SetSoundMixClassOverride(
				GetWorld(), GI->MasterSoundMix, GI->MasterSoundClass, Value, 1.f, 0.f, true);
			UGameplayStatics::PushSoundMixModifier(GetWorld(), GI->MasterSoundMix);
		}
	}
}

void USettingsWidget::OnBackClicked()
{
	RemoveFromParent();
}
