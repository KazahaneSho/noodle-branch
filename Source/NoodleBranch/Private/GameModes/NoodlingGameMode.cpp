// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/NoodlingGameMode.h"
#include "Character/NoodlingPawnData.h"
#include "Character/NoodlingPawnExtensionComponent.h"
#include "Development/NoodlingDeveloperSettings.h"
#include "GameModes/NoodlingExperienceDefinition.h"
#include "GameModes/NoodlingExperienceManagerComponent.h"
#include "GameModes/NoodlingGameState.h"
#include "GameModes/NoodlingWorldSettings.h"
#include "Kismet/GameplayStatics.h"
#include "NoodleBranch/NoodlingLogChannels.h"
#include "Player/NoodlingPawn.h"
#include "Player/NoodlingPlayerBotController.h"
#include "Player/NoodlingPlayerController.h"
#include "Player/NoodlingPlayerSpawningManagerComponent.h"
#include "Player/NoodlingPlayerState.h"
#include "System/NoodlingAssetManager.h"
#include "System/NoodlingGameSession.h"
#include "UI/NoodlingHUD.h"
#include "CommonUserSubsystem.h"
#include "CommonSessionSubsystem.h"
#include "GameMapsSettings.h"
#include "GameModes/NoodlingUserFacingExperienceDefinition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NoodlingGameMode)

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

	// Wait for the next frame to give time to initialize startup settings
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::HandleMatchAssignmentIfNotExpectingOne);
}

UClass* ANoodlingGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (const UNoodlingPawnData* PawnData = GetPawnDataForController(InController))
	{
		if (PawnData->PawnClass)
		{
			return PawnData->PawnClass;
		}
	}
	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

APawn* ANoodlingGameMode::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer,
	const FTransform& SpawnTransform)
{
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Instigator = GetInstigator();
	SpawnParameters.ObjectFlags |= RF_Transient;  // Never save the default player pawn into a map.
	SpawnParameters.bDeferConstruction = true;

	// Do we have a DefaultPawnClass to spawn?
	if (UClass* PawnClass = GetDefaultPawnClassForController(NewPlayer))
	{
		// Did we spawn the PawnClass at SpawnTransform?
		if (APawn* SpawnedPawn = GetWorld()->SpawnActor<APawn>(PawnClass, SpawnTransform, SpawnParameters))
		{
			// Did we get the PawnComponent that adds functionality to the Pawn?
			if (UNoodlingPawnExtensionComponent* PawnExtensionComponent = UNoodlingPawnExtensionComponent::FindPawnExtensionComponent(SpawnedPawn))
			{
				// Do we have PawnData to set on the Pawn?
				if (const UNoodlingPawnData* PawnData = GetPawnDataForController(NewPlayer))
				{
					PawnExtensionComponent->SetPawnData(PawnData);
				}
				else
				{
					UE_LOG(NoodleBranch_Log,
						Error,
						TEXT("GameMode was unable to set PawnData on the spawned pawn [%s] for controller [%s]"), *GetNameSafe(SpawnedPawn), *GetNameSafe(NewPlayer));
				}
			}
			SpawnedPawn->FinishSpawning(SpawnTransform);
			return SpawnedPawn;
		}
		UE_LOG(NoodleBranch_Log,
			Error,
			TEXT("GameMode was unable to spawn Pawn of class [%s] at [%s]"), *GetNameSafe(PawnClass), *SpawnTransform.ToHumanReadableString());
	}
	else
	{
		UE_LOG(NoodleBranch_Log,
			Error,
			TEXT("GameMode was unable to spawn Pawn due to NULL pawn class"));
	}
	return nullptr;
}

bool ANoodlingGameMode::ShouldSpawnAtStartSpot(AController* NewPlayer)
{
	// Don't use StartSpot, always defer to the spawn management component e.g. /Player/NoodlingPlayerSpawningManagerComponent
	return false;
}

void ANoodlingGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	// Delay the starting of new players until the experience has been loaded
	// Players who log in before the experience has been loaded will be started by OnExperienceLoaded
	if (!IsExperienceLoaded()) return;

	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
}

AActor* ANoodlingGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	if (UNoodlingPlayerSpawningManagerComponent* PlayerSpawningManagerComponent = GameState->FindComponentByClass<UNoodlingPlayerSpawningManagerComponent>())
	{
		return PlayerSpawningManagerComponent->ChoosePlayerStart(Player);
	}
	
	return Super::ChoosePlayerStart_Implementation(Player);
}

