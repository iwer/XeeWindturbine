// Copyright (c) Iwer Petersen. All rights reserved.

#include "XeeWindturbineActor.h"
//#include "MathUtils.h"
//#include "PhysicsUtils.h"

AXeeWindturbineActor::AXeeWindturbineActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // Members
    WindshearGroundtype = EWindshearGroundType::WSGT_Agricultural1KM;
    AngleOfAttack = 15;
    EntryRotation = 0;
    NumWings = 3;
    HouseHeight = 0;
    HubOffset = FVector(0, 0, 0);
    HubTilt = 0;
    LastReportedWindSpeed = 0;
    LastReportedTemperature = 20;
    LastReportedAirPressure = 1000;
    LastReportedRelativeHumidity = 50;

    State = EWindTurbineState::Stopped;

    USceneComponent * Root = CreateDefaultSubobject<USceneComponent>("RootComponent");
    Root->SetMobility(EComponentMobility::Static);
    SetRootComponent(Root);

    
    RelativeLocation = CreateDefaultSubobject<USceneComponent>("RelLocComponent");
    RelativeLocation->SetupAttachment(Root);
    RelativeLocation->SetRelativeLocation(FVector::ZeroVector);
    RelativeLocation->SetMobility(EComponentMobility::Movable);
    
    // Foundation
    Foundation = CreateDefaultSubobject<UXeeWindturbineComponent>(TEXT("Foundation"));
    Foundation->SetupAttachment(RelativeLocation);
    Foundation->SetMobility(EComponentMobility::Movable);

    Tower = CreateDefaultSubobject<UXeeWindturbineComponent>(TEXT("Tower"));
    Tower->SetupAttachment(RelativeLocation);
    Tower->SetMobility(EComponentMobility::Stationary);

    Entry = CreateDefaultSubobject<UXeeWindturbineComponent>(TEXT("Entry"));
    Entry->SetupAttachment(RelativeLocation);

    House = CreateDefaultSubobject<UXeeTurbineHouseComponent>(TEXT("House"));
    House->SetupAttachment(RelativeLocation);

    Hub = CreateDefaultSubobject<UXeeRotorHubComponent>(TEXT("Hub"));
    Hub->SetupAttachment(House);

    // Blades
    for (int32 i = 0; i < NumWings; ++i) {
        auto Blade = CreateDefaultSubobject<UXeeRotorBladeComponent>(FName(TEXT("Blade"), i));
        Blade->SetupAttachment(Hub);
        Wings.Add(Blade);
    }
}



// Called when the game starts or when spawned
void AXeeWindturbineActor::BeginPlay()
{
    Super::BeginPlay();

    FOnTimelineEvent startupFinishedFct{};
    startupFinishedFct.BindUFunction(this, TEXT("OnStartupFinished"));
    Hub->SetStartupFinishedFunction(startupFinishedFct);
    FOnTimelineEvent shutdownFinishedFct;
    shutdownFinishedFct.BindUFunction(this, TEXT("OnShutdownFinished"));
    Hub->SetShutdownFinishedFunction(shutdownFinishedFct);

}

// Called every frame
void AXeeWindturbineActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AXeeWindturbineActor::OnConstruction(const FTransform &Transform)
{
    // Construct parent after Lat/Lon is set
    Super::OnConstruction(Transform);

    if(FoundationMesh) {
        Foundation->MeshComponent->SetStaticMesh(FoundationMesh);
    }
    if(EntryMesh) {
        Entry->MeshComponent->SetStaticMesh(EntryMesh);
        FRotator rotator(0, EntryRotation, 0);
        Entry->SetRelativeRotation(rotator);
    }
    if(TowerMesh){
        Tower->MeshComponent->SetStaticMesh(TowerMesh);
    }
    if(HouseMesh){
        House->MeshComponent->SetStaticMesh(HouseMesh);
        House->SetRelativeLocation(FVector(0, 0, HouseHeight));
    }
    if(RotorhubMesh){
        Hub->MeshComponent->SetStaticMesh(RotorhubMesh);
        Hub->SetRelativeLocation(HubOffset);
        FRotator rot(HubTilt, 0, 0);
        Hub->SetRelativeRotation(rot);
    }

    House->AttenuationSettings = AzimuthAttenuationSettings;
    House->SoundClass = AzimuthSoundClass;
    
    for (int32 i = 0; i < Wings.Num(); ++i) {
        // set individual blades rotation
        Wings[i]->SetRelativeRotation(FRotator(0, 0, 360.0 / NumWings * i));
        
        Wings[i]->EdgeAudio->AttenuationSettings = EdgeAttenuationSettings;
        Wings[i]->FrontTipAudio->AttenuationSettings = TipAttenuationSettings;
        Wings[i]->BackTipAudio->AttenuationSettings = TipAttenuationSettings;

        Wings[i]->EdgeAudio->SoundClassOverride = EdgeSoundClass;
        Wings[i]->FrontTipAudio->SoundClassOverride = TipSoundClass;
        Wings[i]->BackTipAudio->SoundClassOverride = TipSoundClass;
        
        // set mesh
        if (WingMesh) {
            Wings[i]->MeshComponent->SetStaticMesh(WingMesh);
            FBox WingBounds = WingMesh->GetBoundingBox();
            float TipRadius = WingBounds.GetSize().Y + Wings[i]->WingRadialOffset;

            Wings[i]->BladeTip->SetRelativeLocation(FVector(0, -TipRadius, 0));
            Wings[i]->TipSynthLocation = FVector(0, -TipRadius, 0) + TipSynthLocationOffset;
            Wings[i]->EdgeSynthLocation = FVector(0, -TipRadius / 2, 0) + EdgeSynthLocationOffset;
            Wings[i]->EdgeSynthRotation = FRotator(0,0,90) + EdgeSynthRotationOffset;
        }
    }
}

