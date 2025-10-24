// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/NoodlingPlayerController.h"
#include "Player/NoodlingCheatManager.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Development/NoodlingDeveloperSettings.h"
#include "Net/UnrealNetwork.h"
#include "Camera/NoodlingPlayerCameraManager.h"
#include "GameFramework/Pawn.h"
#include "CommonInputSubsystem.h"
#include "EngineUtils.h"
#include "GameMapsSettings.h"
#include "ReplaySubsystem.h"
#include "AbilitySystem/NoodlingAbilitySystemComponent.h"
#include "GameModes/NoodlingGameState.h"
#include "NoodleBranch/NoodlingGameplayTags.h"
#include "NoodleBranch/NoodlingLogChannels.h"
#include "Player/NoodlingLocalPlayer.h"
#include "Player/NoodlingPlayerState.h"
#include "Replays/NoodlingReplaySubsystem.h"
#include "Settings/NoodlingSettingsLocal.h"
#include "Settings/NoodlingSettingsShared.h"
#include "UI/NoodlingHUD.h"
#if WITH_RPC_REGISTRY
#include "Http/NoodlingGameplayRpcRegistrationComponent.h"
#include "HttpServerModule.h"
#endif //#if WITH_RPC_REGISTRY

#include UE_INLINE_GENERATED_CPP_BY_NAME(NoodlingPlayerController)

namespace NoodleBranch::Input
{
	static int32 ShouldAlwaysPlayForceFeedback = 0;
	static FAutoConsoleVariableRef CVarShouldAlwaysPlayForceFeedback
	(TEXT("NoodleBranchPlayerController.ShouldAlwaysPlayForceFeedback"),
		ShouldAlwaysPlayForceFeedback,
		TEXT("Should force feedback effects be played, even if the last input device was not a gamepad?"));
}

ANoodlingPlayerController::ANoodlingPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerCameraManagerClass = ANoodlingPlayerCameraManager::StaticClass();

#if USING_CHEAT_MANAGER
	CheatClass = UNoodlingCheatManager::StaticClass();
#endif // #if USING_CHEAT_MANAGER
}

void ANoodlingPlayerController::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

void ANoodlingPlayerController::BeginPlay()
{
	Super::BeginPlay();

	//This is for testing purposes for Http Rpc Registration
#if WITH_RPC_REGISTRY
	FHttpServerModule::Get().StartAllListeners();
	if (int32 RpcPort = 0; FParse::Value(FCommandLine::Get(), TEXT("rpcport="), RpcPort))
	{
		if (UNoodlingGameplayRpcRegistrationComponent* ObjectInstance = UNoodlingGameplayRpcRegistrationComponent::GetInstance();
			ObjectInstance &&
			ObjectInstance->IsValidLowLevel())
		{
			ObjectInstance->RegisterAlwaysOnHttpCallbacks();
			ObjectInstance->RegisterInMatchHttpCallbacks();
		}
	}
#endif
	SetActorHiddenInGame(false);
}

void ANoodlingPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ANoodlingPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Disable replicating the PC target view as it doesn't work well for replays or client-side spectating.
	// The engine TargetViewRotation is only set in APlayerController::TickActor if the server knows ahead of time that 
	// a specific pawn is being spectated, and it only replicates down for COND_OwnerOnly.
	// In client-saved replays, COND_OwnerOnly is never true and the target pawn is not always known at the time of recording.
	// To support client-saved replays, the replication of this was moved to ReplicatedViewRotation and updated in PlayerTick.
	DISABLE_REPLICATED_PROPERTY(APlayerController, TargetViewRotation);
}

void ANoodlingPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

#if WITH_SERVER_CODE && WITH_EDITOR
	if (GIsEditor && (InPawn != nullptr) && (GetPawn() == InPawn))
	{
		for (const auto& [ExecutionPhase, Cheat] : GetDefault<UNoodlingDeveloperSettings>()->CheatsToRun)
		{
			if (ExecutionPhase == ECheatExecutionTime::OnPlayerPawnPossession)
			{
				ConsoleCommand(Cheat, /*bWriteToLog=*/ true);
			}
		}
	}
#endif

	SetIsAutoRunning(false);
}

void ANoodlingPlayerController::OnUnPossess()
{
	// Make sure the pawn that is being unpossessed doesn't remain our AbilitySystemComponent's avatar actor
	if (const APawn* PawnBeingUnpossessed = GetPawn())
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(PlayerState))
		{
			if (AbilitySystemComponent->GetAvatarActor() == PawnBeingUnpossessed)
			{
				AbilitySystemComponent->SetAvatarActor(nullptr);
			}
		}
	}
	
	Super::OnUnPossess();
}

