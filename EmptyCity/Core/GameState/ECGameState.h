#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ECGameState.generated.h"

class UUIConfigDataAsset;

UCLASS()
class EMPTYCITY_API AECGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	/** @brief 게임 내에서 사용되는 모든 UI 위젯의 클래스를 총괄하는 데이터 에셋입니다.
	 *  @note HUD나 UI 관리 서브시스템이 런타임에 이 에셋을 참조하여 필요한 UI들을 동적으로 생성하고 초기화합니다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Configuration")
	TObjectPtr<UUIConfigDataAsset> UIConfigDataAsset;
};
