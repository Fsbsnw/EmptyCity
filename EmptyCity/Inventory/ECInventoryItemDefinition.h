#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ECInventoryItemDefinition.generated.h"

class UECInventoryItemInstance;
/**
 * 인벤토리 아이템에 "기능 조각"으로 붙일 수 있는 Fragment의 추상 기반 클래스입니다.
 *
 * [Fragment 패턴이란?]
 * 상속으로 기능을 늘리는 대신, 필요한 기능 조각(Fragment)만 골라 아이템에 끼워 넣는 컴포지션 방식입니다.
 * - 예: "장착 가능" 기능이 필요하면 → UECInventoryFragment_EquippableItem을 붙입니다.
 * - 아이템마다 필요한 Fragment만 조합하면 되므로, 새 기능을 추가할 때 클래스 폭발을 막을 수 있습니다.
 * - DefaultToInstanced + EditInlineNew 덕분에 에디터에서 Fragment를 인라인으로 펼쳐 바로 편집할 수 있습니다.
 */
UCLASS(Abstract, DefaultToInstanced, EditInlineNew)
class UECInventoryItemFragment : public UObject
{
	GENERATED_BODY()

public:
	/** 아이템 인스턴스가 생성 시 호출되는 함수입니다. */
	virtual void OnInstanceCreated(UECInventoryItemInstance* Instance) const {}

};

/**
 * 인벤토리 아이템 한 종류를 정의하는 데이터 에셋(설계도)입니다.
 *
 * 에셋이므로 런타임에 직접 바뀌지 않고, "이 아이템은 무엇인가"라는 고정 정보만 담습니다.
 * 수량·내구도처럼 플레이어마다 달라지는 런타임 상태는 UECInventoryItemInstance가 담당합니다.
 * 아이템의 실제 기능(장착 가능 여부 등)은 Fragments 배열에 조각을 끼워 선언적으로 구성합니다.
 */
UCLASS(Blueprintable)
class EMPTYCITY_API UECInventoryItemDefinition : public UObject
{
	GENERATED_BODY()

// ─────────────────────────────────────────────────────────────
// Fragment Query
// ─────────────────────────────────────────────────────────────
public:
	/** Fragments 중 지정한 클래스(또는 그 파생 클래스)에 해당하는 Fragment를 찾아 반환합니다. 없으면 nullptr. */
	const UECInventoryItemFragment* FindFragmentByClass(TSubclassOf<UECInventoryItemFragment> FragmentClass) const;

	
// ─────────────────────────────────────────────────────────────
// Variables
// ─────────────────────────────────────────────────────────────
public:
	/** UI에 표시할 아이템 이름입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "변수")
	FText DisplayName;

	/** 이 아이템에 끼워 둔 기능 조각 목록입니다. (장착 가능 여부, 스탯 등 아이템의 능력을 선언) */
	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadOnly, Category = "변수")
	TArray<TObjectPtr<UECInventoryItemFragment>> Fragments;

};
