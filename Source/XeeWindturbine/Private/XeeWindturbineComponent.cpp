// Copyright (c) Iwer Petersen. All rights reserved.


#include "XeeWindturbineComponent.h"

// Sets default values for this component's properties
UXeeWindturbineComponent::UXeeWindturbineComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
	Anchor = CreateDefaultSubobject<USceneComponent>(TEXT("Anchor"));
	Anchor->SetupAttachment(this);
	Anchor->SetMobility(EComponentMobility::Type::Movable);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(Anchor);
	MeshComponent->SetMobility(EComponentMobility::Type::Movable);
}


// Called when the game starts
void UXeeWindturbineComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

// Called every frame
void UXeeWindturbineComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UXeeWindturbineComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

}