void ANoodlingPlayerController::InitPlayerState()
{
	Super::InitPlayerState();
	BroadcastOnPlayerStateChanged();
}

void ANoodlingPlayerController::CleanupPlayerState()
{
	Super::CleanupPlayerState();
	BroadcastOnPlayerStateChanged();
}

void ANoodlingPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	BroadcastOnPlayerStateChanged();

	// When we're a client connected to a remote server, the player controller may replicate later than the PlayerState and AbilitySystemComponent.
	// However, TryActivateAbilitiesOnSpawn depends on the player controller being replicated in order to check whether on-spawn abilities should
	// execute locally. Therefore, once the PlayerController exists and has resolved the PlayerState, try once again to activate on-spawn abilities.
	// On other net modes the PlayerController will never replicate late, so NoodlingASC's own TryActivateAbilitiesOnSpawn calls will succeed. The handling 
	// here is only for when the PlayerState and ASC replicated before the PC and incorrectly thought the abilities were not for the local player.
	if (GetWorld()->IsNetMode(NM_Client))
	{
		if (const ANoodlingPlayerState* NoodlingPS = GetPlayerState<ANoodlingPlayerState>())
		{
			if (UNoodlingAbilitySystemComponent* NoodlingAbilitySystemComponent = NoodlingPS->GetNoodlingAbilitySystemComponent())
			{
				NoodlingAbilitySystemComponent->RefreshAbilityActorInfo();
				NoodlingAbilitySystemComponent->TryActivateAbilitiesOnSpawn();
			}
		}
	}
}

void ANoodlingPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
}

void ANoodlingPlayerController::PlayerTick(const float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
}

void ANoodlingPlayerController::SetPlayer(UPlayer* InPlayer)
{
	Super::SetPlayer(InPlayer);

	if (const UNoodlingLocalPlayer* NoodlingLocalPlayer = Cast<UNoodlingLocalPlayer>(InPlayer))
	{
		UNoodlingSettingsShared* UserSettings = NoodlingLocalPlayer->GetSharedSettings();
		UserSettings->OnSettingChanged.AddUObject(this, &ThisClass::OnSettingsChanged);

		OnSettingsChanged(UserSettings);
	}
}

void ANoodlingPlayerController::OnSettingsChanged(const UNoodlingSettingsShared* InSettings)
{
	bForceFeedbackEnabled = InSettings->GetForceFeedbackEnabled();
}

void ANoodlingPlayerController::AddCheats(bool bForce)
{
#if USING_CHEAT_MANAGER
	Super::AddCheats(true);
#else // #if USING_CHEAT_MANAGER
	Super::AddCheats(bForce);
#endif // #else //#if USING_CHEAT_MANAGER
}

void ANoodlingPlayerController::UpdateForceFeedback(IInputInterface* InputInterface, const int32 ControllerId)
{
	if (bForceFeedbackEnabled)
	{
		if (const UCommonInputSubsystem* CommonInputSubsystem = UCommonInputSubsystem::Get(GetLocalPlayer()))
		{
			if (const ECommonInputType CurrentInputType = CommonInputSubsystem->GetCurrentInputType();
				NoodleBranch::Input::ShouldAlwaysPlayForceFeedback ||
				CurrentInputType == ECommonInputType::Gamepad ||
				CurrentInputType == ECommonInputType::Touch)
			{
				InputInterface->SetForceFeedbackChannelValues(ControllerId, ForceFeedbackValues);
				return;
			}
		}
	}
	
	InputInterface->SetForceFeedbackChannelValues(ControllerId, FForceFeedbackValues());
}

