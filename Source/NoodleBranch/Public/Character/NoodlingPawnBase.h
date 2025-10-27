// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ModularPawn.h"
#include "Teams/NoodlingTeamAgentInterface.h"
#include "NoodlingPawnBase.generated.h"

#define UE_API NOODLEBRANCH_API

UCLASS(MinimalAPI)
class ANoodlingPawnBase : public AModularPawn, public INoodlingTeamAgentInterface
{
	GENERATED_BODY()

public:

	UE_API explicit ANoodlingPawnBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~AActor interface
	UE_API virtual void PreInitializeComponents() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of AActor interface

	//~APawn interface
	UE_API virtual void PossessedBy(AController* NewController) override;
	UE_API virtual void UnPossessed() override;
	//~End of APawn interface

	//~INoodlingTeamAgentInterface interface
	UE_API virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	UE_API virtual FGenericTeamId GetGenericTeamId() const override;
	UE_API virtual FOnNoodlingTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	//~End of INoodlingTeamAgentInterface interface

protected:
	// Called to determine what happens to the team ID when possession ends
	virtual FGenericTeamId DetermineNewTeamAfterPossessionEnds(FGenericTeamId OldTeamID) const
	{
		// This could be changed to return, e.g., OldTeamID if you want to keep it assigned afterward, or return an ID for some neutral faction, or etc...
		return FGenericTeamId::NoTeam;
	}

private:
	UFUNCTION()
	UE_API void OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);

	UFUNCTION()
	UE_API void OnRep_MyTeamID(FGenericTeamId OldTeamID);

private:
	UPROPERTY(ReplicatedUsing = OnRep_MyTeamID)
	FGenericTeamId MyTeamID;

	UPROPERTY()
	FOnNoodlingTeamIndexChangedDelegate OnTeamChangedDelegate;
};

#undef UE_API
