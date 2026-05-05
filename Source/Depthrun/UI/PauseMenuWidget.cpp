// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "UI/PauseMenuWidget.h"
#include "UI/SettingsWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Player/PlayerEconomy.h"
#include "Items/RunItemInventory.h"
#include "GameFramework/Character.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/KismetSystemLibrary.h"

void UPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(true);
	SetKeyboardFocus();

	if (ContinueBtn) ContinueBtn->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnContinueClicked);
	if (SettingsBtn)  SettingsBtn->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnSettingsClicked);
	if (ToHubBtn)     ToHubBtn->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnToHubClicked);
	if (ToMenuBtn)    ToMenuBtn->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnToMenuClicked);
	if (QuitBtn)      QuitBtn->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnQuitClicked);
}

void UPauseMenuWidget::Show()
{
	SetVisibility(ESlateVisibility::Visible);
	SetKeyboardFocus();
}

FReply UPauseMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	if (Key == EKeys::Escape || Key == EKeys::P)
	{
		OnContinueClicked(); // unpause + remove
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UPauseMenuWidget::OnContinueClicked()
{
	UGameplayStatics::SetGamePaused(GetWorld(), false);
	RemoveFromParent();
}

void UPauseMenuWidget::OnSettingsClicked()
{
	if (!SettingsWidgetClass) return;
	if (APlayerController* PC = GetOwningPlayer())
	{
		USettingsWidget* Settings = CreateWidget<USettingsWidget>(PC, SettingsWidgetClass);
		if (Settings)
		{
			// When Settings closes, return focus to pause menu
			Settings->OnSettingsClosed.AddLambda([this]()
			{
				if (IsValid(this))
					SetKeyboardFocus();
			});
			Settings->AddToViewport(11);
		}
	}
}

void UPauseMenuWidget::OnToHubClicked()
{
	UGameplayStatics::SetGamePaused(GetWorld(), false);

	if (ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
	{
		if (UPlayerEconomy* Econ = Player->FindComponentByClass<UPlayerEconomy>())
			Econ->OnRunExitToHub();
		if (URunItemInventory* Inv = Player->FindComponentByClass<URunItemInventory>())
			Inv->ClearItems();
	}

	UGameplayStatics::OpenLevel(GetWorld(), TEXT("L_Hub"));
}

void UPauseMenuWidget::OnToMenuClicked()
{
	UGameplayStatics::SetGamePaused(GetWorld(), false);
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("L_MainMenu"));
}

void UPauseMenuWidget::OnQuitClicked()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), GetOwningPlayer(), EQuitPreference::Quit, false);
}
