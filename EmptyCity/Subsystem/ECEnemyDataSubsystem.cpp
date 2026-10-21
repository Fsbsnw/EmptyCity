#include "Subsystem/ECEnemyDataSubsystem.h"

#include "Character/Enemy/ECEnemyCharacterBase.h"
#include "Data/Combat/ECHitVFXAttachmentProfile.h"
#include "Data/Enemy/ECEnemySettings.h"
#include "Data/Enemy/ECEnemyTableRow.h"
#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY_STATIC(LogECEnemyData, Log, All);

void UECEnemyDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	EnemyClassToTag.Reset();
	EnemyClassToDisplayName.Reset();
	EnemyClassToDeathScreenImage.Reset();
	EnemyClassToHitVFXAttachmentProfile.Reset();

	const UECEnemySettings* Settings = GetDefault<UECEnemySettings>();
	if (!Settings || Settings->EnemyDataTable.IsNull())
	{
		UE_LOG(LogECEnemyData, Error, TEXT("Enemy Settings에 Enemy Data Table이 설정되지 않았습니다."));
		return;
	}

	EnemyDataTable = Settings->EnemyDataTable.LoadSynchronous();
	if (!IsValid(EnemyDataTable))
	{
		UE_LOG(
			LogECEnemyData,
			Error,
			TEXT("Enemy Data Table을 로드하지 못했습니다: %s"),
			*Settings->EnemyDataTable.ToSoftObjectPath().ToString());
		return;
	}

	if (EnemyDataTable->GetRowStruct() != FECEnemyTableRow::StaticStruct())
	{
		UE_LOG(
			LogECEnemyData,
			Error,
			TEXT("Enemy Data Table의 행 구조가 FECEnemyTableRow가 아닙니다: %s"),
			*GetNameSafe(EnemyDataTable));
		EnemyDataTable = nullptr;
		return;
	}

	for (const TPair<FName, uint8*>& RowPair : EnemyDataTable->GetRowMap())
	{
		const FECEnemyTableRow* EnemyData = reinterpret_cast<const FECEnemyTableRow*>(RowPair.Value);
		UClass* EnemyClass = EnemyData ? EnemyData->EnemyClass.Get() : nullptr;
		if (!IsValid(EnemyClass) || !EnemyData->EnemyTag.IsValid())
		{
			UE_LOG(
				LogECEnemyData,
				Warning,
				TEXT("EnemyClass 또는 EnemyTag가 지정되지 않은 행입니다: %s"),
				*RowPair.Key.ToString());
			continue;
		}

		if (!EnemyClass->HasAnyClassFlags(CLASS_Native))
		{
			UE_LOG(
				LogECEnemyData,
				Warning,
				TEXT("Native C++ 클래스가 아닌 EnemyClass는 등록할 수 없습니다: %s (%s)"),
				*RowPair.Key.ToString(),
				*GetNameSafe(EnemyClass));
			continue;
		}

		if (const FGameplayTag* ExistingTag = EnemyClassToTag.Find(EnemyClass))
		{
			UE_LOG(
				LogECEnemyData,
				Error,
				TEXT("동일한 EnemyClass를 참조하는 행이 중복되었습니다: %s (%s, %s)"),
				*GetNameSafe(EnemyClass),
				*ExistingTag->ToString(),
				*EnemyData->EnemyTag.ToString());
			continue;
		}

		EnemyClassToTag.Add(EnemyClass, EnemyData->EnemyTag);

		FText EnemyDisplayName = EnemyData->EnemyName;
		if (EnemyDisplayName.IsEmpty())
		{
			FString EnemyTagString = EnemyData->EnemyTag.ToString();
			FString EnemyTagLeaf;
			if (!EnemyTagString.Split(TEXT("."), nullptr, &EnemyTagLeaf, ESearchCase::CaseSensitive, ESearchDir::FromEnd))
			{
				EnemyTagLeaf = MoveTemp(EnemyTagString);
			}

			EnemyDisplayName = FText::FromString(EnemyTagLeaf);
		}

		EnemyClassToDisplayName.Add(EnemyClass, MoveTemp(EnemyDisplayName));
		EnemyClassToDeathScreenImage.Add(EnemyClass, EnemyData->PlayerDeathScreenImage);
		EnemyClassToHitVFXAttachmentProfile.Add(EnemyClass, EnemyData->HitVFXAttachmentProfile);
	}

	UE_LOG(
		LogECEnemyData,
		Log,
		TEXT("Enemy Data Table 로드 완료: %s (%d개 행, %d개 Class)"),
		*GetNameSafe(EnemyDataTable),
		EnemyDataTable->GetRowMap().Num(),
		EnemyClassToTag.Num());
}

