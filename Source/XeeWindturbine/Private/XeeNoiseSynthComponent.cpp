// Copyright (c) Iwer Petersen. All rights reserved.


#include "XeeNoiseSynthComponent.h"

// Sets default values for this component's properties
UXeeNoiseSynthComponent::UXeeNoiseSynthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	bActive = false;
	
	TipGainFactor = .3;
	TipMaxAmplitudeDegree = 120;
	TipMaxFrequencyDegree = 120;
	TipFrequencyModulationFactor = .5;
	TipAmplitudeModulationFactor = .5;
	TipCenterFrequency = 4500;

	EdgeLowPassBaseFrequency = 4000;
	EdgeGainFactor = 1;
	EdgeStallGainFactor = 5;
	EdgeMaxFrequencyDegree = 120;
	EdgeMaxGainDegree = 120;
	EdgeFrequencyModulationFactor = .5;
	EdgeAmplitudeModulationFactor = .5;

	FreqOffset = FMath::FRandRange(.95, 1.05);
	
	Audio = CreateDefaultSubobject<UAudioComponent>("Audio");
	Audio->AttenuationSettings = AttenuationSettings;
	Audio->SetupAttachment(this);
}

void UXeeNoiseSynthComponent::UpdateSynthState(const float PlayerDistance, const TEnumAsByte<EWindTurbineState> State)
{
	const bool ShouldBeOff = (PlayerDistance > AttenuationSettings->Attenuation.FalloffDistance * 1.2 || State == EWindTurbineState::Stopped);

	if( ShouldBeOff && bActive)
	{
		SetSynthActive(false);
	} else if(!ShouldBeOff && !bActive)
	{
		SetSynthActive(true);
	}
}

void UXeeNoiseSynthComponent::ModulateEdgeParameters(const float WingRotation, const float EffectiveAirspeed,
													const float EffectiveStreamAngle, float AngleOfAttack,
													const float PlayerDistance, const TEnumAsByte<EWindTurbineState> State)
{
	UpdateSynthState(PlayerDistance, State);

	const float WingRot = FGenericPlatformMath::Fmod(WingRotation, 360);
	const float SpeedScale = EffectiveAirspeed / 100; // gives around 1.0 at approximate top speed
	const float RotFreqScale = EdgeFrequencyModulationFactor * (1 + FGenericPlatformMath::Cos(
		FMath::DegreesToRadians(WingRot + EdgeMaxFrequencyDegree)) / 2); // maximum is at offset

	const float SpeedFreq = SpeedScale * FreqOffset * EdgeLowPassBaseFrequency;
	const float FilterFreq = SpeedFreq + (RotFreqScale * SpeedFreq);

	//float gain1 = speedScale * EdgeGainFactor;
	const float Gain2 = EffectiveStreamAngle * EdgeStallGainFactor;

	const float Gain1 = SpeedScale * EdgeAmplitudeModulationFactor * (1 + FGenericPlatformMath::Cos(
		FMath::DegreesToRadians(WingRot + EdgeMaxGainDegree)) / 2);
	
	Synth->SetFilterFrequency(FilterFreq);
	Synth->SetOscGain(0, Gain1);
	Synth->SetOscGain(1, Gain2);
	
}

void UXeeNoiseSynthComponent::ModulateTipParameters(const float WingRotation, const float EffectiveAirspeed,
                                                    float EffectiveStreamAngle, const float PlayerDistance,
                                                    const TEnumAsByte<EWindTurbineState> State)
{
	UpdateSynthState(PlayerDistance, State);

	const float WingRot = FGenericPlatformMath::Fmod(WingRotation, 360);
	const float SpeedScale = EffectiveAirspeed / 100; // gives around 1.0 at approximate top speed
		
	const float RotFreqScale = TipFrequencyModulationFactor * (1 + FGenericPlatformMath::Cos(
		FMath::DegreesToRadians(WingRot + TipMaxFrequencyDegree)) / 2); // maximum is at offset

	const float SpeedFreq = TipCenterFrequency * FreqOffset * SpeedScale;
	const float FilterFreq = SpeedFreq + (RotFreqScale * SpeedFreq);

	const float Gain = TipGainFactor * SpeedScale * TipAmplitudeModulationFactor * (1 + FGenericPlatformMath::Cos(
			FMath::DegreesToRadians(WingRot + TipMaxAmplitudeDegree)) / 2);

	Synth->SetFilterFrequency(FilterFreq);
	Synth->SetOscGain(0, Gain);
	
}

void UXeeNoiseSynthComponent::ModulateTipParametersDoppler(const float EffectiveAirspeed, const float DopplerFactor,
												const float PlayerDistance, const TEnumAsByte<EWindTurbineState> State)
{
	UpdateSynthState(PlayerDistance, State);
	const float SpeedScale = FMath::GetMappedRangeValueClamped(FVector2D(25.0,80.0), FVector2D(0.0,1.0), EffectiveAirspeed);
	
	Synth->SetFilterFrequency(DopplerFactor * TipCenterFrequency * FreqOffset );
	Synth->SetOscGain(0,DopplerFactor * SpeedScale);
	//UE_LOG(LogTemp, Warning, TEXT("Tip Gain: %f*%f=%f"), DopplerFactor, SpeedScale, DopplerFactor * SpeedScale)
}

void UXeeNoiseSynthComponent::SetSynthActive(const bool bState)
{
	// dont do anything if were already there
	if(bState == bActive)
		return;
	
	if (Synth) {
		if (bState == true) {
			//UE_LOG(LogTemp, Warning, TEXT("UXeeNoiseSynthComponent: %s Note ON"), *GetName());
			Synth->Activate();
			Synth->SetSynthPreset(NoisePreset);
			Synth->SetFilterFrequency(0);
			Synth->SetOscGain(0, 0);
			Synth->SetOscGain(1, 0);
			Synth->NoteOn(60, 100, -1);
			bActive = true;
		}
		else
		{
			//UE_LOG(LogTemp, Warning, TEXT("UXeeNoiseSynthComponent: %s Note OFF"), *GetName());
			Synth->NoteOff(60, true, true);
			Synth->Deactivate();	
			bActive = false;
		}
	}
}


// Called when the game starts
void UXeeNoiseSynthComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// ...
	Synth = NewObject<UModularSynthComponent>(this);
	Synth->SetRelativeLocation(FVector::ZeroVector);
	Synth->SetupAttachment(this);
	Synth->RegisterComponent();
	Synth->SetSynthPreset(NoisePreset);
	Synth->bAllowSpatialization = true;
	Synth->VoiceCount = 1;
	
	if (AttenuationSettings)
	{
		Synth->AttenuationSettings = AttenuationSettings;
	}
	if(SoundClass)
		Synth->SoundClass = SoundClass;
	
}


// Called every frame
void UXeeNoiseSynthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...

}
