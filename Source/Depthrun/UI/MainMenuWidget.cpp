// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "MainMenuWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/UISoundLibrary.h"

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

	// TODO (Stage 9H): open settings panel
	UE_LOG(LogTemp, Log, TEXT("[MainMenu] Settings pressed — not yet implemented"));
}

void UMainMenuWidget::OnQuitPressed()
{
	if (UUISoundLibrary* SFX = GetGameInstance()->GetSubsystem<UUISoundLibrary>())
		SFX->PlayButtonClick();

	UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false);
}
