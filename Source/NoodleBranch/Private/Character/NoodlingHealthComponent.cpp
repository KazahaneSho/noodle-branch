// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/NoodlingHealthComponent.h"


UNoodlingHealthComponent::UNoodlingHealthComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UNoodlingHealthComponent::InitializeWithAbilitySystem(UNoodlingAbilitySystemComponent* InAbilitySystemComponent)
{
}

void UNoodlingHealthComponent::UnInitializeFromAbilitySystem()
{
}

void UNoodlingHealthComponent::DamageSelfDestruct(bool bFellOutOfWorld)
{
}


