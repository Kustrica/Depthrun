// Copyright Depthrun Project, 2026. All Rights Reserved.

#include "DepthrunGameInstance.h"
#include "DepthrunLogChannels.h"

UDepthrunGameInstance::UDepthrunGameInstance()
{
}

void UDepthrunGameInstance::Init()
{
	Super::Init();
	UE_LOG(LogDepthrun, Log, TEXT("UDepthrunGameInstance::Init — game instance ready"));
}

void UDepthrunGameInstance::Shutdown()
{
	UE_LOG(LogDepthrun, Log, TEXT("UDepthrunGameInstance::Shutdown"));
	Super::Shutdown();
}
