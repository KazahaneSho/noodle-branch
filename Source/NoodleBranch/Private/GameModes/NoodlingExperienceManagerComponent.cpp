// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/NoodlingExperienceManagerComponent.h"

UNoodlingExperienceManagerComponent::UNoodlingExperienceManagerComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

bool UNoodlingExperienceManagerComponent::ShouldShowLoadingScreen(FString& OutReason) const
{
	return false;
}

void UNoodlingExperienceManagerComponent::SetCurrentExperience(FPrimaryAssetId ExperienceId)
{
	// TODO: Implement
}

void UNoodlingExperienceManagerComponent::CallOrRegister_OnExperienceLoaded(FOnNoodlingExperienceLoaded::FDelegate&& Delegate)
{
	// TODO: Implement
}

bool UNoodlingExperienceManagerComponent::IsExperienceLoaded() const
{
	// TODO: Implement
	return false;
}

const UNoodlingExperienceDefinition* UNoodlingExperienceManagerComponent::GetCurrentExperienceChecked() const
{
	// TODO: Implement
	return nullptr;
}
