// Copyright (c) Iwer Petersen. All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "WindturbineState.h"
#include "Components/SceneComponent.h"
#include "SynthComponents/EpicSynth1Component.h"

#include "XeeNoiseSynthComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class XEEWINDTURBINE_API UXeeNoiseSynthComponent : public USceneComponent
{
	GENERATED_BODY()

public:
    // Synths
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Noise Synthesis")
    float TipMaxAmplitudeDegree;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Noise Synthesis")
        float TipGainFactor;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Noise Synthesis")
    float TipMaxFrequencyDegree;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Noise Synthesis")
    float TipFrequencyModulationFactor;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Noise Synthesis")
    float TipAmplitudeModulationFactor;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Noise Synthesis")
    float TipCenterFrequency;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Noise Synthesis")
    float EdgeLowPassBaseFrequency;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Noise Synthesis")
        float EdgeGainFactor;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Noise Synthesis")
        float EdgeStallGainFactor;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Noise Synthesis")
        float EdgeFrequencyModulationFactor;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Noise Synthesis")
        float EdgeMaxFrequencyDegree;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Noise Synthesis")
        float EdgeMaxGainDegree;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Noise Synthesis")
        float EdgeAmplitudeModulationFactor;

    // Synth Preset
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Noise Synthesis")
    FModularSynthPreset NoisePreset;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Noise Synthesis")
    USoundAttenuation * AttenuationSettings;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Noise Synthesis")
    USoundClass * SoundClass;
    
    UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Noise Synthesis")
    UModularSynthComponent* Synth;

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="NoiseSynthesis")
    USoundCue * SoundCue;

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="NoiseSynthesis")
    UAudioComponent * Audio;
    
	// Sets default values for this component's properties
	UXeeNoiseSynthComponent();
    

    UFUNCTION()
    void ModulateEdgeParameters(float WingRotation, float EffectiveAirspeed, float EffectiveStreamAngle, float AngleOfAttack, float PlayerDistance, TEnumAsByte<EWindTurbineState> State);
    UFUNCTION()
    void ModulateTipParameters(float WingRotation, float EffectiveAirspeed, float EffectiveStreamAngle, float PlayerDistance, TEnumAsByte<EWindTurbineState> State);
    UFUNCTION()
    void ModulateTipParametersDoppler(const float EffectiveAirspeed, float DopplerFactor, float PlayerDistance, TEnumAsByte<EWindTurbineState> State);
    UFUNCTION()
    void SetSynthActive(bool bState);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
private:
    bool bActive;
    void UpdateSynthState(float PlayerDistance, TEnumAsByte<EWindTurbineState> State);
    float FreqOffset;
};
