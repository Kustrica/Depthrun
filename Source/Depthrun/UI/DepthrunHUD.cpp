// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "DepthrunHUD.h"
#include "Blueprint/UserWidget.h"

ADepthrunHUD::ADepthrunHUD() {}

void ADepthrunHUD::BeginPlay()
{
	Super::BeginPlay();
	// TODO (Stage 9): create MainWidget and DebugWidget, add to viewport
}

void ADepthrunHUD::UpdatePlayerHP(float Current, float Max)
{
	// TODO (Stage 9): call into MainWidget HP bar
}

void ADepthrunHUD::ToggleAdaptiveDebugWidget()
{
	// TODO (Stage 9 / Stage 7): show/hide DebugWidget
	bDebugWidgetVisible = !bDebugWidgetVisible;
}
