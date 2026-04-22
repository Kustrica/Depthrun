// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "AdaptiveConfig.h"

UAdaptiveConfig::UAdaptiveConfig()
{
    // ─── Initialize Transition Cost Matrix ───────────────────────────────────
    // Matrix 6x6 for EFSMStateType (None=0, Idle=1, Chase=2, Attack=3, Retreat=4, Flank=5)
    
    TransitionCostMatrix.SetNumUninitialized(36);

    // Row 0: None
    for(int i=0; i<6; ++i) TransitionCostMatrix[0 * 6 + i] = 1.0f; // High cost from None
    for(int i=0; i<6; ++i) TransitionCostMatrix[i * 6 + 0] = 1.0f; // High cost to None

    // Row 1: Idle (From Idle)
    TransitionCostMatrix[1 * 6 + 1] = 0.00f; // To Idle
    TransitionCostMatrix[1 * 6 + 2] = 0.05f; // To Chase
    TransitionCostMatrix[1 * 6 + 3] = 0.10f; // To Attack
    TransitionCostMatrix[1 * 6 + 4] = 0.05f; // To Retreat
    TransitionCostMatrix[1 * 6 + 5] = 0.15f; // To Flank

    // Row 2: Chase (From Chase)
    TransitionCostMatrix[2 * 6 + 1] = 0.10f; // To Idle
    TransitionCostMatrix[2 * 6 + 2] = 0.00f; // To Chase
    TransitionCostMatrix[2 * 6 + 3] = 0.05f; // To Attack
    TransitionCostMatrix[2 * 6 + 4] = 0.15f; // To Retreat
    TransitionCostMatrix[2 * 6 + 5] = 0.10f; // To Flank

    // Row 3: Attack (From Attack)
    TransitionCostMatrix[3 * 6 + 1] = 0.15f; // To Idle
    TransitionCostMatrix[3 * 6 + 2] = 0.10f; // To Chase
    TransitionCostMatrix[3 * 6 + 3] = 0.00f; // To Attack
    TransitionCostMatrix[3 * 6 + 4] = 0.20f; // To Retreat
    TransitionCostMatrix[3 * 6 + 5] = 0.15f; // To Flank

    // Row 4: Retreat (From Retreat)
    TransitionCostMatrix[4 * 6 + 1] = 0.10f; // To Idle
    TransitionCostMatrix[4 * 6 + 2] = 0.15f; // To Chase
    TransitionCostMatrix[4 * 6 + 3] = 0.20f; // To Attack
    TransitionCostMatrix[4 * 6 + 4] = 0.00f; // To Retreat
    TransitionCostMatrix[4 * 6 + 5] = 0.25f; // To Flank

    // Row 5: Flank (From Flank)
    TransitionCostMatrix[5 * 6 + 1] = 0.15f; // To Idle
    TransitionCostMatrix[5 * 6 + 2] = 0.10f; // To Chase
    TransitionCostMatrix[5 * 6 + 3] = 0.10f; // To Attack
    TransitionCostMatrix[5 * 6 + 4] = 0.15f; // To Retreat
    TransitionCostMatrix[5 * 6 + 5] = 0.00f; // To Flank
}
