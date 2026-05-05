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
#include "UI/UISoundLibrary.h"

void UPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(true);
	SetKeyboardFocus();

	if (ContinueBtn) { ContinueBtn->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnContinueClicked); ContinueBtn->OnHovered.AddDynamic(this, &UPauseMenuWidget::OnButtonHovered); }
	if (SettingsBtn)  { SettingsBtn->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnSettingsClicked);  SettingsBtn->OnHovered.AddDynamic(this, &UPauseMenuWidget::OnButtonHovered); }
	if (ToHubBtn)     { ToHubBtn->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnToHubClicked);        ToHubBtn->OnHovered.AddDynamic(this, &UPauseMenuWidget::OnButtonHovered); }
	if (ToMenuBtn)    { ToMenuBtn->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnToMenuClicked);      ToMenuBtn->OnHovered.AddDynamic(this, &UPauseMenuWidget::OnButtonHovered); }
	if (QuitBtn)      { QuitBtn->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnQuitClicked);          QuitBtn->OnHovered.AddDynamic(this, &UPauseMenuWidget::OnButtonHovered); }
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

void UPauseMenuWidget::OnButtonHovered()
{
	if (UUISoundLibrary* SFX = GetGameInstance()->GetSubsystem<UUISoundLibrary>())
		SFX->PlayButtonHover();
}

void UPauseMenuWidget::OnContinueClicked()
{
	UGameplayStatics::SetGamePaused(GetWorld(), false);
	OnPauseClosed.Broadcast();
	RemoveFromParent();
}

void UPauseMenuWidget::OnSettingsClicked()
{
	if (UUISoundLibrary* SFX = GetGameInstance()->GetSubsystem<UUISoundLibrary>())
		SFX->PlayButtonClick();
	if (!SettingsWidgetClass) return;
	if (APlayerController* PC = GetOwningPlayer())
	{
		USettingsWidget* Settings = CreateWidget<USettingsWidget>(PC, SettingsWidgetClass);
		if (Settings)
		{
			// Hide pause menu while settings is open
			SetVisibility(ESlateVisibility::Hidden);

			// When Settings closes, show pause menu again and restore focus
			Settings->OnSettingsClosed.AddLambda([this]()
			{
				if (IsValid(this))
				{
					SetVisibility(ESlateVisibility::Visible);
					SetKeyboardFocus();
				}
			});
			Settings->AddToViewport(11);
		}
	}
}

void UPauseMenuWidget::OnToHubClicked()
{
	if (UUISoundLibrary* SFX = GetGameInstance()->GetSubsystem<UUISoundLibrary>())
		SFX->PlayButtonClick();
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
	if (UUISoundLibrary* SFX = GetGameInstance()->GetSubsystem<UUISoundLibrary>())
		SFX->PlayButtonClick();
	UGameplayStatics::SetGamePaused(GetWorld(), false);
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("L_MainMenu"));
}

void UPauseMenuWidget::OnQuitClicked()
{
	if (UUISoundLibrary* SFX = GetGameInstance()->GetSubsystem<UUISoundLibrary>())
		SFX->PlayButtonClick();
	UKismetSystemLibrary::QuitGame(GetWorld(), GetOwningPlayer(), EQuitPreference::Quit, false);
}
