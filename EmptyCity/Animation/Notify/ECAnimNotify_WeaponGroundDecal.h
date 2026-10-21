#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Engine/EngineTypes.h"
#include "ECAnimNotify_WeaponGroundDecal.generated.h"

class UMaterialInterface;
class USceneComponent;

/** 지면 충격 위치를 계산할 메시를 선택합니다. */
UENUM(BlueprintType)
enum class EECWeaponDecalSource : uint8
{
	/** 부착된 무기 메시 컴포넌트를 먼저 찾고, 없으면 애니메이션 메시의 소켓을 사용합니다. */
	Auto UMETA(DisplayName = "자동"),

	/** Notify가 배치된 애니메이션 메시의 소켓을 사용합니다. */
	AnimationMesh UMETA(DisplayName = "애니메이션 메시"),

	/** 캐릭터 메시 아래에 붙은 Static/Skeletal Mesh 또는 별도 무기 Actor 메시를 사용합니다. */
	AttachedWeapon UMETA(DisplayName = "부착 무기 메시"),
};

/**
 * 애니메이션의 지정 프레임에서 선택한 소켓 아래 지면을 검사해 충격 데칼을 생성합니다.
 * 발 착지처럼 실제 충돌 구간이 필요 없는 프레임 단위 지면 연출에 사용합니다.
 */
UCLASS(meta = (DisplayName = "EC Ground Impact Decal"))
class EMPTYCITY_API UECAnimNotify_WeaponGroundDecal : public UAnimNotify
{
	GENERATED_BODY()

public:
	UECAnimNotify_WeaponGroundDecal(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;

private:
	/** SocketName을 찾을 메시의 종류입니다. */
	UPROPERTY(EditAnywhere, Category = "Source", meta = (AllowPrivateAccess = "true"))
	EECWeaponDecalSource Source = EECWeaponDecalSource::Auto;

	/** 지면 위치를 검사할 소켓 또는 컴포넌트 이름입니다. */
	UPROPERTY(EditAnywhere, Category = "Source", meta = (AllowPrivateAccess = "true"))
	FName SocketName = NAME_None;

	/** 선택한 소켓의 로컬 좌표 기준 추가 오프셋입니다. */
	UPROPERTY(EditAnywhere, Category = "Source", meta = (AllowPrivateAccess = "true"))
	FVector SocketLocalOffset = FVector::ZeroVector;

	/** 소켓 위치에서 트레이스 시작점을 반대 방향으로 올리는 거리입니다. */
	UPROPERTY(EditAnywhere, Category = "Ground Trace", meta = (ClampMin = "0.0", Units = "cm", AllowPrivateAccess = "true"))
	float TraceUpDistance = 30.0f;

	/** 트레이스 시작점에서 지면 방향으로 검사하는 거리입니다. */
	UPROPERTY(EditAnywhere, Category = "Ground Trace", meta = (ClampMin = "0.0", Units = "cm", AllowPrivateAccess = "true"))
	float TraceDownDistance = 150.0f;

	/** 지면 검사에 사용할 충돌 채널입니다. */
	UPROPERTY(EditAnywhere, Category = "Ground Trace", meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	/** 생성할 Deferred Decal 재질입니다. */
	UPROPERTY(EditAnywhere, Category = "Decal", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInterface> DecalMaterial;

	/** 데칼 투영 깊이(X), 너비(Y), 높이(Z)입니다. */
	UPROPERTY(EditAnywhere, Category = "Decal", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	FVector DecalSize = FVector(16.0f, 64.0f, 64.0f);

	/** 표면 노멀을 기준으로 추가할 회전입니다. */
	UPROPERTY(EditAnywhere, Category = "Decal", meta = (AllowPrivateAccess = "true"))
	FRotator RotationOffset = FRotator::ZeroRotator;

	/** 0이면 자동 제거하지 않습니다. */
	UPROPERTY(EditAnywhere, Category = "Decal", meta = (ClampMin = "0.0", Units = "s", AllowPrivateAccess = "true"))
	float LifeSpan = 5.0f;

	/** 수명 종료 전에 페이드 아웃하는 시간입니다. */
	UPROPERTY(EditAnywhere, Category = "Decal", meta = (ClampMin = "0.0", Units = "s", AllowPrivateAccess = "true"))
	float FadeOutDuration = 1.0f;

	/** 움직이는 피격 컴포넌트에도 데칼이 따라가게 부착합니다. */
	UPROPERTY(EditAnywhere, Category = "Decal", meta = (AllowPrivateAccess = "true"))
	bool bAttachToHitComponent = true;

	/** 트레이스 결과를 월드에 표시합니다. */
	UPROPERTY(EditAnywhere, Category = "Debug", meta = (AllowPrivateAccess = "true"))
	bool bDrawDebug = false;

	USceneComponent* ResolveSourceComponent(USkeletalMeshComponent* AnimationMesh) const;
	USceneComponent* FindAttachedWeaponComponent(USkeletalMeshComponent* AnimationMesh) const;
	bool TryGetSourceTransform(USceneComponent* SourceComponent, FTransform& OutTransform) const;
};