void ANoodlingPlayerController::UpdateHiddenComponents(const FVector& ViewLocation,
	TSet<FPrimitiveComponentId>& OutHiddenComponents)
{
	Super::UpdateHiddenComponents(ViewLocation, OutHiddenComponents);

	if (bHideViewTargetPawnNextFrame)
	{
		if (const AActor* const ViewTargetPawn = PlayerCameraManager ? Cast<AActor>(PlayerCameraManager->GetViewTarget()) : nullptr)
		{
			// internal helper func to hide all the components
			auto AddToHiddenComponents = [&OutHiddenComponents](const TInlineComponentArray<UPrimitiveComponent*>& InComponents)
			{
				// add every component and all attached children
				for (const UPrimitiveComponent* Comp : InComponents)
				{
					if (Comp->IsRegistered())
					{
						OutHiddenComponents.Add(Comp->GetPrimitiveSceneId());

						for (USceneComponent* AttachedChild : Comp->GetAttachChildren())
						{
							static FName Name_NoParentAutoHide(TEXT("NoParentAutoHide"));
							if (const UPrimitiveComponent* AttachChildPC = Cast<UPrimitiveComponent>(AttachedChild);
								AttachChildPC && AttachChildPC->IsRegistered() &&
								!AttachChildPC->ComponentTags.Contains(Name_NoParentAutoHide))
							{
								OutHiddenComponents.Add(AttachChildPC->GetPrimitiveSceneId());
							}
						}
					}
				}
			};

			// TODO: Solve with an interface.  Gather hidden components or something.
			// TODO: Hiding isn't awesome, sometimes you want the effect of a fade out over a proximity, needs to bubble up to designers.

			// hide pawn's components
			TInlineComponentArray<UPrimitiveComponent*> PawnComponents;
			ViewTargetPawn->GetComponents(PawnComponents);
			AddToHiddenComponents(PawnComponents);

			//// hide weapon too
			//if (ViewTargetPawn->CurrentWeapon)
			//{
			//	TInlineComponentArray<UPrimitiveComponent*> WeaponComponents;
			//	ViewTargetPawn->CurrentWeapon->GetComponents(WeaponComponents);
			//	AddToHiddenComponents(WeaponComponents);
			//}
		}

		// we consumed it, reset for next frame
		bHideViewTargetPawnNextFrame = false;
	}
}

void ANoodlingPlayerController::PreProcessInput(const float DeltaTime, const bool bGamePaused)
{
	Super::PreProcessInput(DeltaTime, bGamePaused);
}

void ANoodlingPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	if (UNoodlingAbilitySystemComponent* NoodlingAbilitySystemComponent = GetNoodlingAbilitySystemComponent())
	{
		NoodlingAbilitySystemComponent->ProcessAbilityInput(DeltaTime, bGamePaused);
	}
	
	Super::PostProcessInput(DeltaTime, bGamePaused);
}

void ANoodlingPlayerController::OnCameraPenetratingTarget()
{
	bHideViewTargetPawnNextFrame = true;
}

void ANoodlingPlayerController::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	UE_LOG(NoodleBranch_LogTeams,
		Error,
		TEXT("You can't set the team ID on a player controller (%s); it's driven by the associated player state"), *GetPathNameSafe(this));
}

FGenericTeamId ANoodlingPlayerController::GetGenericTeamId() const
{
	if (const INoodlingTeamAgentInterface* PSWithTeamInterface = Cast<INoodlingTeamAgentInterface>(PlayerState))
	{
		return PSWithTeamInterface->GetGenericTeamId();
	}
	return FGenericTeamId::NoTeam;
}

FOnNoodlingTeamIndexChangedDelegate* ANoodlingPlayerController::GetOnTeamIndexChangedDelegate()
{
	return &OnTeamChangedDelegate;
}

ANoodlingPlayerState* ANoodlingPlayerController::GetNoodlingPlayerState() const
{
	return CastChecked<ANoodlingPlayerState>(PlayerState, ECastCheckedType::NullAllowed);
}

UNoodlingAbilitySystemComponent* ANoodlingPlayerController::GetNoodlingAbilitySystemComponent() const
{
	const ANoodlingPlayerState* NoodlingPlayerState = GetNoodlingPlayerState();
	return NoodlingPlayerState ? NoodlingPlayerState->GetNoodlingAbilitySystemComponent() : nullptr;
}

ANoodlingHUD* ANoodlingPlayerController::GetNoodlingHUD() const
{
	return CastChecked<ANoodlingHUD>(GetHUD(), ECastCheckedType::NullAllowed);
}

bool ANoodlingPlayerController::TryToRecordClientReplay()
{
	// See if we should record a replay
	if (ShouldRecordClientReplay())
	{
		if (UNoodlingReplaySubsystem* ReplaySubsystem = GetGameInstance()->GetSubsystem<UNoodlingReplaySubsystem>())
		{
			if (const APlayerController* FirstLocalPlayerController = GetGameInstance()->GetFirstLocalPlayerController(); FirstLocalPlayerController == this)
			{
				// If this is the first player, update the spectator player for local replays and then record
				if (ANoodlingGameState* GameState = Cast<ANoodlingGameState>(GetWorld()->GetGameState()))
				{
					GameState->SetRecorderPlayerState(PlayerState);

					ReplaySubsystem->RecordClientReplay(this);
					return true;
				}
			}
		}
	}
	return false;
}

