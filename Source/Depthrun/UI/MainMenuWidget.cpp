// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "MainMenuWidget.h"
#include "UI/SettingsWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/UISoundLibrary.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(true);
	SetKeyboardFocus();
}

FReply UMainMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		OnQuitPressed();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UMainMenuWidget::OnPlayPressed()
{
	if (UUISoundLibrary* SFX = GetGameInstance()->GetSubsystem<UUISoundLibrary>())
		SFX->PlayButtonClick();

	PlayTransitionOut();
	UGameplayStatics::OpenLevel(this, HubLevelName);
}

void UMainMenuWidget::OnSettingsPressed()
{
	if (UUISoundLibrary* SFX = GetGameInstance()->GetSubsystem<UUISoundLibrary>())
		SFX->PlayButtonClick();

	if (!SettingsWidgetClass) return;

	USettingsWidget* Settings = CreateWidget<USettingsWidget>(GetOwningPlayer(), SettingsWidgetClass);
	if (!Settings) return;

	SetVisibility(ESlateVisibility::Hidden);
	Settings->OnSettingsClosed.AddLambda([this]()
	{
		if (IsValid(this))
		{
			SetVisibility(ESlateVisibility::Visible);
			SetKeyboardFocus();
		}
	});
	Settings->AddToViewport(5);
}

void UMainMenuWidget::OnQuitPressed()
{
	if (UUISoundLibrary* SFX = GetGameInstance()->GetSubsystem<UUISoundLibrary>())
		SFX->PlayButtonClick();

	UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false);
}

void UMainMenuWidget::OnButtonHovered()
{
	if (UUISoundLibrary* SFX = GetGameInstance()->GetSubsystem<UUISoundLibrary>())
		SFX->PlayButtonHover();
}
