// Copyright (c) Iwer Petersen. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "XeeWindturbineComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class XEEWINDTURBINE_API UXeeWindturbineComponent : public USceneComponent
{
	GENERATED_BODY()

public:	

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
		USceneComponent* Anchor;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Meshes")
	UStaticMeshComponent* MeshComponent;

	// Sets default values for this component's properties
	UXeeWindturbineComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void OnComponentCreated() override;

		
};
