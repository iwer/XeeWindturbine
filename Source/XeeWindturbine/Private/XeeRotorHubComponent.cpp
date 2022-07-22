// Copyright (c) Iwer Petersen. All rights reserved.


#include "XeeRotorHubComponent.h"


// Sets default values for this component's properties
UXeeRotorHubComponent::UXeeRotorHubComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
	CurrentRpm = 0;
	RpmChangePerSec = 1;
	bSuppressRotation = false;

	RpmAdaptionCurve = CreateDefaultSubobject<UCurveFloat>(TEXT("RPMAdaptionCurve"));
	RpmAdaptionCurve->FloatCurve.AddKey(0, 0);
	RpmAdaptionCurve->FloatCurve.AddKey(1, 1);
	RpmAdaptionTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("RPMTimeline"));
	RpmAdaptionTimeline->SetTimelineLength(1);
	

	StartTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("StartTimeline"));
	StartTimelineFunction.BindUFunction(this, TEXT("StartTimelineCallback"));

	StopTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("StopTimeline"));
	StopTimelineFunction.BindUFunction(this, TEXT("StopTimelineCallback"));
}

// Called when the game starts
void UXeeRotorHubComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	StartTimeline->SetTimelineFinishedFunc(StartTimelineFinishedFunction);
	StopTimeline->SetTimelineFinishedFunc(StopTimelineFinishedFunction);
	RpmAdaptionTimelineFunc.BindUFunction(this, TEXT("RpmAdaptionCallback"));
	RpmAdaptionTimeline->AddInterpFloat(RpmAdaptionCurve, RpmAdaptionTimelineFunc, TEXT("Percent RPM changed"));
}

void UXeeRotorHubComponent::OnComponentCreated()
{
	if (StartupCurve) {
		StartTimeline->AddInterpFloat(StartupCurve, StartTimelineFunction, TEXT("PercentStartRPM"));
		StartTimeline->SetTimelineLength(StartupCurve->FloatCurve.GetLastKey().Time);
	}
	if (ShutdownCurve) {
		StopTimeline->AddInterpFloat(ShutdownCurve, StopTimelineFunction, TEXT("PercentStopRPM"));
		StartTimeline->SetTimelineLength(ShutdownCurve->FloatCurve.GetLastKey().Time);
	}
}

// Called every frame
void UXeeRotorHubComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
	if (!bSuppressRotation) {
		if (State != EWindTurbineState::Stopped && State != EWindTurbineState::SafetyStopped) {
			// animate rotor
			const float DegreePerSec = CurrentRpm / 60.0 * -360.0;
			const FRotator Rot(0, 0, DeltaTime * DegreePerSec);
			AddRelativeRotation(Rot);
		}
	}
}