bool ANoodlingPlayerController::ShouldRecordClientReplay()
{
	const UWorld* World = GetWorld();
	if (const UGameInstance* GameInstance = GetGameInstance();
		GameInstance != nullptr &&
		World != nullptr &&
		!World->IsPlayingReplay() &&
		!World->IsRecordingClientReplay() &&
		NM_DedicatedServer != GetNetMode() &&
		IsLocalPlayerController())
	{
		// DefaultMap here is meant to represent the FrontEndMap set under
		// [Script/EngineSettings.GameMapSettings] GameDefaultMap= in DefaultEngine.ini
		const FString DefaultMap = UGameMapsSettings::GetGameDefaultMap();
		FString CurrentMap = World->URL.Map;

#if WITH_EDITOR
		CurrentMap = UWorld::StripPIEPrefixFromPackageName(CurrentMap, World->StreamingLevelsPrefix);
#endif
		if (CurrentMap == DefaultMap)
		{
			// Never record demos on the default frontend map, this could be replaced with a better check for being in the main menu
			return false;
		}

		if (const UReplaySubsystem* ReplaySubsystem = GameInstance->GetSubsystem<UReplaySubsystem>())
		{
			if (ReplaySubsystem->IsRecording() || ReplaySubsystem->IsPlaying())
			{
				// Only one at a time
				return false;
			}
		}

		// If this is possible, now check the settings
		if (const UNoodlingLocalPlayer* NoodlingLocalPlayer = Cast<UNoodlingLocalPlayer>(GetLocalPlayer()))
		{
			if (NoodlingLocalPlayer->GetLocalSettings()->ShouldAutoRecordReplays())
			{
				return true;
			}
		}
	}
	return false;
}

void ANoodlingPlayerController::SetIsAutoRunning(const bool bEnabled)
{
	if (const bool bIsAutoRunning = GetIsAutoRunning(); bEnabled != bIsAutoRunning)
	{
		if (!bEnabled)
		{
			OnEndAutoRun();
		}
		else
		{
			OnStartAutoRun();
		}
	}
}

bool ANoodlingPlayerController::GetIsAutoRunning() const
{
	bool bIsAutoRunning = false;
	if (const UNoodlingAbilitySystemComponent* NoodlingAbilitySystemComponent = GetNoodlingAbilitySystemComponent())
	{
		bIsAutoRunning = NoodlingAbilitySystemComponent->GetTagCount(NoodlingGameplayTags::Status_AutoRunning) > 0;
	}
	return bIsAutoRunning;
}

void ANoodlingPlayerController::OnStartAutoRun()
{
	if (UNoodlingAbilitySystemComponent* NoodlingAbilitySystemComponent = GetNoodlingAbilitySystemComponent())
	{
		NoodlingAbilitySystemComponent->SetLooseGameplayTagCount(NoodlingGameplayTags::Status_AutoRunning, 1);
		K2_OnStartAutoRun();
	}	
}

void ANoodlingPlayerController::OnEndAutoRun()
{
	if (UNoodlingAbilitySystemComponent* NoodlingAbilitySystemComponent = GetNoodlingAbilitySystemComponent())
	{
		NoodlingAbilitySystemComponent->SetLooseGameplayTagCount(NoodlingGameplayTags::Status_AutoRunning, 0);
		K2_OnEndAutoRun();
	}
}

void ANoodlingPlayerController::OnPlayerStateChangedTeam(UObject* TeamAgent, const int32 OldTeam, const int32 NewTeam)
{
	ConditionalBroadcastTeamChanged(this, IntegerToGenericTeamId(OldTeam), IntegerToGenericTeamId(NewTeam));
}

void ANoodlingPlayerController::OnPlayerStateChanged()
{
	// Empty, place for derived classes to implement without having to hook all the other events
}

