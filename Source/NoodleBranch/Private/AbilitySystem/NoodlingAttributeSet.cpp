// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/NoodlingAttributeSet.h"

#include "AbilitySystem/NoodlingAbilitySystemComponent.h"

UNoodlingAttributeSet::UNoodlingAttributeSet()
{
}

UWorld* UNoodlingAttributeSet::GetWorld() const
{
	const UObject* Outer = GetOuter();
	check(Outer);

	return Outer->GetWorld();
}

UNoodlingAbilitySystemComponent* UNoodlingAttributeSet::GetNoodlingAbilitySystemComponent() const
{
	return Cast<UNoodlingAbilitySystemComponent>(GetOwningAbilitySystemComponent());
}
