// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/GameStateComponent.h"
#include "NoodlingPlayerSpawningManagerComponent.generated.h"

#define UE_API NOODLEBRANCH_API


UCLASS(MinimalAPI)
class UNoodlingPlayerSpawningManagerComponent : public UGameStateComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	explicit UNoodlingPlayerSpawningManagerComponent(const FObjectInitializer& ObjectInitializer);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
protected:

private:
	UE_API AActor* ChoosePlayerStart(AController* Player);
	UE_API void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation);
	UE_API bool ControllerCanRestart(AController* Player);
	friend class ANoodlingGameMode;
};

#undef UE_API