void ANoodlingGameMode::FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation)
{
	if (UNoodlingPlayerSpawningManagerComponent* PlayerSpawningManagerComponent = GameState->FindComponentByClass<UNoodlingPlayerSpawningManagerComponent>())
	{
		return PlayerSpawningManagerComponent->FinishRestartPlayer(NewPlayer, StartRotation);
	}
	
	Super::FinishRestartPlayer(NewPlayer, StartRotation);
}

bool ANoodlingGameMode::PlayerCanRestart_Implementation(APlayerController* Player)
{
	return ControllerCanRestart(Player);
}

void ANoodlingGameMode::InitGameState()
{
	Super::InitGameState();
	// TODO: Uncomment when GameState implemented.
	// // Hook into OnExperienceLoaded delegate from UNoodlingExperienceManager and call ThisClass::OnExperienceLoaded when experience load is complete
	// UNoodlingExperienceManagerComponent* ExperienceManagerComponent = GameState->FindComponentByClass<UNoodlingExperienceManagerComponent>();
	// check(ExperienceManagerComponent);
	// ExperienceManagerComponent->CallOrRegister_OnExperienceLoaded(FOnNoodlingExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
}

bool ANoodlingGameMode::UpdatePlayerStartSpot(AController* Player, const FString& Portal, FString& OutErrorMessage)
{
	// Do nothing here, because we'll wait until PostLogin to try to spawn the player.
	// Doing anything now is bad, because systems like TeamAssignment haven't occured yet.
	return true;
}

void ANoodlingGameMode::GenericPlayerInitialization(AController* NewPlayer)
{
	Super::GenericPlayerInitialization(NewPlayer);

	OnGameModePlayerInitialized.Broadcast(this, NewPlayer);
}

void ANoodlingGameMode::FailedToRestartPlayer(AController* NewPlayer)
{
	Super::FailedToRestartPlayer(NewPlayer);

	// Try again, but check if there's actually a pawn class before we try forever
	if (UClass* PawnClass = GetDefaultPawnClassForController(NewPlayer))
	{
		if (APlayerController* NewPC = Cast<APlayerController>(NewPlayer))
		{
			if (PlayerCanRestart(NewPC))
			{
				RequestPlayerRestartNextFrame(NewPlayer, false);
			}
			else
			{
				UE_LOG(NoodleBranch_Log,
					Verbose,
					TEXT("FailedToRestartPlayer(%s) and PlayerCanRestart returned false, so not trying again."), *GetPathNameSafe(NewPlayer));
			}
		}
		else
		{
			RequestPlayerRestartNextFrame(NewPlayer, false);
		}
	}
	else
	{
		UE_LOG(NoodleBranch_Log,
			Verbose,
			TEXT("FailedToRestartPlayer(%s) but there's no pawn class so giving up."), *GetPathNameSafe(NewPlayer));
	}
}

const UNoodlingPawnData* ANoodlingGameMode::GetPawnDataForController(const AController* InController) const
{
	// Check if pawn data is already set in PlayerState.
	if (InController != nullptr)
	{
		if (const ANoodlingPlayerState* NoodlingPlayerState = InController->GetPlayerState<ANoodlingPlayerState>())
		{
			if (const UNoodlingPawnData* PawnData = NoodlingPlayerState->GetPawnData<UNoodlingPawnData>())
			{
				return PawnData;
			}
		}
	}

	// Fall Back to Default PawnData for current experience.
	check(GameState)
	const UNoodlingExperienceManagerComponent* ExperienceManagerComponent = GameState->FindComponentByClass<UNoodlingExperienceManagerComponent>();
	check(ExperienceManagerComponent);

	if (ExperienceManagerComponent->IsExperienceLoaded())
	{
		if (const UNoodlingExperienceDefinition* Experience = ExperienceManagerComponent->GetCurrentExperienceChecked(); Experience->DefaultPawnData != nullptr)
		{
			return Experience->DefaultPawnData.Get();
		}

		// Experience is loaded but still no pawn data set, so fall back to AssetManager
		return UNoodlingAssetManager::Get().GetDefaultPawnData();
	}
	return nullptr;
}

