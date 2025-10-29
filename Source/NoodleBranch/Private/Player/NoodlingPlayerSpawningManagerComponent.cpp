// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/NoodlingPlayerSpawningManagerComponent.h"

#include "EngineUtils.h"
#include "Engine/PlayerStartPIE.h"
#include "GameFramework/PlayerState.h"
#include "Player/NoodlingPlayerStart.h"

DEFINE_LOG_CATEGORY_STATIC(LogPlayerSpawning, Log, All);

UNoodlingPlayerSpawningManagerComponent::UNoodlingPlayerSpawningManagerComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(false);
	bAutoRegister = true;
	bAutoActivate = true;
	bWantsInitializeComponent = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bAllowTickOnDedicatedServer = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UNoodlingPlayerSpawningManagerComponent::InitializeComponent()
{
	Super::InitializeComponent();

	FWorldDelegates::LevelAddedToWorld.AddUObject(this, &ThisClass::OnLevelAdded);

	const UWorld* World = GetWorld();
	World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateUObject(this, &ThisClass::HandleOnActorSpawned));

	for (TActorIterator<ANoodlingPlayerStart> It(World); It; ++It)
	{
		if (ANoodlingPlayerStart* PlayerStart = *It)
		{
			CachedPlayerStarts.Add(PlayerStart);
		}
	}
}

void UNoodlingPlayerSpawningManagerComponent::TickComponent(const float DeltaTime, const ELevelTick TickType,
                                                            FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

APlayerStart* UNoodlingPlayerSpawningManagerComponent::GetFirstRandomUnoccupiedPlayerStart(AController* Controller,
	const TArray<ANoodlingPlayerStart*>& FoundStartPoints)
{
	if (Controller)
	{
		TArray<ANoodlingPlayerStart*> UnOccupiedStartPoints;
		TArray<ANoodlingPlayerStart*> OccupiedStartPoints;

		for (ANoodlingPlayerStart* StartPoint : FoundStartPoints)
		{
			switch (ENoodlingPlayerStartLocationOccupancy State = StartPoint->GetLocationOccupancy(Controller))
			{
			case ENoodlingPlayerStartLocationOccupancy::Empty:
				UnOccupiedStartPoints.Add(StartPoint);
				break;
			case ENoodlingPlayerStartLocationOccupancy::Partial:
				OccupiedStartPoints.Add(StartPoint);
				break;
			default:
				// Should already be in OccupiedStartPoints.
				break;
			}
		}

		if (UnOccupiedStartPoints.Num() > 0)
		{
			return UnOccupiedStartPoints[FMath::RandRange(0, UnOccupiedStartPoints.Num() - 1)];
		}
		else if (OccupiedStartPoints.Num() > 0)
		{
			return OccupiedStartPoints[FMath::RandRange(0, OccupiedStartPoints.Num() - 1)];
		}
	}

	return nullptr;
}

// ANoodlingGameMode Proxied Calls - Need to handle when someone chooses
// to restart a player the normal way in the engine.
//======

AActor* UNoodlingPlayerSpawningManagerComponent::ChoosePlayerStart(AController* Player)
{
	if (Player)
	{
#if WITH_EDITOR
		if (APlayerStart* PlayerStart = FindPlayFromHereStart(Player))
		{
			return PlayerStart;
		}
#endif

		TArray<ANoodlingPlayerStart*> StarterPoints;
		for (auto StartIt = CachedPlayerStarts.CreateIterator(); StartIt; ++StartIt)
		{
			if (ANoodlingPlayerStart* Start = StartIt->Get())
			{
				StarterPoints.Add(Start);
			}
			else
			{
				StartIt.RemoveCurrent();
			}
		}

		if (const APlayerState* PlayerState = Player->GetPlayerState<APlayerState>())
		{
			// start dedicated spectators at any random starting location, but they do not claim it
			if (PlayerState->IsOnlyASpectator())
			{
				if (!StarterPoints.IsEmpty())
				{
					return StarterPoints[FMath::RandRange(0, StarterPoints.Num() - 1)];
				}

				return nullptr;
			}
		}

		AActor* PlayerStart = OnChoosePlayerStart(Player, StarterPoints);

		if (!PlayerStart)
		{
			PlayerStart = GetFirstRandomUnoccupiedPlayerStart(Player, StarterPoints);
		}

		if (ANoodlingPlayerStart* NoodlingStart = Cast<ANoodlingPlayerStart>(PlayerStart))
		{
			NoodlingStart->TryClaim(Player);
		}

		return PlayerStart;
	}

	return nullptr;
}

void UNoodlingPlayerSpawningManagerComponent::FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation)
{
	OnFinishRestartPlayer(NewPlayer, StartRotation);
	K2_OnFinishRestartPlayer(NewPlayer, StartRotation);
}

bool UNoodlingPlayerSpawningManagerComponent::ControllerCanRestart(AController* Player)
{
	bool bCanRestart = true;

	// @TODO: Can they restart?

	return bCanRestart;
}

//================================================================

void UNoodlingPlayerSpawningManagerComponent::OnLevelAdded(ULevel* InLevel, UWorld* InWorld)
{
	if (InWorld == GetWorld())
	{
		for (AActor* Actor : InLevel->Actors)
		{
			if (ANoodlingPlayerStart* PlayerStart = Cast<ANoodlingPlayerStart>(Actor))
			{
				ensure(!CachedPlayerStarts.Contains(PlayerStart));
				CachedPlayerStarts.Add(PlayerStart);
			}
		}
	}
}

void UNoodlingPlayerSpawningManagerComponent::HandleOnActorSpawned(AActor* SpawnedActor)
{
	if (ANoodlingPlayerStart* PlayerStart = Cast<ANoodlingPlayerStart>(SpawnedActor))
	{
		CachedPlayerStarts.Add(PlayerStart);
	}
}

#if WITH_EDITOR
APlayerStart* UNoodlingPlayerSpawningManagerComponent::FindPlayFromHereStart(const AController* Player) const
{
	// Only 'Play From Here' for a player controller, bots etc. should all spawn from normal spawn points.
	if (!Player->IsA<APlayerController>())
	{
		UE_LOG(LogPlayerSpawning, Error, TEXT("FindPlayFromHereStart() - %s : !IsA<APlayerController>"), *GetNameSafe(Player))
		return nullptr;
	}

	if (const UWorld* World = GetWorld())
	{
		for (TActorIterator<APlayerStart> It(World); It; ++It)
		{
			if (APlayerStart* PlayerStart = *It)
			{
				if (PlayerStart->IsA<APlayerStartPIE>())
				{
					// Always prefer the first "Play from Here" PlayerStart, if we find one while in PIE mode
					return PlayerStart;
				}
			}
		}
	}

	return nullptr;
}
#endif // WITH_EDITOR


