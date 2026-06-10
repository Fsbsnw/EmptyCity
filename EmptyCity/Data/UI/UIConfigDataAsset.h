// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "UIConfigDataAsset.generated.h"

class UECMainLayoutWidget;
class UInputMappingContext;
class UECViewModelBase;
class UECUserWidget;

// ============================================================================
// 열거형 정의 (Enums)
// ============================================================================
/**
 * @brief 위젯이 화면에서 닫힐 때의 메모리 관리 및 캐싱 정책
 */
UENUM(BlueprintType)
enum class EWidgetCachePolicy : uint8
{
	/** 창을 닫아도 메모리에 인스턴스를 유지합니다. (Visibility만 Hidden/Collapsed로 조절) */
	AlwaysCache     UMETA(DisplayName = "Always Cache"),
    
	/** 창을 닫으면 완전히 메모리에서 해제합니다. (RemoveFromParent 후 포인터 Null 처리) */
	DestroyOnClose  UMETA(DisplayName = "Destroy On Close")
};

/**
 * @brief 메인 레이아웃에서 추가될 레이어 계층 설정
 */
UENUM(BlueprintType)
enum class EWidgetLayerType : uint8
{
	HUD         UMETA(DisplayName = "HUD Layer (1F)"),
	Window      UMETA(DisplayName = "Window Layer (2F)")
};

// ============================================================================
// 구조체 정의 (Structs)
// ============================================================================

/**
 * @brief 단일 UI 위젯의 클래스 정보, 캐시 정책, 초기화 시 주입할 MVVM 뷰모델 목록을 정의하는 구조체
 */
USTRUCT(BlueprintType)
struct FUIConfigWidget
{
	GENERATED_BODY()

	/** 이 위젯이 닫힐 때 메모리를 유지할지, 해제할지 결정하는 정책 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI Config")
	EWidgetCachePolicy CachePolicy = EWidgetCachePolicy::DestroyOnClose;
    
	/** 생성할 실제 위젯의 블루프린트 클래스 타입 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI Config")
	TSubclassOf<UECUserWidget> WidgetClass;

	/** 위젯 생성 직후 `InjectViewModel`을 통해 자동으로 주입해 줄 뷰모델 클래스 목록 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI Config")
	TArray<TSubclassOf<UECViewModelBase>> ViewModels;

	/** 위젯이 열릴 때 덮어씌울 전용 입력 에셋 (맵 전용 IMC 등) */
	UPROPERTY(EditAnywhere, Category = "UI Config|Input")
	UInputMappingContext* OverrideIMC = nullptr;

	/** 입력 우선순위 */
	UPROPERTY(EditAnywhere, Category = "UI Config|Input")
	int32 IMCPriority = 10;

	/** 위젯이 열릴 때 마우스 커서를 보여줄지 결정 */
	UPROPERTY(EditAnywhere, Category = "UI Config|Input")
	bool bShowMouseCursor = true;

	/** 어느 레이어에 추가할 것인지 결정 */
	UPROPERTY(EditDefaultsOnly, Category = "UI Config|Layout")
	EWidgetLayerType LayerType = EWidgetLayerType::Window;
};

/**
 * @brief 게임 내 전체 UI 배치를 총괄하는 데이터 컨테이너 구조체
 */
USTRUCT(BlueprintType)
struct FUIConfigData
{
	GENERATED_BODY()

	/** 기본이 될 레이아웃 클래스 정보 */
	UPROPERTY(EditDefaultsOnly, Category = "UI Config|Layout")
	TSubclassOf<UECMainLayoutWidget> MainLayoutClass;

	/** 게임플레이 태그를 Key로 사용하여 여닫을 일반 위젯들을 매핑한 맵 데이터 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI Config|Widgets")
	TMap<FGameplayTag, FUIConfigWidget> WidgetMap;
};

/**
 * 
 */
UCLASS()
class EMPTYCITY_API UUIConfigDataAsset : public UDataAsset
{
	GENERATED_BODY()
public:	
	UPROPERTY(EditDefaultsOnly)
	FUIConfigData UIConfigData;
};
