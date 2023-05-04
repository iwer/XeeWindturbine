// Copyright (c) Iwer Petersen. All rights reserved.


#include "XeeTurbineHouseComponent.h"

// Sets default values for this component's properties
UXeeTurbineHouseComponent::UXeeTurbineHouseComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
	AzimuthSpeed = 5;
	bAWLEnabled = false;
	AzimuthWindDirectionTolerance = 10;
	CurrentAzimuth = 0;
	bSuppressRotation = false;
	
	// Timelines setup
	BlinkCurve = CreateDefaultSubobject<UCurveFloat>(TEXT("BlinkCurve"));
	BlinkCurve->FloatCurve.AddKey(0.0, 1.0);
	BlinkCurve->FloatCurve.AddKey(1.0, 1.0);
	BlinkCurve->FloatCurve.AddKey(1.01, 0.0);
	BlinkCurve->FloatCurve.AddKey(1.5, 0.0);
	BlinkCurve->FloatCurve.AddKey(1.51, 1.0);
	BlinkCurve->FloatCurve.AddKey(2.5, 1.0);
	BlinkCurve->FloatCurve.AddKey(2.51, 0.0);
	BlinkCurve->FloatCurve.AddKey(4.0, 0.0);

	BlinkTimelineFunction.BindUFunction(this, TEXT("BlinkTimelineCallback"));

	BlinkTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("BlinkTimeline"));
	BlinkTimeline->SetTimelineLength(4);
	BlinkTimeline->SetLooping(true);
	BlinkTimeline->AddInterpFloat(BlinkCurve, BlinkTimelineFunction, TEXT("LightStatus"));

	AzimuthCurve = CreateDefaultSubobject<UCurveFloat>(TEXT("AzimuthCurve"));
	AzimuthCurve->FloatCurve.AddKey(0, 0);
	AzimuthCurve->FloatCurve.AddKey(1, 1);
	
	
	AzimuthTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("AzimuthTimeline"));
	AzimuthTimeline->SetTimelineLength(1);


	
	

	// for (auto& p : AWLPositions) {
	// 	UPointLightComponent* light = CreateDefaultSubobject<UPointLightComponent>(TEXT("AWL"));
	// 	light->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	// 	light->SetMobility(EComponentMobility::Type::Movable);
	// 	light->RegisterComponent();
	// 	light->SetRelativeLocation_Direct(p);
	// 	light->SetLightColor(FLinearColor::Red);
	// 	light->SetAttenuationRadius(10000);
	// 	light->SetSourceRadius(30);
	// 	light->SetSourceLength(30);
	// 	light->SetIntensityUnits(ELightUnits::Unitless);
	// 	light->SetVolumetricScatteringIntensity(100);
	// 	light->SetCastVolumetricShadow(true);
	// 	light->SetAffectGlobalIllumination(true);
	// 	AWLs.Add(light);
	// }
}

void UXeeTurbineHouseComponent::UpdateAzimuth(const float NewAzimuth)
{
	if (!bSuppressRotation) {
		const FRotator Rot(0, NewAzimuth, 0);
		SetRelativeRotation(Rot);
	}
}


// Called when the game starts
void UXeeTurbineHouseComponent::BeginPlay()
{
	Super::BeginPlay();
	// ...
	CreateAWLPointLights();

	AzimuthTimelineFunction.BindUFunction(this, TEXT("AzimuthTimelineCallback"));
	AzimuthTimeline->AddInterpFloat(AzimuthCurve, AzimuthTimelineFunction, TEXT("PercentAzimutTurn"));
		
	AzimuthCorrectionFinished.BindUFunction(this, TEXT("OnAzimuthCorrectionFinished"));
	AzimuthTimeline->SetTimelineFinishedFunc(AzimuthCorrectionFinished);
}


// Called every frame
void UXeeTurbineHouseComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
	// start / stop blink timeline
	if (bAWLEnabled && !BlinkTimeline->IsPlaying()) {
		BlinkTimeline->Play();
	}
	else if (!bAWLEnabled && BlinkTimeline->IsPlaying()) {
		BlinkTimeline->Stop();
	}

	// apply AWLState to components
	for (auto& l : AWLs) {
		l->SetIntensity(AWLState * 1000);
		l->SetVolumetricScatteringIntensity(AWLState * 10);
		if (AWLState <= .001) {
			l->SetVisibility(false);
		}
		else {
			l->SetVisibility(true);
		}
	}
	if (State != EWindTurbineState::Stopped) {
		UpdateAzimuth(CurrentAzimuth);
	}
}

