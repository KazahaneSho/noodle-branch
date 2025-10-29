// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ModularAIController.h"
#include "Teams/NoodlingTeamAgentInterface.h"
#include "NoodlingPlayerBotController.generated.h"

/**
 * ANoodlingBotController
 *
 * The AI controller class for the project.
 */
UCLASS(Blueprintable)
class ANoodlingPlayerBotController : public AModularAIController, public INoodlingTeamAgentInterface
{
	GENERATED_BODY()

public:
	explicit ANoodlingPlayerBotController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~INoodlingTeamAgentInterface interface
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual FOnNoodlingTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;
	//~End of INoodlingTeamAgentInterface interface

	// Attempt to restart this controller (e.g. To respawn it)
	void ServerRestartController();

	//Update Team Attitude for the AI, i.e. if game ability changes team assignment
	UFUNCTION(BlueprintCallable, Category = "Noodling AI Player Controller")
	static void UpdateTeamAttitude(UAIPerceptionComponent* AIPerception);

	virtual void OnUnPossess() override;

protected:
	//~AController interface
	virtual void InitPlayerState() override;
	virtual void CleanupPlayerState() override;
	virtual void OnRep_PlayerState() override;
	//~End of AController interface

	// Called when the player state is set or cleared
	virtual void OnPlayerStateChanged();

private:
	UFUNCTION()
	void OnPlayerStateChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);

	void BroadcastOnPlayerStateChanged();

	UPROPERTY()
	FOnNoodlingTeamIndexChangedDelegate OnTeamChangedDelegate;

	UPROPERTY()
	TObjectPtr<APlayerState> LastSeenPlayerState;
};