void ANoodlingGameMode::RequestPlayerRestartNextFrame(AController* Controller, bool bForceReset)
{
	if (bForceReset && (Controller != nullptr))
	{
		Controller->Reset();
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		GetWorldTimerManager().SetTimerForNextTick(PlayerController, &APlayerController::ServerRestartPlayer_Implementation);
	}
	else if (ANoodlingPlayerBotController* BotController = Cast<ANoodlingPlayerBotController>(Controller))
	{
		GetWorldTimerManager().SetTimerForNextTick(BotController, &ANoodlingPlayerBotController::ServerRestartController);
	}
}

bool ANoodlingGameMode::ControllerCanRestart(AController* Controller)
{
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (!Super::PlayerCanRestart_Implementation(PlayerController))
		{
			return false;
		}
	}
	else
	{
		// Bot Version of Super::PlayerCanRestart_Implementation
		if (Controller == nullptr || Controller->IsPendingKillPending())
		{
			return false;
		}
	}

	if (UNoodlingPlayerSpawningManagerComponent* PlayerSpawningManagerComponent = GameState->FindComponentByClass<UNoodlingPlayerSpawningManagerComponent>())
	{
		return PlayerSpawningManagerComponent->ControllerCanRestart(Controller);
	}

	return true;
}

void ANoodlingGameMode::OnExperienceLoaded(const UNoodlingExperienceDefinition* CurrentExperience)
{
	// Spawn any players that are already attached
	//@TODO: Here we're handling only *player* controllers, but in GetDefaultPawnClassForController_Implementation we skipped all controllers
	// GetDefaultPawnClassForController_Implementation might only be getting called for players anyways
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Cast<APlayerController>(*Iterator);
		if (PlayerController != nullptr && PlayerController->GetPawn() == nullptr)
		{
			if (PlayerCanRestart(PlayerController))
			{
				RestartPlayer(PlayerController);
			}
		}
	}
}

bool ANoodlingGameMode::IsExperienceLoaded() const
{
	// TODO: Uncomment when GameState implemented.
	// check(GameState);
	// const UNoodlingExperienceManagerComponent* ExperienceManagerComponent = GameState->FindComponentByClass<UNoodlingExperienceManagerComponent>();
	// check(ExperienceManagerComponent)
	// return ExperienceManagerComponent->IsExperienceLoaded();
	return false;
}

void ANoodlingGameMode::OnMatchAssignmentGiven(const FPrimaryAssetId& ExperienceId, const FString& ExperienceSourceId) const
{
	if (ExperienceId.IsValid())
	{
		UE_LOG(NoodleBranch_LogExperience,
			Log,
			TEXT("Identified experience [%s] from source: [%s]"), *ExperienceId.ToString(), *ExperienceSourceId);

		UNoodlingExperienceManagerComponent* ExperienceManagerComponent = GameState->FindComponentByClass<UNoodlingExperienceManagerComponent>();
		check(ExperienceManagerComponent);
		ExperienceManagerComponent->SetCurrentExperience(ExperienceId);
	}
	else
	{
		UE_LOG(NoodleBranch_LogExperience,
			Error,
			TEXT("Failed to identify experience, loading screen will be shown forever."));
	}
}

