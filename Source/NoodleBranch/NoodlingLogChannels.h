#pragma once

#include "Logging/LogMacros.h"

class UObject;

NOODLEBRANCH_API DECLARE_LOG_CATEGORY_EXTERN(NoodleBranch_Log, Log, All);
NOODLEBRANCH_API DECLARE_LOG_CATEGORY_EXTERN(NoodleBranch_LogExperience, Log, All);
NOODLEBRANCH_API DECLARE_LOG_CATEGORY_EXTERN(NoodleBranch_LogAbilitySystem, Log, All);
NOODLEBRANCH_API DECLARE_LOG_CATEGORY_EXTERN(NoodleBranch_LogTeams, Log, All);

NOODLEBRANCH_API FString GetClientServerContextString(UObject* ContextObject = nullptr);
