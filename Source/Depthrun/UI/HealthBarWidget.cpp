// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "HealthBarWidget.h"

void UHealthBarWidget::SetHealthPercent(float Current, float Max)
{
	HealthPercent = (Max > 0.f) ? FMath::Clamp(Current / Max, 0.f, 1.f) : 0.f;
	OnHealthUpdated(HealthPercent);
}
