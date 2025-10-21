// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonPlayerController.h"
#include "Camera/NoodlingCameraAssistInterface.h"
#include "Teams/NoodlingTeamAgentInterface.h"
#include "NoodlingPlayerController.generated.h"

#define UE_API NOODLEBRANCH_API

class UNoodlingSettingsShared;
class ANoodlingHUD;
class UNoodlingAbilitySystemComponent;
class ANoodlingPlayerState;
/**
 * Project Base PlayerController class.
 */
UCLASS(MinimalAPI, Config = Game, Meta = (ShortTooltip = "The base player controller class used by this project."))
class ANoodlingPlayerController : public ACommonPlayerController, public INoodlingCameraAssistInterface, public INoodlingTeamAgentInterface
{
	GENERATED_BODY()

public:

	UE_API explicit ANoodlingPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~AActor interface
	UE_API virtual void PreInitializeComponents() override;
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	UE_API virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~End of AActor interface

	//~AController interface
	UE_API virtual void OnPossess(APawn* InPawn) override;
	UE_API virtual void OnUnPossess() override;
	UE_API virtual void InitPlayerState() override;
	UE_API virtual void CleanupPlayerState() override;
	UE_API virtual void OnRep_PlayerState() override;
	//~End of AController interface

	//~APlayerController interface
	UE_API virtual void ReceivedPlayer() override;
	UE_API virtual void PlayerTick(float DeltaTime) override;
	UE_API virtual void SetPlayer(UPlayer* InPlayer) override;
	UE_API virtual void AddCheats(bool bForce) override;
	UE_API virtual void UpdateForceFeedback(IInputInterface* InputInterface, const int32 ControllerId) override;
	UE_API virtual void UpdateHiddenComponents(const FVector& ViewLocation, TSet<FPrimitiveComponentId>& OutHiddenComponents) override;
	UE_API virtual void PreProcessInput(const float DeltaTime, const bool bGamePaused) override;
	UE_API virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;
	//~End of APlayerController interface

	//~INoodlingCameraAssistInterface interface
	UE_API virtual void OnCameraPenetratingTarget() override;
	//~End of INoodlingCameraAssistInterface interface
	
	//~INoodlingTeamAgentInterface interface
	UE_API virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	UE_API virtual FGenericTeamId GetGenericTeamId() const override;
	UE_API virtual FOnNoodlingTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	//~End of INoodlingTeamAgentInterface interface

	UFUNCTION(BlueprintCallable, Category = "NoodleBranch|PlayerController")
	UE_API ANoodlingPlayerState* GetNoodlingPlayerState() const;

	UFUNCTION(BlueprintCallable, Category = "NoodleBranch|PlayerController")
	UE_API UNoodlingAbilitySystemComponent* GetNoodlingAbilitySystemComponent() const;

	UFUNCTION(BlueprintCallable, Category = "NoodleBranch|PlayerController")
	UE_API ANoodlingHUD* GetNoodlingHUD() const;

	// Call from game state logic to start recording an automatic client replay if ShouldRecordClientReplay returns true
	UFUNCTION(BlueprintCallable, Category = "NoodleBranch|PlayerController")
	UE_API bool TryToRecordClientReplay();

	// Call to see if we should record a replay, subclasses could change this
	UE_API virtual bool ShouldRecordClientReplay();

	// Run a cheat command on the server.
	UFUNCTION(Reliable, Server, WithValidation)
	UE_API void ServerCheat(const FString& Msg);

	// Run a cheat command on the server for all players.
	UFUNCTION(Reliable, Server, WithValidation)
	UE_API void ServerCheatAll(const FString& Msg);

	UFUNCTION(BlueprintCallable, Category = "NoodleBranch|Character")
	UE_API void SetIsAutoRunning(const bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "NoodleBranch|Character")
	UE_API bool GetIsAutoRunning() const;

private:
	UPROPERTY()
	FOnNoodlingTeamIndexChangedDelegate OnTeamChangedDelegate;

	UPROPERTY()
	TObjectPtr<APlayerState> LastSeenPlayerState;

private:
	UFUNCTION()
	void OnPlayerStateChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);

protected:
	// Called when the player state is set or cleared
	UE_API virtual void OnPlayerStateChanged();

private:
	void BroadcastOnPlayerStateChanged();

protected:

	//~APlayerController interface

	//~End of APlayerController interface

	UE_API void OnSettingsChanged(const UNoodlingSettingsShared* InSettings);
	
	UE_API void OnStartAutoRun();
	UE_API void OnEndAutoRun();

	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="OnStartAutoRun"))
	UE_API void K2_OnStartAutoRun();

	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="OnEndAutoRun"))
	UE_API void K2_OnEndAutoRun();

	bool bHideViewTargetPawnNextFrame = false;
};

// Player Controller for Replay Capture and Playback
UCLASS()
class ANoodlingReplayPlayerController : public ANoodlingPlayerController
{
	GENERATED_BODY()

	virtual void Tick(float DeltaSeconds) override;
	virtual void SmoothTargetViewRotation(APawn* TargetPawn, float DeltaSeconds) override;
	virtual bool ShouldRecordClientReplay() override;

	// Callback for when the game state's RecorderPlayerState gets replicated during replay playback
	void RecorderPlayerStateUpdated(APlayerState* NewRecorderPlayerState);

	// Callback for when the followed player state changes pawn
	UFUNCTION()
	void OnPlayerStatePawnSet(APlayerState* ChangedPlayerState, APawn* NewPlayerPawn, APawn* OldPlayerPawn);

	// The player state we are currently following */
	UPROPERTY(Transient)
	TObjectPtr<APlayerState> FollowedPlayerState;
};

#undef UE_API