void AXeeWindturbineActor::OnStartupFinished()
{
    State = EWindTurbineState::Running;
    Hub->SetStateDirect(State);
    UE_LOG(LogTemp, Warning, TEXT("AXeeWindturbineActor: State RUNNING"));
}

void AXeeWindturbineActor::OnShutdownFinished()
{
    if (State == EWindTurbineState::Stopping) {
        State = EWindTurbineState::Stopped;
        UE_LOG(LogTemp, Warning, TEXT("AXeeWindturbineActor: State STOPPED"));
    }
    else if (State == EWindTurbineState::SafetyStopping) {
        State = EWindTurbineState::SafetyStopped;
        UE_LOG(LogTemp, Warning, TEXT("AXeeWindturbineActor: State SAFETYSTOPPED"));
    }
    Hub->SetStateDirect(State);

    StopAudio();
}

void AXeeWindturbineActor::OnWindParametersUpdate(const float Speed, const float Direction) {
    if (!RPMCurve) {
        UE_LOG(LogTemp, Error, TEXT("AXeeWindturbineActor: No RPMCurve!"));
        return;
    }
    LastReportedWindSpeed = Speed;

    const float TargetRpm = RPMCurve->GetFloatValue(Speed);
    UE_LOG(LogTemp, Warning, TEXT("AXeeWindturbineActor: Wind changed: s: %f d:%f. Should be running at %f RPM"), Speed, Direction, TargetRpm);
    if (State == EWindTurbineState::Stopped) {
        if (TargetRpm > 0) {
            // start
            State = EWindTurbineState::Starting;
            UE_LOG(LogTemp, Warning, TEXT("AXeeWindturbineActor: State STARTING"));
            Hub->InitRpmCorrection(TargetRpm, State);
            House->InitAzimuthCorrection(Direction, State);
            InitPitchCorrection(Speed);
            StartAudio();
        }
        else {
            // if were stopped below the threshold changes are uninteresting
        }
    }
    else if (State == EWindTurbineState::SafetyStopped) {
        if (TargetRpm > 0) {
            // start
            State = EWindTurbineState::Starting;
            UE_LOG(LogTemp, Warning, TEXT("AXeeWindturbineActor: State STARTING"));
            Hub->InitRpmCorrection(TargetRpm, State);
            House->InitAzimuthCorrection(Direction, State);
            InitPitchCorrection(Speed);
            StartAudio();
        }
        else {
            // in safety shutdown we want to follow winddirection with azimuth
            House->InitAzimuthCorrection(Direction, State);
        }
    }
    else if (State == EWindTurbineState::Starting) {
        if (TargetRpm == 0) {
            // stop
            if (Speed >= 8) {
                State = EWindTurbineState::SafetyStopping;
                UE_LOG(LogTemp, Warning, TEXT("AXeeWindturbineActor: State SAFETYSTOPPING"));
            }
            else {
                State = EWindTurbineState::Stopping;
                UE_LOG(LogTemp, Warning, TEXT("AXeeWindturbineActor: State STOPPING"));
            }
            Hub->InitRpmCorrection(TargetRpm, State);

            InitPitchCorrection(Speed);
        }
        else {
            // adapt target rpm and azimuth
            Hub->InitRpmCorrection(TargetRpm, State);
            House->InitAzimuthCorrection(Direction, State);
            InitPitchCorrection(Speed);
        }
    }
    else if (State == EWindTurbineState::Running) {
        if (TargetRpm == 0) {
            // stop
            if (Speed >= 8) {
                State = EWindTurbineState::SafetyStopping;
                UE_LOG(LogTemp, Warning, TEXT("AXeeWindturbineActor: State SAFETYSTOPPING"));
            }
            else {
                State = EWindTurbineState::Stopping;
                UE_LOG(LogTemp, Warning, TEXT("AXeeWindturbineActor: State STOPPING"));
            }
            Hub->InitRpmCorrection(TargetRpm, State);
            House->InitAzimuthCorrection(Direction, State);
            InitPitchCorrection(Speed);
        }
        else {
            // adapt target rpm and azimut
            Hub->InitRpmCorrection(TargetRpm, State);
            House->InitAzimuthCorrection(Direction, State);
            InitPitchCorrection(Speed);
        }
    }
    else if (State == EWindTurbineState::Stopping) {
        if (TargetRpm > 0) {
            // start
            State = EWindTurbineState::Starting;
            UE_LOG(LogTemp, Warning, TEXT("AXeeWindturbineActor: State STARTING"));
            Hub->InitRpmCorrection(TargetRpm, State);
            House->InitAzimuthCorrection(Direction, State);
            InitPitchCorrection(Speed);
        }
        else {
            // if were stopping below the threshold changes are uninteresting
        }
    }
    else if (State == EWindTurbineState::SafetyStopping) {
        if (TargetRpm > 0) {
            // start
            State = EWindTurbineState::Starting;
            UE_LOG(LogTemp, Warning, TEXT("AXeeWindturbineActor: State STARTING"));
            Hub->InitRpmCorrection(TargetRpm, State);
            House->InitAzimuthCorrection(Direction, State);
            InitPitchCorrection(Speed);
        }
        else {
            // in safety shutdown we want to follow winddirection with azimut
            House->InitAzimuthCorrection(Direction, State);
        }
    }
}

