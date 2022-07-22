// Copyright (c) Iwer Petersen. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Windshear.h"
#include "Components/StaticMeshComponent.h"
#include "XeeWindturbineComponent.h"
#include "XeeTurbineHouseComponent.h"
#include "XeeRotorHubComponent.h"
#include "XeeRotorBladeComponent.h"

#include "XeeWindturbineActor.generated.h"



UCLASS(Blueprintable, BlueprintType)
class XEEWINDTURBINE_API AXeeWindturbineActor : public AActor {
    GENERATED_BODY()
public:
    // MeshComponents

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="RelativeLocation")
    USceneComponent * RelativeLocation;    
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Meshes")
    UStaticMesh * FoundationMesh;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Meshes")
    UStaticMesh * EntryMesh;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Meshes")
    UStaticMesh * TowerMesh;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Meshes")
    UStaticMesh * HouseMesh;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Meshes")
    UStaticMesh * RotorhubMesh;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Meshes")
    UStaticMesh * WingMesh;

    // Windshear variables
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Windshear")
    TEnumAsByte<EWindshearGroundType> WindshearGroundtype;

    // Curves
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Curves")
    UCurveFloat * RPMCurve;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="TurbineParameters")
    float AngleOfAttack;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="TurbineParameters")
    float EntryRotation;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="TurbineParameters")
    int32 NumWings;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TurbineParameters")
    float HouseHeight;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TurbineParameters")
    FVector HubOffset;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TurbineParameters")
    float HubTilt;

    // synth variables
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Noise Synthesis")
    FVector TipSynthLocationOffset;

    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Noise Synthesis")
    FVector EdgeSynthLocationOffset;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Noise Synthesis")
    FRotator EdgeSynthRotationOffset;
/*
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Noise Synthesis")
    UModularSynthPresetBank * PresetBank;
*/    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Noise Synthesis")
    USoundAttenuation* EdgeAttenuationSettings;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Noise Synthesis")
    USoundAttenuation* TipAttenuationSettings;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Noise Synthesis")
    USoundAttenuation* AzimuthAttenuationSettings;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Noise Synthesis")
    USoundClass * TipSoundClass;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Noise Synthesis")
    USoundClass * EdgeSoundClass;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Noise Synthesis")
    USoundClass * AzimuthSoundClass;
    
    // Components (can be private after developement)
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
    UXeeWindturbineComponent* Foundation;
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
    UXeeWindturbineComponent* Tower;
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
    UXeeWindturbineComponent* Entry;
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
    UXeeTurbineHouseComponent* House;
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
    UXeeRotorHubComponent* Hub;
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
    TArray<UXeeRotorBladeComponent*> Wings;

    AXeeWindturbineActor();

    UFUNCTION(BlueprintCallable)
    void OnWindParametersUpdate(float Speed, float Direction);

    UFUNCTION(BlueprintCallable)
    void OnAtmosphereParametersUpdate(float Temperature, float RelativeHumidity, float AirPressure);

    UFUNCTION(BlueprintCallable)
    float  GetEffectiveSpeedAtComponent(USceneComponent* Component);

    UFUNCTION(BlueprintCallable)
    float  GetEffectiveWindStreamAngleAtComponent(USceneComponent* Component);

    UFUNCTION(BlueprintCallable)
    float GetWingRotation(int32 WingIndex);

    UPROPERTY(BlueprintReadOnly, EditAnywhere)
    TEnumAsByte<EWindTurbineState> State;

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void OnConstruction(const FTransform &Transform) override;
    
    UFUNCTION()
    void OnStartupFinished();
    UFUNCTION()
    void OnShutdownFinished();

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNoiseStateChangedEvent,bool, AudioState);
    UPROPERTY(BlueprintAssignable)
    FNoiseStateChangedEvent NoiseStateChanged;
    
private:
    
    float LastReportedWindSpeed;
    float LastReportedTemperature;
    float LastReportedRelativeHumidity;
    float LastReportedAirPressure;

    
    
    void InitPitchCorrection(float Speed);
    float RotationalSpeedAtRadius(float Radius) const;
    float GetWindshearMultiplier(float BaseSpeed, float Height) const;
    float GetEffectiveSpeedAtComponent(USceneComponent*Component, float WindSpeed) const;
    float GetEffectiveWindStreamAngleAtComponent(USceneComponent* Component, float WindSpeed) const;

    void StartAudio();
    void StopAudio();
    FVector GetPlayerPosition() const;
};
