// Fill out your copyright notice in the Description page of Project Settings.


#include "Settings/NoodlingSettingsLocal.h"

UNoodlingSettingsLocal* UNoodlingSettingsLocal::Get()
{
	return GEngine ? CastChecked<UNoodlingSettingsLocal>(GEngine->GetGameUserSettings()) : nullptr;
}

void UNoodlingSettingsLocal::OnExperienceLoaded()
{
}
