#pragma once

#include "CoreMinimal.h"
#include "CharacterPartTypes.generated.h"

/** 스폰된 파츠를 식별하는 핸들입니다. INDEX_NONE이면 무효합니다. */
USTRUCT(BlueprintType)
struct FECCharacterPartHandle
{
	GENERATED_BODY()

public:
	/** 핸들을 무효화합니다. */
	void Reset() { PartHandle = INDEX_NONE; }

	bool IsValid() const { return PartHandle != INDEX_NONE; }

	UPROPERTY()
	int32 PartHandle = INDEX_NONE;
};

/** 파츠 하나의 정의(메타데이터)입니다. 스폰할 클래스와 부착 소켓만 보관합니다. */
USTRUCT(BlueprintType)
struct FECCharacterPart
{
	GENERATED_BODY()

public:
	/** 파츠로 스폰할 Actor 클래스입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> PartClass;

	/** 부착할 소켓(본) 이름입니다. None이면 루트에 부착합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SocketName;
};
