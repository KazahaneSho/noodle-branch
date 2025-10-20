// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularGameMode.h"
#include "NoodlingGameMode.generated.h"

#define UE_API NOODLEBRANCH_API

enum class ECommonUserOnlineContext : uint8;
enum class ECommonUserPrivilege : uint8;
enum class ECommonSessionOnlineMode : uint8;
class UCommonUserInfo;
class UNoodlingExperienceDefinition;
class UNoodlingPawnData;
/**
 * Post login event, triggered when a player or bot joins the game as well as after seamless and non-seamless travel
 *
 * This is called after the player has finished initialization
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnNoodlingGameModePlayerInitialized, AGameModeBase* /*GameMode*/, AController* /*NewPlayer*/);

/**
 * Project Base GameMode class.
 */
UCLASS(MinimalAPI, Config = Game)
class ANoodlingGameMode : public AModularGameModeBase
{
	GENERATED_BODY()

public:

	UE_API explicit ANoodlingGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~AGameModeBase Interface Overrides
	UE_API virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	UE_API virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	UE_API virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;
	UE_API virtual bool ShouldSpawnAtStartSpot(AController* NewPlayer) override;
	UE_API virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	UE_API virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	UE_API virtual void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation) override;
	UE_API virtual bool PlayerCanRestart_Implementation(APlayerController* Player) override;
	UE_API virtual void InitGameState() override;
	UE_API virtual bool UpdatePlayerStartSpot(AController* Player, const FString& Portal, FString& OutErrorMessage) override;
	UE_API virtual void GenericPlayerInitialization(AController* NewPlayer) override;
	UE_API virtual void FailedToRestartPlayer(AController* NewPlayer) override;
	//~End of AGameModeBase Interface Overrides

	UFUNCTION(BlueprintCallable, Category="Noodling|Pawn")
	UE_API const UNoodlingPawnData* GetPawnDataForController(const AController* InController) const;

	// Restart (respawn) the specified player or bot next frame
	// - If bForceReset is true, the controller will be reset this frame (abandoning the currently possessed pawn, if any)
	UFUNCTION(BlueprintCallable)
	UE_API void RequestPlayerRestartNextFrame(AController* Controller, bool bForceReset = false);

	// Agnostic version of PlayerCanRestart that can be used for both player bots and players
	UE_API virtual bool ControllerCanRestart(AController* Controller);

	FOnNoodlingGameModePlayerInitialized OnGameModePlayerInitialized;

protected:

	UE_API void OnExperienceLoaded(const UNoodlingExperienceDefinition* CurrentExperience);
	UE_API bool IsExperienceLoaded() const;

	UE_API void OnMatchAssignmentGiven(const FPrimaryAssetId& ExperienceId, const FString& ExperienceSourceId) const;

	UE_API void HandleMatchAssignmentIfNotExpectingOne();

	UE_API bool TryDedicatedServerLogin();
	UE_API void	HostDedicatedServerMatch(ECommonSessionOnlineMode OnlineMode) const;

	UFUNCTION()
	UE_API void OnUserInitializedForDedicatedServer(const UCommonUserInfo* UserInfo, bool bSuccess, FText Error, ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext);
};

#undef UE_API