void UECEnemyDataSubsystem::Deinitialize()
{
	EnemyClassToTag.Reset();
	EnemyClassToDisplayName.Reset();
	EnemyClassToDeathScreenImage.Reset();
	EnemyClassToHitVFXAttachmentProfile.Reset();
	EnemyDataTable = nullptr;
	Super::Deinitialize();
}

UECEnemyDataSubsystem& UECEnemyDataSubsystem::Get(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert);
	check(World);

	UECEnemyDataSubsystem* Subsystem = UGameInstance::GetSubsystem<UECEnemyDataSubsystem>(World->GetGameInstance());
	check(Subsystem);
	return *Subsystem;
}

FGameplayTag UECEnemyDataSubsystem::FindEnemyTag(UClass* EnemyClass) const
{
	UClass* NativeEnemyClass = ResolveNativeEnemyClass(EnemyClass);
	if (!NativeEnemyClass)
	{
		return FGameplayTag();
	}

	if (const FGameplayTag* EnemyTag = EnemyClassToTag.Find(NativeEnemyClass))
	{
		return *EnemyTag;
	}

	return FGameplayTag();
}

FGameplayTag UECEnemyDataSubsystem::GetEnemyTag(TSubclassOf<AECEnemyCharacterBase> EnemyClass) const
{
	return FindEnemyTag(EnemyClass.Get());
}

FText UECEnemyDataSubsystem::FindEnemyDisplayName(UClass* EnemyClass) const
{
	UClass* NativeEnemyClass = ResolveNativeEnemyClass(EnemyClass);
	if (!NativeEnemyClass)
	{
		return FText::GetEmpty();
	}

	if (const FText* EnemyDisplayName = EnemyClassToDisplayName.Find(NativeEnemyClass))
	{
		return *EnemyDisplayName;
	}

	return FText::GetEmpty();
}

FText UECEnemyDataSubsystem::GetEnemyDisplayName(TSubclassOf<AECEnemyCharacterBase> EnemyClass) const
{
	return FindEnemyDisplayName(EnemyClass.Get());
}

UTexture2D* UECEnemyDataSubsystem::FindPlayerDeathScreenImage(UClass* EnemyClass) const
{
	UClass* NativeEnemyClass = ResolveNativeEnemyClass(EnemyClass);
	if (!NativeEnemyClass)
	{
		return nullptr;
	}

	const TSoftObjectPtr<UTexture2D>* DeathScreenImage = EnemyClassToDeathScreenImage.Find(NativeEnemyClass);
	return DeathScreenImage ? DeathScreenImage->LoadSynchronous() : nullptr;
}

UTexture2D* UECEnemyDataSubsystem::GetPlayerDeathScreenImage(
	TSubclassOf<AECEnemyCharacterBase> EnemyClass) const
{
	return FindPlayerDeathScreenImage(EnemyClass.Get());
}

UECHitVFXAttachmentProfile* UECEnemyDataSubsystem::FindHitVFXAttachmentProfile(
	UClass* EnemyClass) const
{
	UClass* NativeEnemyClass = ResolveNativeEnemyClass(EnemyClass);
	if (!NativeEnemyClass)
	{
		return nullptr;
	}

	const TSoftObjectPtr<UECHitVFXAttachmentProfile>* Profile =
		EnemyClassToHitVFXAttachmentProfile.Find(NativeEnemyClass);
	return Profile ? Profile->LoadSynchronous() : nullptr;
}

UECHitVFXAttachmentProfile* UECEnemyDataSubsystem::GetHitVFXAttachmentProfile(
	TSubclassOf<AECEnemyCharacterBase> EnemyClass) const
{
	return FindHitVFXAttachmentProfile(EnemyClass.Get());
}

UClass* UECEnemyDataSubsystem::ResolveNativeEnemyClass(UClass* EnemyClass)
{
	if (!IsValid(EnemyClass) || !EnemyClass->IsChildOf(AECEnemyCharacterBase::StaticClass()))
	{
		return nullptr;
	}

	UClass* NativeEnemyClass = EnemyClass;
	while (NativeEnemyClass && !NativeEnemyClass->HasAnyClassFlags(CLASS_Native))
	{
		NativeEnemyClass = NativeEnemyClass->GetSuperClass();
	}

	return NativeEnemyClass && NativeEnemyClass->IsChildOf(AECEnemyCharacterBase::StaticClass())
		? NativeEnemyClass
		: nullptr;
}