void UXeeTurbineHouseComponent::CreateAWLPointLights()
{
	AWLs.Empty();
	for (auto& p : AWLPositions) {
		UPointLightComponent* light = NewObject<UPointLightComponent>(this, UPointLightComponent::StaticClass());
		light->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
		light->SetMobility(EComponentMobility::Type::Movable);
		light->RegisterComponent();
		light->SetRelativeLocation(p);
		light->SetLightColor(FLinearColor::Red);
		light->SetAttenuationRadius(1000);
		light->SetSourceRadius(30);
		light->SetSourceLength(30);
		light->SetIntensityUnits(ELightUnits::Candelas);
		light->SetCastVolumetricShadow(false);
		light->SetAffectGlobalIllumination(false);
		light->bAffectsWorld = false;
		AWLs.Add(light);
	}
}

void UXeeTurbineHouseComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

	// aerial warning lights
	CreateAWLPointLights();



	//UpdateAzimuth(CurrentAzimuth);
}

void UXeeTurbineHouseComponent::BlinkTimelineCallback(const float Val)
{
	AWLState = Val;
}

void UXeeTurbineHouseComponent::AzimuthTimelineCallback(const float Val)
{
	float TargetAngle = AzimuthStart + Val * AzimuthDiff;
	if (TargetAngle > 360) {
		TargetAngle -= 360;
	}
	else if (TargetAngle < 0) {
		TargetAngle += 360;
	}
	CurrentAzimuth = TargetAngle;
}

void UXeeTurbineHouseComponent::InitAzimuthCorrection(const float WindDirection, const TEnumAsByte<EWindTurbineState> NewState)
{
	UE_LOG(LogTemp, Warning, TEXT("UXeeTurbineHouseComponent::InitAzimuthCorrection(WindDirection: %f)"), WindDirection);
	State = NewState;
	AzimuthTimeline->Stop();
	// determine shorter turn
	float Diff = 0;
	if (WindDirection == CurrentAzimuth) {
		UE_LOG(LogTemp, Warning, TEXT("UXeeTurbineHouseComponent::InitAzimuthCorrection Winddir == Azimut"))
		return;
	}
	else if (WindDirection > CurrentAzimuth) {
		if (WindDirection - CurrentAzimuth < 180) { // N----a-(<180)--w----N => forward rotation
			UE_LOG(LogTemp, Warning, TEXT("UXeeTurbineHouseComponent: Azimut forward rotation"))
			Diff = WindDirection - CurrentAzimuth;
		}
		else {                                     // N----a-(>=180)-w----N => backward rotation
			UE_LOG(LogTemp, Warning, TEXT("UXeeTurbineHouseComponent: Azimut backward rotation"))
			Diff = WindDirection - CurrentAzimuth - 360;
		}
	}
	else {
		if (CurrentAzimuth - WindDirection < 180) { // N----w-(<180)-a----N => backward rotation
			UE_LOG(LogTemp, Warning, TEXT("UXeeTurbineHouseComponent: Azimut backward rotation"))
			Diff = WindDirection - CurrentAzimuth;
		}
		else {                                     // N----w-(>=180)--a----N => forward rotation
			UE_LOG(LogTemp, Warning, TEXT("UXeeTurbineHouseComponent: Azimut forward rotation"))
			Diff = WindDirection - CurrentAzimuth + 360;
		}
	}

	if (FMath::Abs(Diff) >= AzimuthWindDirectionTolerance) {
		AzimuthStart = CurrentAzimuth;
		AzimuthDiff = Diff;
		const float SecondsToTurn = FMath::Abs(Diff) / AzimuthSpeed;

		UE_LOG(LogTemp, Warning, TEXT("UXeeTurbineHouseComponent::InitAzimuthCorrection: Initiating azimuth turn of %f ° at %f °/s for %f s"), Diff, AzimuthSpeed, SecondsToTurn)
		AzimuthTimeline->SetPlayRate(1 / SecondsToTurn);
		AzimuthTimeline->PlayFromStart();

	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("UXeeTurbineHouseComponent::InitAzimuthCorrection: Direction diff inside tolerance"))
	}
}

void UXeeTurbineHouseComponent::OnAzimuthCorrectionFinished() const
{
}