void ANoodlingGameMode::HandleMatchAssignmentIfNotExpectingOne()
{
	FPrimaryAssetId ExperienceId;
	FString ExperienceIdSource;

	// Precedence order (highest wins)
	//  - Matchmaking assignment (TODO: Implement if present)
	//  - URL Options override
	//  - Developer Settings (PIE only)
	//  - Command Line override
	//  - World Settings
	//  - Dedicated server
	//  - Default experience

	UWorld* World = GetWorld();

	/**
	 * Check if there is an OptionsString
	 * This is constructed by UNoodlingUserFacingExperienceDefinition::CreateHostingRequest(const UObject* WorldContextObject))
	 * where the Key is “Experience”,
	 * this is defined in a DataAsset derived from NoodlingUserFacingExperienceDefinition
	 */
	if (!ExperienceId.IsValid() && UGameplayStatics::HasOption(OptionsString, TEXT("Experience")))
	{
		const FString ExperienceFromOptions = UGameplayStatics::ParseOption(OptionsString, TEXT("Experience"));
		ExperienceId = FPrimaryAssetId(FPrimaryAssetType(UNoodlingExperienceDefinition::StaticClass()->GetFName()), FName(*ExperienceFromOptions));
		ExperienceIdSource = TEXT("OptionsString");
	}

	/*
	 * Check if there is an ExperienceOverride
	 * set in Edit > Editor Preferences… > Noodling Developer Settings,
	 * this loads the chosen Experience in whatever the currently open map is.
	 */
	if (!ExperienceId.IsValid() && World->IsPlayInEditor())
	{
		ExperienceId = GetDefault<UNoodlingDeveloperSettings>()->ExperienceOverride;
		ExperienceIdSource = TEXT("DeveloperSettings");
	}

	/*
	 * Check if “Experience=<ExperienceName>” is in the CommandLine if opening a map from the CommandLine
	 */
	if (!ExperienceId.IsValid())
	{
		if (FString ExperienceFromCommandLine; FParse::Value(FCommandLine::Get(), TEXT("Experience="), ExperienceFromCommandLine))
		{
			ExperienceId = FPrimaryAssetId::ParseTypeAndName(ExperienceFromCommandLine);
			if (!ExperienceId.PrimaryAssetType.IsValid())
			{
				ExperienceId = FPrimaryAssetId(FPrimaryAssetType(UNoodlingExperienceDefinition::StaticClass()->GetFName()), FName(*ExperienceFromCommandLine));
			}
			ExperienceIdSource = TEXT("CommandLine");
		}
	}

	/*
	 * If already in the editor Check if WorldSettings in the .umap has a DefaultGameplayExperience set.
	 * This setting is from ANoodlingWorldSettings
	 * which is set to WorldSettings class in the DefaultEngine.ini
	 * (See: [/Script/Engine.Engine] WorldSettingsClassName=/Script/LyraGame.LyraWorldSettings)
	 */
	if (!ExperienceId.IsValid())
	{
		if (ANoodlingWorldSettings* TypedWorldSettings = Cast<ANoodlingWorldSettings>(GetWorldSettings()))
		{
			ExperienceId = TypedWorldSettings->GetDefaultGameplayExperience();
			ExperienceIdSource = TEXT("WorldSettings");
		}
	}

	/*
	 * Check if UNoodlingAssetManager recognizes the ExperienceId if set above.
	 */
	UNoodlingAssetManager& AssetManager = UNoodlingAssetManager::Get();
	if (FAssetData Dummy /*Dummy variable created for OutAssetData parameter*/; ExperienceId.IsValid() && !AssetManager.GetPrimaryAssetData(ExperienceId, /*OutAssetData=*/Dummy))
	{
		UE_LOG(NoodleBranch_LogExperience,
			Error,
			TEXT("EXPERIENCE: Wanted to use [%s] but couldn't find it, falling back to default experience"), *ExperienceId.ToString());
		ExperienceId = FPrimaryAssetId();
	}

	if (!ExperienceId.IsValid())
	{
		if (TryDedicatedServerLogin())
		{
			// This will start to host as a dedicated server
			return;
		}

		//@TODO: Pull this from a config setting or something
		ExperienceId = FPrimaryAssetId(FPrimaryAssetType("NoodlingExperienceDefinition"), FName("NoodlingDefaultExperience"));
		ExperienceIdSource = TEXT("Default");
	}
	OnMatchAssignmentGiven(ExperienceId, ExperienceIdSource);
}

bool ANoodlingGameMode::TryDedicatedServerLogin()
{
	// Some basic code to register as an active dedicated server, this would be heavily modified by the game
	const FString DefaultMap = UGameMapsSettings::GetGameDefaultMap(); //Requires PrivateDependencyModuleNames "EngineSettings" in Build.cs
	const UWorld* World = GetWorld();
	if (const UGameInstance* GameInstance = GetGameInstance(); GameInstance && World && World->GetNetMode() == NM_DedicatedServer && World->URL.Map == DefaultMap)
	{
		// Only register if this is the default map on a dedicated server
		UCommonUserSubsystem* UserSubsystem = GameInstance->GetSubsystem<UCommonUserSubsystem>();

		// Dedicated servers may need to do an online login
		UserSubsystem->OnUserInitializeComplete.AddDynamic(this, &ANoodlingGameMode::OnUserInitializedForDedicatedServer);

		// There are no local users on dedicated server, but index 0 means the default platform user which is handled by the online login code
		if (!UserSubsystem->TryToLoginForOnlinePlay(0))
		{
			OnUserInitializedForDedicatedServer(nullptr, false, FText(), ECommonUserPrivilege::CanPlayOnline, ECommonUserOnlineContext::Default);
		}

		return true;
	}
	return false;
}

