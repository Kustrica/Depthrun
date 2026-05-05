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

	// Must be focusable to receive NativeOnKeyDown
	SetIsFocusable(true);
	SetKeyboardFocus();

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

	if (UDepthrunGameInstance* GI = Cast<UDepthrunGameInstance>(GetGameInstance()))
	{
		GI->MasterVolume = Value;
		if (GI->MasterSoundMix && GI->MasterSoundClass)
		{
			// Apply to project SC_Master class
			UGameplayStatics::SetSoundMixClassOverride(
				GetWorld(), GI->MasterSoundMix, GI->MasterSoundClass, Value, 1.f, 0.f, true);
			UGameplayStatics::PushSoundMixModifier(GetWorld(), GI->MasterSoundMix);
		}
		// Also apply to engine built-in "Master" class so ALL sounds are affected
		// regardless of whether they explicitly use SC_Master
		if (GI->MasterSoundMix)
		{
			if (USoundClass* BuiltInMaster = LoadObject<USoundClass>(
				nullptr, TEXT("/Engine/EngineSounds/Master.Master")))
			{
				UGameplayStatics::SetSoundMixClassOverride(
					GetWorld(), GI->MasterSoundMix, BuiltInMaster, Value, 1.f, 0.f, true);
			}
		}
	}
}

void USettingsWidget::OnBackClicked()
{
	OnSettingsClosed.Broadcast();
	RemoveFromParent();
}

FReply USettingsWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	if (Key == EKeys::Escape || Key == EKeys::P)
	{
		OnSettingsClosed.Broadcast();
		RemoveFromParent();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}
