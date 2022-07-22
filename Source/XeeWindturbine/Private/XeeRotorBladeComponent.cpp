// Copyright (c) Iwer Petersen. All rights reserved.


#include "XeeRotorBladeComponent.h"

#include "DrawDebugHelpers.h" 


// Sets default values for this component's properties
UXeeRotorBladeComponent::UXeeRotorBladeComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
	PitchSpeed = 5;
	AngleOfAttack = 15;
	CurrentPitch = 90;

	PitchCurve = CreateDefaultSubobject<UCurveFloat>(TEXT("PitchCurve"));
	PitchCurve->FloatCurve.AddKey(0, 0);
	PitchCurve->FloatCurve.AddKey(1, 1);
	PitchTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("PitchTimeline"));
	PitchTimeline->SetTimelineLength(1);
	PitchTimelineFunction.BindUFunction(this, TEXT("PitchTimelineCallback"));
	PitchTimeline->AddInterpFloat(PitchCurve, PitchTimelineFunction, TEXT("PercentPitch"));

	BladeTip = CreateDefaultSubobject<USceneComponent>(TEXT("BladeTip"));
	BladeTip->SetupAttachment(Anchor);

	EdgeAudio = CreateDefaultSubobject<UAudioComponent>("EdgeAudio");
	FrontTipAudio = CreateDefaultSubobject<UAudioComponent>("FrontTipAudio");
	BackTipAudio = CreateDefaultSubobject<UAudioComponent>("BackTipAudio");

	EdgeAudio->SetupAttachment(Anchor);
	FrontTipAudio->SetupAttachment(Anchor);
	BackTipAudio->SetupAttachment(Anchor);
}

void UXeeRotorBladeComponent::UpdatePitch(const float NewPitch) const
{
	if (!bSuppressRotation) {
		const FRotator Rot(NewPitch, 0, 0);
		Anchor->SetRelativeRotation(Rot);
	}
}


// Called when the game starts
void UXeeRotorBladeComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UXeeRotorBladeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
	if (State != EWindTurbineState::Stopped
		&& State != EWindTurbineState::SafetyStopped) {
		UpdatePitch(CurrentPitch);
	}
}

void UXeeRotorBladeComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

	SetRelativeLocation(FVector(0, -WingRadialOffset, 0));
	UpdatePitch(CurrentPitch);
}

void UXeeRotorBladeComponent::PitchTimelineCallback(const float Val)
{
	CurrentPitch = PitchStart + Val * (TargetPitch - PitchStart);
}

void UXeeRotorBladeComponent::InitPitchCorrection(const float EffectiveSpeed, const float EffectiveAngle, const TEnumAsByte<EWindTurbineState> NewState)
{
	UE_LOG(LogTemp, Warning, TEXT("UXeeRotorBladeComponent::InitPitchCorrection(effSpeed:%f effAngle:%f)"), EffectiveSpeed, EffectiveAngle);

	State = NewState;
	PitchTimeline->Stop();

	float TargetAngle = EffectiveAngle + AngleOfAttack;

	// override in stopping/stopped states to 90°
	if (State == EWindTurbineState::Stopped || State == EWindTurbineState::Stopping
		|| State == EWindTurbineState::SafetyStopping || State == EWindTurbineState::SafetyStopped) {
		TargetAngle = 90;
	}

	PitchStart = CurrentPitch;
	TargetPitch = TargetAngle;
	const float PitchDiff = FMath::Abs(CurrentPitch - TargetAngle);
	if (PitchDiff > 0) {
		const float SecondsToPitch = PitchDiff / PitchSpeed;
		PitchTimeline->SetPlayRate(1 / SecondsToPitch);
		PitchTimeline->PlayFromStart();
	}
}

void UXeeRotorBladeComponent::PostLoad()
{
	Super::PostLoad();

	EdgeAudio->SetRelativeLocation(EdgeSynthLocation);
	BackTipAudio->SetRelativeLocation(TipSynthLocation);
	BackTipAudio->SetRelativeRotation(FRotator(0.0,180.0,0.0));
	FrontTipAudio->SetRelativeLocation(TipSynthLocation);
}
