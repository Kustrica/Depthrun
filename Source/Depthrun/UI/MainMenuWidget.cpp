// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "MainMenuWidget.h"
#include "Kismet/GameplayStatics.h"

void UMainMenuWidget::OnPlayPressed()
{
	PlayTransitionOut();
	// TODO (Stage 9A): fade-out then load Hub level
	UGameplayStatics::OpenLevel(this, HubLevelName);
}

void UMainMenuWidget::OnQuitPressed()
{
	UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false);
}
