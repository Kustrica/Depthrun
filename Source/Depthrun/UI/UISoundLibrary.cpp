// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "UI/UISoundLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

void UUISoundLibrary::PlayButtonClick()
{
	PlaySound2D(ButtonClickSound);
}

void UUISoundLibrary::PlayButtonHover()
{
	PlaySound2D(ButtonHoverSound);
}

void UUISoundLibrary::PlayChestOpen()
{
	PlaySound2D(ChestOpenSound);
}

void UUISoundLibrary::PlaySound2D(USoundBase* Sound)
{
	if (!IsValid(Sound)) { return; }

	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World) { return; }

	UGameplayStatics::PlaySound2D(World, Sound);
}
