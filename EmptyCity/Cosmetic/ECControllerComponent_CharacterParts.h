#pragma once

#include "CoreMinimal.h"
#include "CharacterPartTypes.h"
#include "Components/ControllerComponent.h"
#include "ECControllerComponent_CharacterParts.generated.h"

class UECPawnComponent_CharacterParts;

/** Controller가 보관하는 파츠 항목입니다. 정의(원본)와 현재 Pawn의 핸들을 함께 보관합니다. */
USTRUCT()
struct FECControllerCharacterPartEntry
{
	GENERATED_BODY()

	/** Controller에 영구 보관되는 파츠 정의입니다. */
	UPROPERTY(EditAnywhere)
	FECCharacterPart Part;

	/** 현재 Pawn에서 발급된 핸들입니다. Pawn 교체 시 갱신됩니다. */
	FECCharacterPartHandle Handle;
};

/**
 * Controller에 부착되어 외형 정의를 세션 내내 보관하고,
 * Pawn 전환 시 파츠를 재장착하는 컴포넌트입니다.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EMPTYCITY_API UECControllerComponent_CharacterParts : public UControllerComponent
{
	GENERATED_BODY()

public:
	UECControllerComponent_CharacterParts(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 현재 Possess 중인 Pawn의 PawnComponent를 반환합니다. 없으면 nullptr을 반환합니다. */
	UECPawnComponent_CharacterParts* GetPawnCustomizer() const;

	/** 파츠를 추가하는 외부 진입점입니다. */
	UFUNCTION(BlueprintCallable, Category = Cosmetics)
	void AddCharacterPart(const FECCharacterPart& NewPart);

	/** 모든 파츠를 Pawn에서 제거하고 목록을 비웁니다. */
	void RemoveAllCharacterParts();

	/** Possess 대상이 바뀌면 기존 Pawn에서 제거 후 새 Pawn에 재장착합니다. */
	UFUNCTION()
	void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);

	/** Controller가 보관하는 파츠 정의 목록입니다. Pawn 수명과 무관하게 유지됩니다. */
	UPROPERTY(EditAnywhere, Category = Cosmetics)
	TArray<FECControllerCharacterPartEntry> CharacterParts;

private:
	/** AddCharacterPart의 실제 구현입니다. 등록 후 Pawn이 있으면 즉시 장착합니다. */
	void AddCharacterPartInternal(const FECCharacterPart& NewPart);
};
