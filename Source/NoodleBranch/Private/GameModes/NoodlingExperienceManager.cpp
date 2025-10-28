// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/NoodlingExperienceManager.h"

#if WITH_EDITOR

void UNoodlingExperienceManager::OnPlayInEditorBegun()
{
	ensure(GameFeaturePluginRequestCountMap.IsEmpty());
	GameFeaturePluginRequestCountMap.Empty();
}

void UNoodlingExperienceManager::NotifyOfPluginActivation(const FString& PluginURL)
{
	if (GIsEditor)
	{
		UNoodlingExperienceManager* ExperienceManagerSubsystem = GEngine->GetEngineSubsystem<UNoodlingExperienceManager>();
		check(ExperienceManagerSubsystem);

		// Track the number of requesters who activate this plugin. Multiple load/activation requests are always allowed because concurrent requests are handled.
		int32& Count = ExperienceManagerSubsystem->GameFeaturePluginRequestCountMap.FindOrAdd(PluginURL);
		++Count;
	}
}

bool UNoodlingExperienceManager::RequestToDeactivatePlugin(const FString& PluginURL)
{
	if (GIsEditor)
	{
		UNoodlingExperienceManager* ExperienceManagerSubsystem = GEngine->GetEngineSubsystem<UNoodlingExperienceManager>();
		check(ExperienceManagerSubsystem);

		// Only let the last requester to get this far deactivate the plugin
		int32& Count = ExperienceManagerSubsystem->GameFeaturePluginRequestCountMap.FindChecked(PluginURL);
		--Count;

		if (Count == 0)
		{
			ExperienceManagerSubsystem->GameFeaturePluginRequestCountMap.Remove(PluginURL);
			return true;
		}

		return false;
	}

	return true;
}

#endif
