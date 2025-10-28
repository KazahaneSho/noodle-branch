// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/GameStateComponent.h"
#include "LoadingProcessInterface.h"
#include "NoodlingExperienceManagerComponent.generated.h"

#define UE_API NOODLEBRANCH_API

namespace UE::GameFeatures
{
	struct FResult;
}

class UNoodlingExperienceDefinition;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnNoodlingExperienceLoaded, const UNoodlingExperienceDefinition* /*Experience*/);

enum class ENoodlingExperienceLoadState
{
	Unloaded,
	Loading,
	LoadingGameFeatures,
	LoadingChaosTestingDelay,
	ExecutingActions,
	Loaded,
	Deactivating
};

UCLASS(MinimalAPI)
class UNoodlingExperienceManagerComponent final : public UGameStateComponent, public ILoadingProcessInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties.
	UE_API explicit UNoodlingExperienceManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UActorComponent interface
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of UActorComponent interface

	//~ILoadingProcessInterface Interface Overrides
	UE_API virtual bool ShouldShowLoadingScreen(FString& OutReason) const override;
	//~End of ILoadingProcessInterface Overrides

	// Tries to set the current experience, either UI or Gameplay
	UE_API void SetCurrentExperience(const FPrimaryAssetId& ExperienceId);

	// Ensures the delegate is called once the experience has been loaded,
	// before others are called.
	// However, if the experience has already loaded, calls the delegate immediately.
	UE_API void CallOrRegister_OnExperienceLoaded_HighPriority(FOnNoodlingExperienceLoaded::FDelegate&& Delegate);

	// Ensures the delegate is called once the experience has been loaded.
	// If the experience has already loaded, the delegate will be called immediately.
	UE_API void CallOrRegister_OnExperienceLoaded(FOnNoodlingExperienceLoaded::FDelegate&& Delegate);

	// Ensures the delegate is called once the experience has been loaded
	// If the experience has already loaded, calls the delegate immediately
	UE_API void CallOrRegister_OnExperienceLoaded_LowPriority(FOnNoodlingExperienceLoaded::FDelegate&& Delegate);

	
	// Returns the current experience if it's fully loaded, throws Error otherwise (i.e. It has been called too soon).
	UE_API const UNoodlingExperienceDefinition* GetCurrentExperienceChecked() const;

	// True if the experience is fully loaded.
	UE_API bool IsExperienceLoaded() const;

private:
	UFUNCTION()
	void OnRep_CurrentExperience();

	void StartExperienceLoad();
	void OnExperienceLoadComplete();
	void OnGameFeaturePluginLoadComplete(const UE::GameFeatures::FResult& Result);
	void OnExperienceFullLoadCompleted();

	void OnActionDeactivationCompleted();
	void OnAllActionsDeactivated();

private:
	UPROPERTY(ReplicatedUsing=OnRep_CurrentExperience)
	TObjectPtr<const UNoodlingExperienceDefinition> CurrentExperience;

	ENoodlingExperienceLoadState LoadState = ENoodlingExperienceLoadState::Unloaded;

	int32 NumGameFeaturePluginsLoading = 0;
	TArray<FString> GameFeaturePluginURLs;

	int32 NumObservedPausers = 0;
	int32 NumExpectedPausers = 0;

	/**
	 * Delegate called when the experience has finished loading just before others
	 * (e.g., subsystems that set up for regular gameplay)
	 */
	FOnNoodlingExperienceLoaded OnExperienceLoaded_HighPriority;

	/** Delegate called when the experience has finished loading */
	FOnNoodlingExperienceLoaded OnExperienceLoaded;

	/** Delegate called when the experience has finished loading */
	FOnNoodlingExperienceLoaded OnExperienceLoaded_LowPriority;
};

#undef UE_API
