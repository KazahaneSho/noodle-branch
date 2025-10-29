// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/GameStateComponent.h"
#include "NoodlingPlayerSpawningManagerComponent.generated.h"

#define UE_API NOODLEBRANCH_API


class ANoodlingPlayerStart;

UCLASS(MinimalAPI)
class UNoodlingPlayerSpawningManagerComponent : public UGameStateComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	explicit UNoodlingPlayerSpawningManagerComponent(const FObjectInitializer& ObjectInitializer);

	/** UActorComponent */
	UE_API virtual void InitializeComponent() override;
	UE_API virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	/** ~UActorComponent */
protected:
	// Utility
	UE_API static APlayerStart* GetFirstRandomUnoccupiedPlayerStart(AController* Controller, const TArray<ANoodlingPlayerStart*>& FoundStartPoints);
	
	virtual AActor* OnChoosePlayerStart(AController* Player, TArray<ANoodlingPlayerStart*>& PlayerStarts) { return nullptr; }
	virtual void OnFinishRestartPlayer(AController* Player, const FRotator& StartRotation) { }

	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName=OnFinishRestartPlayer))
	UE_API void K2_OnFinishRestartPlayer(AController* Player, const FRotator& StartRotation);

private:
	/** We proxy these calls from ANoodlingGameMode, to this component so that each experience can more easily customize the respawn system they want. */
	UE_API AActor* ChoosePlayerStart(AController* Player);
	UE_API void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation);
	UE_API bool ControllerCanRestart(AController* Player);
	friend class ANoodlingGameMode;
	/** ~ANoodlingGameMode */

	UE_API void OnLevelAdded(ULevel* InLevel, UWorld* InWorld);
	UE_API void HandleOnActorSpawned(AActor* SpawnedActor);

#if WITH_EDITOR
	UE_API APlayerStart* FindPlayFromHereStart(const AController* Player) const;
#endif // WITH_EDITOR
	
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<ANoodlingPlayerStart>> CachedPlayerStarts;
};

#undef UE_API