void UXeeRotorHubComponent::InitRpmCorrection(const float TargetRpm, TEnumAsByte<EWindTurbineState> NewState)
{
	UE_LOG(LogTemp, Warning, TEXT("UXeeRotorHubComponent::InitRPMCorrection(TargetRPM: %f, NewState: %s) CurrentRPM: %f, CurrentState: %s"), 
		TargetRpm, *UEnum::GetValueAsString<EWindTurbineState>(NewState), 
		CurrentRpm, *UEnum::GetValueAsString<EWindTurbineState>(State));
	RpmAdaptionTimeline->Stop();

	if (State == EWindTurbineState::Running) {
		UE_LOG(LogTemp, Warning, TEXT("UXeeRotorHubComponent: Currently RUNNING"))
		if (NewState == EWindTurbineState::Running) {
			UE_LOG(LogTemp, Warning, TEXT("UXeeRotorHubComponent: Next RUNNING"))
			RpmDiff = TargetRpm - CurrentRpm;
			RpmStart = CurrentRpm;
			const float SecondsToChange = FMath::Abs(RpmDiff) / RpmChangePerSec;
			RpmAdaptionTimeline->SetPlayRate(1 / SecondsToChange);
			RpmAdaptionTimeline->PlayFromStart();
		}
		else if (NewState == EWindTurbineState::Stopping) {
			UE_LOG(LogTemp, Warning, TEXT("UXeeRotorHubComponent: Next STOPPING"))
			RpmStart = CurrentRpm;
			ShutdownRotor();
		}
		else if (NewState == EWindTurbineState::SafetyStopping) {
			UE_LOG(LogTemp, Warning, TEXT("UXeeRotorHubComponent: Next SAFETYSTOPPING"))
			RpmStart = CurrentRpm;
			ShutdownRotor();
		}
	}
	else if (State == EWindTurbineState::Stopping) {
		if (NewState == EWindTurbineState::Stopping) {

		}
		else if (NewState == EWindTurbineState::Starting) {
			RpmStart = CurrentRpm;
			RpmDiff = TargetRpm - CurrentRpm;
			StartupRotor();
		}
		else if (NewState == EWindTurbineState::Stopped) {

		}
	}
	else if (State == EWindTurbineState::SafetyStopping) {
		if (NewState == EWindTurbineState::SafetyStopping) {

		}
		else if (NewState == EWindTurbineState::Starting) {
			RpmStart = CurrentRpm;
			RpmDiff = TargetRpm - CurrentRpm;
			StartupRotor();
		}
		else if (NewState == EWindTurbineState::SafetyStopped) {

		}
	}
	else if (State == EWindTurbineState::Starting) {
		if (NewState == EWindTurbineState::Running) {

		}
		else if (NewState == EWindTurbineState::Stopping) {
			RpmStart = CurrentRpm;
			ShutdownRotor();
		}
		else if (NewState == EWindTurbineState::Starting) {

		}
	}
	else if (State == EWindTurbineState::Stopped) {
		if (NewState == EWindTurbineState::Starting) {
			RpmStart = CurrentRpm;
			RpmDiff = TargetRpm - CurrentRpm;
			StartupRotor();
		}
		else if (NewState == EWindTurbineState::Stopped) {

		}
	}
	else if (State == EWindTurbineState::SafetyStopped) {
		if (NewState == EWindTurbineState::Starting) {
			RpmStart = CurrentRpm;
			RpmDiff = TargetRpm - CurrentRpm;
			StartupRotor();
		}
		else if (NewState == EWindTurbineState::SafetyStopped) {

		}
	}
	State = NewState;
}

float UXeeRotorHubComponent::GetCurrentRpm()
{
	return CurrentRpm;
}

void UXeeRotorHubComponent::StartupRotor()
{
	UE_LOG(LogTemp, Warning, TEXT("UXeeRotorHubComponent::StartupRotor()"));
	StopTimeline->Stop();
	StartTimeline->Stop();
	// restart start timeline
	StartTimeline->PlayFromStart();
}

void UXeeRotorHubComponent::ShutdownRotor()
{
	UE_LOG(LogTemp, Warning, TEXT("UXeeRotorHubComponent::ShutdowRotor()"));
	StopTimeline->Stop();
	StartTimeline->Stop();
	// restart stop timeline
	StopTimeline->PlayFromStart();
}

void UXeeRotorHubComponent::SetStartupFinishedFunction(FOnTimelineEvent& fct)
{
	StartTimelineFinishedFunction = fct;
	StartTimeline->SetTimelineFinishedFunc(StartTimelineFinishedFunction);
}

void UXeeRotorHubComponent::SetShutdownFinishedFunction(FOnTimelineEvent& fct)
{
	StopTimelineFinishedFunction = fct;
	StopTimeline->SetTimelineFinishedFunc(StopTimelineFinishedFunction);
}

void UXeeRotorHubComponent::SetStartupCurve(UCurveFloat* curve)
{
	if (curve) {
		StartupCurve = curve;
		StartTimeline->AddInterpFloat(StartupCurve, StartTimelineFunction, TEXT("PercentStartRPM"));
		StartTimeline->SetTimelineLength(StartupCurve->FloatCurve.GetLastKey().Time);
	}
}

void UXeeRotorHubComponent::SetShutdownCurve(UCurveFloat* curve)
{
	if (curve) {
		ShutdownCurve = curve;
		StopTimeline->AddInterpFloat(ShutdownCurve, StopTimelineFunction, TEXT("PercentStopRPM"));
		StartTimeline->SetTimelineLength(ShutdownCurve->FloatCurve.GetLastKey().Time);
	}
}

void UXeeRotorHubComponent::SetStateDirect(TEnumAsByte<EWindTurbineState>& NewState)
{
	State = NewState;
}

void UXeeRotorHubComponent::RpmAdaptionCallback(const float Val)
{
	CurrentRpm = RpmStart + Val * RpmDiff;
}

void UXeeRotorHubComponent::StopTimelineCallback(const float Val)
{
	CurrentRpm = Val * RpmStart;
}

void UXeeRotorHubComponent::StartTimelineCallback(const float Val)
{
	CurrentRpm = RpmStart + Val * RpmDiff;
}

