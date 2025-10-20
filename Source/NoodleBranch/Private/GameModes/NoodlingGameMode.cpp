// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/NoodlingGameMode.h"

#include "GameModes/NoodlingGameState.h"
#include "Player/NoodlingPawn.h"
#include "Player/NoodlingPlayerController.h"
#include "Player/NoodlingPlayerState.h"
#include "System/NoodlingGameSession.h"
#include "UI/NoodlingHUD.h"

ANoodlingGameMode::ANoodlingGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GameStateClass = ANoodlingGameState::StaticClass();
	GameSessionClass = ANoodlingGameSession::StaticClass();
	PlayerControllerClass = ANoodlingPlayerController::StaticClass();
	ReplaySpectatorPlayerControllerClass = ANoodlingReplayPlayerController::StaticClass();
	PlayerStateClass = ANoodlingPlayerState::StaticClass();
	DefaultPawnClass = ANoodlingPawn::StaticClass();
	HUDClass = ANoodlingHUD::StaticClass();
	
}

void ANoodlingGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
}

UClass* ANoodlingGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

APawn* ANoodlingGameMode::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer,
	const FTransform& SpawnTransform)
{
	return Super::SpawnDefaultPawnAtTransform_Implementation(NewPlayer, SpawnTransform);
}

bool ANoodlingGameMode::ShouldSpawnAtStartSpot(AController* NewPlayer)
{
	return Super::ShouldSpawnAtStartSpot(NewPlayer);
}

void ANoodlingGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
}

AActor* ANoodlingGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	return Super::ChoosePlayerStart_Implementation(Player);
}

void ANoodlingGameMode::FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation)
{
	Super::FinishRestartPlayer(NewPlayer, StartRotation);
}

bool ANoodlingGameMode::PlayerCanRestart_Implementation(APlayerController* Player)
{
	return Super::PlayerCanRestart_Implementation(Player);
}

void ANoodlingGameMode::InitGameState()
{
	Super::InitGameState();
}

bool ANoodlingGameMode::UpdatePlayerStartSpot(AController* Player, const FString& Portal, FString& OutErrorMessage)
{
	return Super::UpdatePlayerStartSpot(Player, Portal, OutErrorMessage);
}

void ANoodlingGameMode::GenericPlayerInitialization(AController* C)
{
	Super::GenericPlayerInitialization(C);
}

void ANoodlingGameMode::FailedToRestartPlayer(AController* NewPlayer)
{
	Super::FailedToRestartPlayer(NewPlayer);
}
