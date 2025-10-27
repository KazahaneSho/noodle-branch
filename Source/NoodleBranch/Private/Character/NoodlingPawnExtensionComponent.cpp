// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/NoodlingPawnExtensionComponent.h"

UNoodlingPawnExtensionComponent::UNoodlingPawnExtensionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemComponent = nullptr;
}

void UNoodlingPawnExtensionComponent::SetPawnData(const UNoodlingPawnData* InPawnData)
{
}

void UNoodlingPawnExtensionComponent::OnAbilitySystemInitialized_RegisterAndCall(
	FSimpleMulticastDelegate::FDelegate Delegate)
{
}

void UNoodlingPawnExtensionComponent::OnAbilitySystemUninitialized_Register(
	FSimpleMulticastDelegate::FDelegate Delegate)
{
}

void UNoodlingPawnExtensionComponent::InitializeAbilitySystem(UNoodlingAbilitySystemComponent* InAbilitySystemComponent,
	AActor* InOwnerActor)
{
}

void UNoodlingPawnExtensionComponent::UnInitializeAbilitySystem()
{
}

void UNoodlingPawnExtensionComponent::HandleControllerChanged()
{
}

void UNoodlingPawnExtensionComponent::HandlePlayerStateReplicated()
{
}

void UNoodlingPawnExtensionComponent::SetupPlayerInputComponent()
{
}

// Called when the game starts
void UNoodlingPawnExtensionComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}
