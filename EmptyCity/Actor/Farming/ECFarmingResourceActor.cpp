#include "Actor/Farming/ECFarmingResourceActor.h"

#include "AbilitySystem/Ability/Player/ECGameplayAbility_Farm.h"
#include "Components/StaticMeshComponent.h"
#include "Item/ECItemDropComponent.h"

AECFarmingResourceActor::AECFarmingResourceActor()
{
	// 액터 기본 속성을 설정합니다.
	PrimaryActorTick.bCanEverTick = false;

	// 파밍 대상의 외형과 상호작용 콜리전을 생성합니다.
	{
		Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
		Mesh->SetCollisionProfileName(TEXT("Interactable_BlockDynamic"));
		SetRootComponent(Mesh);
	}

	// 파밍 완료 시 아이템을 생성할 드롭 컴포넌트를 생성합니다.
	ItemDropComponent = CreateDefaultSubobject<UECItemDropComponent>(TEXT("ItemDropComponent"));
}

void AECFarmingResourceActor::GatherInteractionOptions(const FInteractionQuery& InteractQuery, FInteractionOptionBuilder& OptionBuilder)
{
	const FECFarmingTableRow& FarmingData = GetFarmingData();

	// 파밍 데이터로 홀딩 상호작용 옵션을 구성해 추가합니다.
	FInteractionOption Option;
	Option.Text = FarmingData.InteractionText;
	Option.bHoldInteraction = true;
	Option.InteractionAbilityToGrant = UECGameplayAbility_Farm::StaticClass();
	OptionBuilder.AddInteractionOption(Option);
}

const FECFarmingTableRow& AECFarmingResourceActor::GetFarmingData() const
{
	return *FarmingRow.GetRow<FECFarmingTableRow>(TEXT("AECFarmingResourceActor::GetFarmingData"));
}

void AECFarmingResourceActor::CompleteFarming()
{
	// 파밍 결과를 월드 아이템으로 생성합니다.
	ItemDropComponent->SpawnDrop();

	// 파밍이 끝난 오브젝트를 영구 제거합니다.
	Destroy();
}
