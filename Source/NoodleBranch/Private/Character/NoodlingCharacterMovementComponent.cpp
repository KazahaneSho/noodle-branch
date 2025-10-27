// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/NoodlingCharacterMovementComponent.h"


// Sets default values for this component's properties
UNoodlingCharacterMovementComponent::UNoodlingCharacterMovementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UNoodlingCharacterMovementComponent::SetReplicatedAcceleration(const FVector& InAcceleration)
{
}


// Called when the game starts
void UNoodlingCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UNoodlingCharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                        FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