void ANoodlingPlayerController::BroadcastOnPlayerStateChanged()
{
	OnPlayerStateChanged();

	// Unbind from the old player state, if any
	FGenericTeamId OldTeamID = FGenericTeamId::NoTeam;
	if (LastSeenPlayerState != nullptr)
	{
		if (INoodlingTeamAgentInterface* PlayerStateTeamInterface = Cast<INoodlingTeamAgentInterface>(LastSeenPlayerState))
		{
			OldTeamID = PlayerStateTeamInterface->GetGenericTeamId();
			PlayerStateTeamInterface->GetTeamChangedDelegateChecked().RemoveAll(this);
		}
	}

	// Bind to the new player state, if any
	FGenericTeamId NewTeamID = FGenericTeamId::NoTeam;
	if (PlayerState != nullptr)
	{
		if (INoodlingTeamAgentInterface* PlayerStateTeamInterface = Cast<INoodlingTeamAgentInterface>(PlayerState))
		{
			NewTeamID = PlayerStateTeamInterface->GetGenericTeamId();
			PlayerStateTeamInterface->GetTeamChangedDelegateChecked().AddDynamic(this, &ThisClass::OnPlayerStateChangedTeam);
		}
	}

	// Broadcast the team change (if it really has)
	ConditionalBroadcastTeamChanged(this, OldTeamID, NewTeamID);

	LastSeenPlayerState = PlayerState;
}

void ANoodlingPlayerController::ServerCheatAll_Implementation(const FString& Msg)
{
#if USING_CHEAT_MANAGER
	if (CheatManager)
	{
		UE_LOG(NoodleBranch_Log, Warning, TEXT("ServerCheatAll: %s"), *Msg);
		for (TActorIterator<ANoodlingPlayerController> It(GetWorld()); It; ++It)
		{
			if (ANoodlingPlayerController* NoodlingPlayerController = *It)
			{
				NoodlingPlayerController->ClientMessage(NoodlingPlayerController->ConsoleCommand(Msg));
			}
		}
	}
#endif // #if USING_CHEAT_MANAGER
}

bool ANoodlingPlayerController::ServerCheatAll_Validate(const FString& Msg)
{
	return true;
}

void ANoodlingPlayerController::ServerCheat_Implementation(const FString& Msg)
{
#if USING_CHEAT_MANAGER
	if (CheatManager)
	{
		UE_LOG(NoodleBranch_Log, Warning, TEXT("ServerCheat: %s"), *Msg);
		ClientMessage(ConsoleCommand(Msg));
	}
#endif // #if USING_CHEAT_MANAGER
}

bool ANoodlingPlayerController::ServerCheat_Validate(const FString& Msg)
{
	return true;
}


//////////////////////////////////////////////////////////////////////
// ANoodlingReplayPlayerController
//////////////////////////////////////////////////////////////////////

void ANoodlingReplayPlayerController::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// The state may go invalid at any time due to scrubbing during a replay
	if (!IsValid(FollowedPlayerState))
	{
		const UWorld* World = GetWorld();

		// Listen for changes for both recording and playback
		if (ANoodlingGameState* GameState = Cast<ANoodlingGameState>(World->GetGameState()))
		{
			if (!GameState->OnRecorderPlayerStateChangedEvent.IsBoundToObject(this))
			{
				GameState->OnRecorderPlayerStateChangedEvent.AddUObject(this, &ThisClass::RecorderPlayerStateUpdated);
			}
			if (APlayerState* RecorderState = GameState->GetRecorderPlayerState())
			{
				RecorderPlayerStateUpdated(RecorderState);
			}
		}
	}
}

void ANoodlingReplayPlayerController::SmoothTargetViewRotation(APawn* TargetPawn, float DeltaSeconds)
{
	// Default behavior is to interpolate to TargetViewRotation which is set from APlayerController::TickActor, but it's not very smooth.
	Super::SmoothTargetViewRotation(TargetPawn, DeltaSeconds);
}

bool ANoodlingReplayPlayerController::ShouldRecordClientReplay()
{
	return false;
}

void ANoodlingReplayPlayerController::RecorderPlayerStateUpdated(APlayerState* NewRecorderPlayerState)
{
	if (NewRecorderPlayerState)
	{
		FollowedPlayerState = NewRecorderPlayerState;

		// Bind to when pawn changes and call now
		NewRecorderPlayerState->OnPawnSet.AddUniqueDynamic(this, &ANoodlingReplayPlayerController::OnPlayerStatePawnSet);
		OnPlayerStatePawnSet(NewRecorderPlayerState, NewRecorderPlayerState->GetPawn(), nullptr);
	}
}

void ANoodlingReplayPlayerController::OnPlayerStatePawnSet(APlayerState* ChangedPlayerState, APawn* NewPlayerPawn,
	APawn* OldPlayerPawn)
{
	if (ChangedPlayerState == FollowedPlayerState)
	{
		SetViewTarget(NewPlayerPawn);
	}
}
