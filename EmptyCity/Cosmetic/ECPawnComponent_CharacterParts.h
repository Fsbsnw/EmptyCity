#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "CharacterPartTypes.h"
#include "CosmeticAnimationTypes.h"
#include "Components/PawnComponent.h"
#include "ECPawnComponent_CharacterParts.generated.h"

struct FECCharacterPartHandle;
class UECPawnComponent_CharacterParts;

/** 파츠 정의, 핸들, 스폰된 컴포넌트를 묶는 배열 원소입니다. */
USTRUCT()
struct FECAppliedCharacterPartEntry
{
	GENERATED_BODY()

	/** 파츠 정의(메타데이터)입니다. */
	UPROPERTY()
	FECCharacterPart Part;

	/** 이 파츠를 식별하는 핸들 값입니다. */
	UPROPERTY()
	int32 PartHandle = INDEX_NONE;

	/** 실제로 스폰된 파츠 Actor를 감싸는 컴포넌트입니다. */
	UPROPERTY()
	TObjectPtr<UChildActorComponent> SpawnedComponent = nullptr;
};

/** 파츠 목록 관리 구조체입니다. 스폰/파괴, 핸들 발급, 태그 수집을 담당합니다. */
USTRUCT()
struct FECCharacterPartList
{
	GENERATED_BODY()

	FECCharacterPartList() : OwnerComponent(nullptr)
	{}

	FECCharacterPartList(UECPawnComponent_CharacterParts* InOwnerComponent) : OwnerComponent(InOwnerComponent)
	{}

	/** Entry의 PartClass를 스폰해 캐릭터에 부착합니다. */
	bool SpawnActorForEntry(FECAppliedCharacterPartEntry& Entry);

	/** Entry에 스폰된 컴포넌트를 파괴합니다. */
	void DestroyActorForEntry(FECAppliedCharacterPartEntry& Entry);

	/** 새 파츠를 추가·스폰하고 핸들을 발급합니다. */
	FECCharacterPartHandle AddEntry(FECCharacterPart NewPart);

	/** 핸들에 해당하는 파츠를 파괴·제거합니다. */
	void RemoveEntry(FECCharacterPartHandle Handle);

	/** 장착된 파츠들의 GameplayTag를 수집해 합칩니다. */
	FGameplayTagContainer CollectCombinedTags() const;

	/** 현재 장착된 파츠 목록입니다. */
	UPROPERTY()
	TArray<FECAppliedCharacterPartEntry> Entries;

	/** BroadcastChanged() 호출을 위한 소유 컴포넌트입니다. */
	UPROPERTY()
	TObjectPtr<UECPawnComponent_CharacterParts> OwnerComponent;

	/** 핸들 고유성을 보장하는 카운터입니다. */
	int32 PartHandleCounter = 0;
};

/** Pawn에 부착되어 파츠 스폰/파괴와 SkeletalMesh 갱신을 담당하는 컴포넌트입니다. */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EMPTYCITY_API UECPawnComponent_CharacterParts : public UPawnComponent
{
	GENERATED_BODY()

public:
	UECPawnComponent_CharacterParts(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** 캐릭터의 SkeletalMeshComponent를 반환합니다(ACharacter일 때만). */
	USkeletalMeshComponent* GetParentMeshComponent() const;

	/** 파츠를 부착할 SceneComponent를 반환합니다(Mesh 우선, 없으면 Root). */
	USceneComponent* GetSceneComponentToAttachTo() const;

	/** 장착된 파츠들의 태그를 합산해 반환합니다(Prefix 지정 시 필터링). */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = Cosmetics)
	FGameplayTagContainer GetCombinedTags(FGameplayTag RequiredPrefix) const;

	/** 파츠 변경 시 태그를 재수집해 최적 Mesh를 적용합니다. */
	void BroadcastChanged();

	/** 새 파츠를 추가하고 핸들을 반환합니다. */
	FECCharacterPartHandle AddCharacterPart(const FECCharacterPart& NewPart);

	/** 핸들로 파츠를 제거합니다. */
	void RemoveCharacterPart(FECCharacterPartHandle Handle);

	/** 파츠 목록입니다. */
	UPROPERTY()
	FECCharacterPartList CharacterPartList;

	/** 태그 기반 SkeletalMesh 선택 규칙 세트입니다. */
	UPROPERTY(EditAnywhere, Category = Cosmetics)
	FECAnimBodyStyleSelectionSet BodyMeshes;
};
