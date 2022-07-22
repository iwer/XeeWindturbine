// Copyright (c) Iwer Petersen. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "XeeWindturbineComponent.h"
#include "Curves/CurveFloat.h" 
#include "Components/TimelineComponent.h" 
#include "WindturbineState.h"

#include "XeeRotorHubComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class XEEWINDTURBINE_API UXeeRotorHubComponent : public UXeeWindturbineComponent
{
	GENERATED_BODY()

public:	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Curves")
		UCurveFloat* StartupCurve;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Curves")
		UCurveFloat* ShutdownCurve;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TurbineParameters")
		float RpmChangePerSec;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TurbineDebug")
		bool bSuppressRotation;

	// Sets default values for this component's properties
	UXeeRotorHubComponent();

	UFUNCTION(BlueprintCallable)
	void InitRpmCorrection(float TargetRpm, TEnumAsByte<EWindTurbineState> NewState);
	UFUNCTION(BlueprintCallable)
	float GetCurrentRpm();


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void OnComponentCreated() override;

	void SetStartupFinishedFunction(FOnTimelineEvent& fct);
	void SetShutdownFinishedFunction(FOnTimelineEvent& fct);
	void SetStartupCurve(UCurveFloat* curve);
	void SetShutdownCurve(UCurveFloat* curve);
	void SetStateDirect(TEnumAsByte<EWindTurbineState>& NewState);
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	float CurrentRpm;

private:
	float RpmStart;
	float RpmDiff;
	EWindTurbineState State;

	UPROPERTY()
	UCurveFloat* RpmAdaptionCurve;
	UPROPERTY()
	UTimelineComponent* RpmAdaptionTimeline;
	UPROPERTY()
	UTimelineComponent* StartTimeline;
	UPROPERTY()
	UTimelineComponent* StopTimeline;

	FOnTimelineFloat RpmAdaptionTimelineFunc{};
	FOnTimelineFloat StartTimelineFunction{};
	FOnTimelineFloat StopTimelineFunction{};
	FOnTimelineEvent StartTimelineFinishedFunction{};
	FOnTimelineEvent StopTimelineFinishedFunction{};

	UFUNCTION()
	void RpmAdaptionCallback(float Val);
	UFUNCTION()
	void StopTimelineCallback(float Val);
	UFUNCTION()
	void StartTimelineCallback(float Val);

	void StartupRotor();
	void ShutdownRotor();
};
