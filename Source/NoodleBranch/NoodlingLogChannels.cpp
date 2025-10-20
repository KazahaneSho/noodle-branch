#include "NoodlingLogChannels.h"

DEFINE_LOG_CATEGORY(NoodleBranch_Log)
DEFINE_LOG_CATEGORY(NoodleBranch_LogExperience)
DEFINE_LOG_CATEGORY(NoodleBranch_LogAbilitySystem)
DEFINE_LOG_CATEGORY(NoodleBranch_LogTeams)

FString GetClientServerContextString(UObject* ContextObject)
{
	ENetRole Role = ROLE_None;

	if (const AActor* Actor = Cast<AActor>(ContextObject))
	{
		Role = Actor->GetLocalRole();
	}
	else if (const UActorComponent* Component = Cast<UActorComponent>(ContextObject))
	{
		Role = Component->GetOwnerRole();
	}

	if (Role != ROLE_None)
	{
		return (Role == ROLE_Authority) ? TEXT("Server") : TEXT("Client");
	}

#if WITH_EDITOR
	if (GIsEditor)
	{
		extern ENGINE_API FString GPlayInEditorContextString;
		return GPlayInEditorContextString;
	}
#endif

	return TEXT("[]");
}
