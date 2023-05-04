// Copyright (c) Iwer Petersen. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "XeeWindturbineComponent.h"
#include "Curves/CurveFloat.h" 
#include "Components/TimelineComponent.h" 
#include "WindturbineState.h"
#include "MathUtils.h"

#include "XeeRotorBladeComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class XEEWINDTURBINE_API UXeeRotorBladeComponent : public UXeeWindturbineComponent
{
	GENERATED_BODY()

public:	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TurbineParameters", Meta = (ClampMin = ".1"))
	float PitchSpeed;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TurbineParameters")
	float AngleOfAttack;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TurbineParameters")
	float WingRadialOffset;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TurbineDebug")
	bool bSuppressRotation;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "BladeGeometry")
	USceneComponent* BladeTip;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Noise Synthesis")
	FVector TipSynthLocation;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Noise Synthesis")
	FVector EdgeSynthLocation;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Noise Synthesis")
	FRotator EdgeSynthRotation;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="NoiseSynthesis")
	UAudioComponent * EdgeAudio;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="NoiseSynthesis")
	UAudioComponent * FrontTipAudio;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="NoiseSynthesis")
	UAudioComponent * BackTipAudio;
	
	/*
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Noise Synthesis")
	UXeeNoiseSynthComponent* EdgeSynth;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Noise Synthesis")
	UXeeNoiseSynthComponent* BackTipSynth;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Noise Synthesis")
	UXeeNoiseSynthComponent* FrontTipSynth;

	Differentiator Differentiator;
	*/

	// Sets default values for this component's properties
	UXeeRotorBladeComponent();

	UFUNCTION(BlueprintCallable)
		void InitPitchCorrection(float EffectiveSpeed, float EffectiveAngle, TEnumAsByte<EWindTurbineState> NewState);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void OnComponentCreated() override;
	virtual void PostLoad() override;
	
private:
	float CurrentPitch;
	float PitchStart;
	float TargetPitch;
	TEnumAsByte<EWindTurbineState> State;

	UPROPERTY()
	UCurveFloat* PitchCurve;
	UPROPERTY()
	UTimelineComponent* PitchTimeline;
	FOnTimelineFloat PitchTimelineFunction{};
	UFUNCTION()
	void PitchTimelineCallback(float Val);
	void UpdatePitch(float NewPitch) const;
};
