// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

// ─── Logging Categories ──────────────────────────────────────────────────────
// Declare in the header, define in DepthrunLogChannels.cpp.
// Usage: UE_LOG(LogAdaptiveBehavior, Log, TEXT("..."))

DEPTHRUN_API DECLARE_LOG_CATEGORY_EXTERN(LogDepthrun,         Log, All)
DEPTHRUN_API DECLARE_LOG_CATEGORY_EXTERN(LogFSM,              Log, All)
DEPTHRUN_API DECLARE_LOG_CATEGORY_EXTERN(LogAdaptiveBehavior, Log, All)
DEPTHRUN_API DECLARE_LOG_CATEGORY_EXTERN(LogRoomGen,          Log, All)
DEPTHRUN_API DECLARE_LOG_CATEGORY_EXTERN(LogCombat,           Log, All)
