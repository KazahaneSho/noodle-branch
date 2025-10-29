// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/NoodlingPlayerStart.h"

#include "GameFramework/GameModeBase.h"

ANoodlingPlayerStart::ANoodlingPlayerStart(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

ENoodlingPlayerStartLocationOccupancy ANoodlingPlayerStart::GetLocationOccupancy(
	AController* const ControllerPawnToFit) const
{
	if (UWorld* const World = GetWorld(); HasAuthority() && World)
	{
		if (AGameModeBase* AuthGameMode = World->GetAuthGameMode())
		{
			const TSubclassOf<APawn> PawnClass = AuthGameMode->GetDefaultPawnClassForController(ControllerPawnToFit);
			const APawn* const PawnToFit = PawnClass ? GetDefault<APawn>(PawnClass) : nullptr;

			FVector ActorLocation = GetActorLocation();

			if (const FRotator ActorRotation = GetActorRotation();
				!World->EncroachingBlockingGeometry(
					PawnToFit, ActorLocation, ActorRotation, nullptr))
			{
				return ENoodlingPlayerStartLocationOccupancy::Empty;
			}
			else if (World->FindTeleportSpot(PawnToFit, ActorLocation, ActorRotation))
			{
				return ENoodlingPlayerStartLocationOccupancy::Partial;
			}
		}
	}

	return ENoodlingPlayerStartLocationOccupancy::Full;
}

bool ANoodlingPlayerStart::IsClaimed() const
{
	return ClaimingController != nullptr;
}

bool ANoodlingPlayerStart::TryClaim(AController* OccupyingController)
{
	if (OccupyingController != nullptr && !IsClaimed())
	{
		ClaimingController = OccupyingController;
		if (const UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(ExpirationTimerHandle,
				FTimerDelegate::CreateUObject(this, &ANoodlingPlayerStart::CheckUnclaimed),
				ExpirationCheckInterval, true);
		}
		return true;
	}
	return false;
}

void ANoodlingPlayerStart::CheckUnclaimed()
{
	if (ClaimingController != nullptr &&
		ClaimingController->GetPawn() != nullptr &&
		GetLocationOccupancy(ClaimingController) == ENoodlingPlayerStartLocationOccupancy::Empty)
	{
		ClaimingController = nullptr;
		if (const UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(ExpirationTimerHandle);
		}
	}
}