void ANoodlingGameMode::HostDedicatedServerMatch(const ECommonSessionOnlineMode OnlineMode) const
{
	FPrimaryAssetType UserExperienceType = UNoodlingUserFacingExperienceDefinition::StaticClass()->GetFName();

	// Decide what UserFacingExperience to load
	FPrimaryAssetId UserExperienceId;
	if (FString UserExperienceFromCommandLine;
		FParse::Value(FCommandLine::Get(), TEXT("UserExperience="), UserExperienceFromCommandLine)||
		FParse::Value(FCommandLine::Get(), TEXT("Playlist="), UserExperienceFromCommandLine))
	{
		UserExperienceId = FPrimaryAssetId::ParseTypeAndName(UserExperienceFromCommandLine);
		if (!UserExperienceId.PrimaryAssetType.IsValid())
		{
			UserExperienceId = FPrimaryAssetId(FPrimaryAssetType(UserExperienceType), FName(*UserExperienceFromCommandLine));
		}
	}
	// Search for the matching experience, it's fine to force load them because we're in dedicated server startup
	UNoodlingAssetManager& AssetManager = UNoodlingAssetManager::Get();
	if (const TSharedPtr<FStreamableHandle> Handle = AssetManager.LoadPrimaryAssetsWithType(UserExperienceType); ensure(Handle.IsValid()))
	{
		Handle->WaitUntilComplete();
	}

	TArray<UObject*> UserExperiences;
	AssetManager.GetPrimaryAssetObjectList(UserExperienceType, UserExperiences);
	const UNoodlingUserFacingExperienceDefinition* FoundExperience = nullptr;
	const UNoodlingUserFacingExperienceDefinition* DefaultExperience = nullptr;

	for (UObject* Object : UserExperiences)
	{
		if (const UNoodlingUserFacingExperienceDefinition* UserExperience = Cast<UNoodlingUserFacingExperienceDefinition>(Object); ensure(UserExperience))
		{
			if (UserExperience->GetPrimaryAssetId() == UserExperienceId)
			{
				FoundExperience = UserExperience;
				break;
			}
			
			if (UserExperience->bIsDefaultExperience && DefaultExperience == nullptr)
			{
				DefaultExperience = UserExperience;
			}
		}
	}

	if (FoundExperience == nullptr)
	{
		FoundExperience = DefaultExperience;
	}

	if (const UGameInstance* GameInstance = GetGameInstance(); ensure(FoundExperience && GameInstance))
	{
		// Actually host the game
		if (UCommonSession_HostSessionRequest* HostRequest = FoundExperience->CreateHostingRequest(this); ensure(HostRequest))
		{
			HostRequest->OnlineMode = OnlineMode;

			// TODO override other parameters?

			UCommonSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UCommonSessionSubsystem>();
			SessionSubsystem->HostSession(nullptr, HostRequest);
			
			// This will handle the map travel
		}
	}
}

void ANoodlingGameMode::OnUserInitializedForDedicatedServer(const UCommonUserInfo* UserInfo, const bool bSuccess, FText Error,
	ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext)
{
	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		// Unbind
		UCommonUserSubsystem* UserSubsystem = GameInstance->GetSubsystem<UCommonUserSubsystem>();
		UserSubsystem->OnUserInitializeComplete.RemoveDynamic(this, &ANoodlingGameMode::OnUserInitializedForDedicatedServer);

		// Dedicated servers do not require user login, but some online subsystems may expect it
		if (bSuccess && ensure(UserInfo))
		{
			UE_LOG(NoodleBranch_LogExperience,
				Log,
				TEXT("Dedicated server user login succeeded for id %s, starting online server"), *UserInfo->GetNetId().ToString());
		}
		else
		{
			UE_LOG(NoodleBranch_LogExperience,
				Log,
				TEXT("Dedicated server user login unsuccessful, starting online server as login is not required"));
		}
		
		HostDedicatedServerMatch(ECommonSessionOnlineMode::Online);
	}
}
