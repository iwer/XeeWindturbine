// Copyright (c) Iwer Petersen. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "XeeWindturbineComponent.h"
#include "Curves/CurveFloat.h" 
#include "Components/TimelineComponent.h" 
#include "Components/PointLightComponent.h"
#include "WindturbineState.h"

#include "XeeTurbineHouseComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class XEEWINDTURBINE_API UXeeTurbineHouseComponent : public UXeeWindturbineComponent
{
	GENERATED_BODY()

public:	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TurbineParameters", Meta = (ClampMin = ".1"))
	float AzimuthSpeed;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TurbineParameters")
	float AzimuthWindDirectionTolerance;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TurbineDebug")
	bool bSuppressRotation;

	// aerial warning lights
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Aerial Warning Lights")
	TArray<FVector> AWLPositions;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Aerial Warning Lights")
	bool bAWLEnabled;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Noise Synthesis")
	FVector AzimuthSynthLocation;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Noise Synthesis")
	USoundAttenuation * AttenuationSettings;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Noise Synthesis")
	USoundClass * SoundClass;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Aerial Warning Lights")
	TArray<UPointLightComponent*> AWLs;
	
	// Sets default values for this component's properties
	UXeeTurbineHouseComponent();

	UFUNCTION(BlueprintCallable)
	void InitAzimuthCorrection(float WindDirection, TEnumAsByte<EWindTurbineState> NewState);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void CreateAWLPointLights();
	virtual void OnComponentCreated() override;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	float CurrentAzimuth;

	
private:

	float AzimuthStart;
	float AzimuthDiff;
	float AWLState;
	TEnumAsByte<EWindTurbineState> State;

	UPROPERTY()
	UCurveFloat* BlinkCurve;
	UPROPERTY()
	UCurveFloat* AzimuthCurve;
	UPROPERTY()
	UTimelineComponent* AzimuthTimeline;
	UPROPERTY()
	UTimelineComponent* BlinkTimeline;

	FOnTimelineFloat BlinkTimelineFunction{};
	FOnTimelineFloat AzimuthTimelineFunction{};
	FOnTimelineEvent AzimuthCorrectionFinished{};

	UFUNCTION()
	void BlinkTimelineCallback(const float Val);
	UFUNCTION()
	void AzimuthTimelineCallback(float Val);
	void UpdateAzimuth(float NewAzimuth);

	UFUNCTION()
	void OnAzimuthCorrectionFinished() const;

};