void AXeeWindturbineActor::OnAtmosphereParametersUpdate(const float Temperature, const float RelativeHumidity, const float AirPressure)
{
    LastReportedTemperature = Temperature;
    LastReportedRelativeHumidity = RelativeHumidity;
    LastReportedAirPressure = AirPressure;
}

void AXeeWindturbineActor::InitPitchCorrection(const float Speed)
{
    float TipSpeed = 0;
    float StreamAngle = 0;
    // calculate average tipspeed and angle
    for (auto& w : Wings) {
        TipSpeed += GetEffectiveSpeedAtComponent(w->BladeTip, Speed);
        StreamAngle += GetEffectiveWindStreamAngleAtComponent(w->BladeTip, Speed);
    }
    // apply same angle to every blade
    for (auto& w : Wings) {
        w->InitPitchCorrection(TipSpeed / Wings.Num(), StreamAngle / Wings.Num(), State);
    }
}

float  AXeeWindturbineActor::RotationalSpeedAtRadius(const float Radius) const
{
    return (2 * PI * Radius) * (Hub->GetCurrentRpm() / 60.0);
}

float AXeeWindturbineActor::GetWindshearMultiplier(const float BaseSpeed, const float Height) const
{
    // all length in meters, base windspeed is measured in 10 m height
    const float RoughnessLength = FWindshearData::RoughnessLength[WindshearGroundtype];
    return ((FMath::Loge(Height/100.0/RoughnessLength)/
            (FMath::Loge(10.0/RoughnessLength))) / BaseSpeed);
}

float AXeeWindturbineActor::GetEffectiveSpeedAtComponent(USceneComponent* Component, const float WindSpeed) const
{
    const float Height = Component->GetComponentLocation().Z - GetActorLocation().Z;
    const float HeightSpeedMult = GetWindshearMultiplier(WindSpeed, Height);

    const float Radius = FMath::Abs(Component->GetRelativeLocation().Y) / 100;
    const float RotationalSpeed = RotationalSpeedAtRadius(Radius);

    const float ShearedSpeed = WindSpeed * HeightSpeedMult;

    return FMath::Sqrt(FMath::Pow(ShearedSpeed, 2) + FMath::Pow(RotationalSpeed, 2));
}

float AXeeWindturbineActor::GetEffectiveWindStreamAngleAtComponent(USceneComponent* Component, const float WindSpeed) const
{
    const float Height = Component->GetComponentLocation().Z - GetActorLocation().Z;
    const float HeightSpeedMult = GetWindshearMultiplier(WindSpeed, Height);

    const float Radius = FMath::Abs(Component->GetRelativeLocation().Y);
    const float RotationalSpeed = RotationalSpeedAtRadius(Radius);

    const float ShearedSpeed = WindSpeed * HeightSpeedMult;

    return FMath::Atan2(ShearedSpeed, RotationalSpeed);
}

FVector AXeeWindturbineActor::GetPlayerPosition() const
{
    for(TActorIterator<APawn> It(GetWorld()); It;++It)
    {
        return (*It)->GetActorLocation();
    }
    return GetActorLocation();
}


float AXeeWindturbineActor::GetEffectiveWindStreamAngleAtComponent(USceneComponent* Component)
{
    return GetEffectiveWindStreamAngleAtComponent(Component, LastReportedWindSpeed);
}

float AXeeWindturbineActor::GetEffectiveSpeedAtComponent(USceneComponent* Component)
{
    return GetEffectiveSpeedAtComponent(Component, LastReportedWindSpeed);
}

float AXeeWindturbineActor::GetWingRotation(const int32 WingIndex) {
    if (WingIndex >= 0 && WingIndex < NumWings) {
        const FRotator AnchorRot = Wings[WingIndex]->GetRelativeRotation();
        const FRotator HubRot = Hub->GetRelativeRotation();

        return HubRot.Roll + AnchorRot.Roll;
    }
    return 0;
}

void AXeeWindturbineActor::StartAudio()
{
    NoiseStateChanged.Broadcast(true);
}

void AXeeWindturbineActor::StopAudio()
{
    NoiseStateChanged.Broadcast(false);
}
