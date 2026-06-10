#pragma once

DECLARE_LOG_CATEGORY_EXTERN(ALogAbilitySystem, Log, All);

#define LOG_CALLINFO ANSI_TO_TCHAR(__FUNCTION__)

#define LOG_ABILITY(Format, ...) UE_LOG(ALogAbilitySystem, Warning, TEXT("%s %s"), LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__));