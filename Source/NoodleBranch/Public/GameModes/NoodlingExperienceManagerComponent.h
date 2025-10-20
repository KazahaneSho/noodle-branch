// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/GameStateComponent.h"
#include "LoadingProcessInterface.h"
#include "NoodlingExperienceManagerComponent.generated.h"

#define UE_API NOODLEBRANCH_API

class UNoodlingExperienceDefinition;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnNoodlingExperienceLoaded, const UNoodlingExperienceDefinition* /*Experience*/);

UCLASS(MinimalAPI)
class UNoodlingExperienceManagerComponent final : public UGameStateComponent, public ILoadingProcessInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties.
	UE_API explicit UNoodlingExperienceManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ILoadingProcessInterface Interface Overrides
	UE_API virtual bool ShouldShowLoadingScreen(FString& OutReason) const override;
	//~End of ILoadingProcessInterface Overrides

	// Tries to set the current experience, either UI or Gameplay
	UE_API void SetCurrentExperience(FPrimaryAssetId ExperienceId);

	// Ensures the delegate is called once the experience has been loaded.
	// If the experience has already loaded, the delegate will be called immediately.
	UE_API void CallOrRegister_OnExperienceLoaded(FOnNoodlingExperienceLoaded::FDelegate&& Delegate);

	// True if the experience is fully loaded.
	UE_API bool IsExperienceLoaded() const;

	// Returns the current experience if it's fully loaded, throws Error otherwise (i.e. It has been called too soon).
	UE_API const UNoodlingExperienceDefinition* GetCurrentExperienceChecked() const;
protected:
};

#undef UE_API
