// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/NoodlingPlayerSpawningManagerComponent.h"

UNoodlingPlayerSpawningManagerComponent::UNoodlingPlayerSpawningManagerComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

// Called every frame
void UNoodlingPlayerSpawningManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                            FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

AActor* UNoodlingPlayerSpawningManagerComponent::ChoosePlayerStart(AController* Player)
{
	return nullptr;
}

void UNoodlingPlayerSpawningManagerComponent::FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation)
{
}

bool UNoodlingPlayerSpawningManagerComponent::ControllerCanRestart(AController* Player)
{
	// TODO: Can they restart logic.

	return true;
}

