// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "DemonstrationSubsystem.h"
#include "DepthrunLogChannels.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"

void UDemonstrationSubsystem::Initialize(FSubsystemCollectionBase& Collection) { Super::Initialize(Collection); }
void UDemonstrationSubsystem::Deinitialize()
{
	if (bCSVRecording) { EndCSVRecording(); }
	Super::Deinitialize();
}

void UDemonstrationSubsystem::EnterDemoMode()
{
	// TODO (Stage 11): GetWorld()->GetWorldSettings()->SetTimeDilation(0.1f)
	bDemoModeActive = true;
	UE_LOG(LogAdaptiveBehavior, Log, TEXT("[Demo] Demo mode ON"));
}

void UDemonstrationSubsystem::ExitDemoMode()
{
	// TODO (Stage 11): restore time dilation to 1.0
	bDemoModeActive = false;
	UE_LOG(LogAdaptiveBehavior, Log, TEXT("[Demo] Demo mode OFF"));
}

void UDemonstrationSubsystem::SetStepMode(bool bEnabled)
{
	bStepModeEnabled = bEnabled;
	// TODO (Stage 11): pause/unpause adaptive timers
}

void UDemonstrationSubsystem::RequestStep()
{
	// TODO (Stage 11): trigger one evaluation on all adaptive enemies
}

void UDemonstrationSubsystem::BeginCSVRecording(const FString& Filename)
{
	CSVFilePath = FPaths::ProjectSavedDir() / Filename;
	CSVRows.Reset();
	CSVRows.Add(TEXT("TFinal,Confidence,W0,W1,W2,W3,W4,W5,ChosenState,Pattern"));
	bCSVRecording = true;
	UE_LOG(LogAdaptiveBehavior, Log, TEXT("[Demo] CSV recording → %s"), *CSVFilePath);
}

void UDemonstrationSubsystem::EndCSVRecording()
{
	bCSVRecording = false;
	FString Content = FString::Join(CSVRows, TEXT("\n"));
	FFileHelper::SaveStringToFile(Content, *CSVFilePath);
	UE_LOG(LogAdaptiveBehavior, Log, TEXT("[Demo] CSV saved: %d rows"), CSVRows.Num() - 1);
}

void UDemonstrationSubsystem::RecordEvaluationRow(float TFinal, float Confidence,
	const TArray<float>& Weights, const FString& ChosenState, const FString& Pattern)
{
	if (!bCSVRecording) return;
	FString Row = FString::Printf(TEXT("%.4f,%.4f"), TFinal, Confidence);
	for (int32 i = 0; i < 6; ++i)
	{
		Row += FString::Printf(TEXT(",%.4f"), Weights.IsValidIndex(i) ? Weights[i] : 0.f);
	}
	Row += FString::Printf(TEXT(",%s,%s"), *ChosenState, *Pattern);
	CSVRows.Add(Row);
}
