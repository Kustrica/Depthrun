// Copyright Depthrun Project, 2026. All Rights Reserved.

#include "DepthrunGameMode.h"
#include "DepthrunLogChannels.h"

ADepthrunGameMode::ADepthrunGameMode()
{
	// Default classes are assigned in Project Settings → Maps & Modes
}

void ADepthrunGameMode::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogDepthrun, Log, TEXT("ADepthrunGameMode::BeginPlay — session started"));
}
