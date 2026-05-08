// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "EnemyNumberWidget.h"
#include "Components/TextBlock.h"

void UEnemyNumberWidget::SetEnemyIndex(int32 Index)
{
	EnemyIndex = Index;
	FText Display = FText::FromString(FString::Printf(TEXT("#%d"), Index + 1));

	// Try to update a TextBlock named "NumberText" directly (faster than full BP event)
	if (UTextBlock* TB = Cast<UTextBlock>(GetWidgetFromName(TEXT("NumberText"))))
	{
		TB->SetText(Display);
	}

	// Also fire the Blueprint event for custom animations / styling
	OnIndexSet(Display);
}
