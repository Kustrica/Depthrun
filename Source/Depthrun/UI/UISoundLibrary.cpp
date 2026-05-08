// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "UI/UISoundLibrary.h"
#include "Core/DepthrunGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

void UUISoundLibrary::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (UDepthrunGameInstance* GI = Cast<UDepthrunGameInstance>(GetGameInstance()))
	{
		ButtonClickSound = GI->ButtonClickSound;
		ButtonHoverSound = GI->ButtonHoverSound;
		ChestOpenSound   = GI->ChestOpenSound;
	}
}

static void RefreshFromGI(UUISoundLibrary* Lib)
{
	if (UDepthrunGameInstance* GI = Cast<UDepthrunGameInstance>(Lib->GetGameInstance()))
	{
		Lib->ButtonClickSound = GI->ButtonClickSound;
		Lib->ButtonHoverSound = GI->ButtonHoverSound;
		Lib->ChestOpenSound   = GI->ChestOpenSound;
	}
}

void UUISoundLibrary::PlayButtonClick()
{
	if (!IsValid(ButtonClickSound)) RefreshFromGI(this);
	PlaySound2D(ButtonClickSound);
}

void UUISoundLibrary::PlayButtonHover()
{
	if (!IsValid(ButtonHoverSound)) RefreshFromGI(this);
	PlaySound2D(ButtonHoverSound);
}

void UUISoundLibrary::PlayChestOpen()
{
	if (!IsValid(ChestOpenSound)) RefreshFromGI(this);
	PlaySound2D(ChestOpenSound);
}

void UUISoundLibrary::PlaySound2D(USoundBase* Sound)
{
	if (!IsValid(Sound)) { return; }

	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World) { return; }

	UGameplayStatics::PlaySound2D(World, Sound);
}
