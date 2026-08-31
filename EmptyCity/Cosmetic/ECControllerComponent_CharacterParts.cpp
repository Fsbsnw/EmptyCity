#include "ECControllerComponent_CharacterParts.h"
#include "ECPawnComponent_CharacterParts.h"

UECControllerComponent_CharacterParts::UECControllerComponent_CharacterParts(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UECControllerComponent_CharacterParts::BeginPlay()
{
	Super::BeginPlay();

	// 파츠 스폰은 서버 전용이므로 Possess 이벤트 구독도 Authority에서만 등록합니다.
	if (HasAuthority())
	{
		if (AController* OwningController = GetController<AController>())
		{
			OwningController->OnPossessedPawnChanged.AddDynamic(this, &ThisClass::OnPossessedPawnChanged);
		}
	}
}

void UECControllerComponent_CharacterParts::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 종료 시 현재 Pawn에서 파츠를 모두 제거합니다.
	RemoveAllCharacterParts();
	Super::EndPlay(EndPlayReason);
}

UECPawnComponent_CharacterParts* UECControllerComponent_CharacterParts::GetPawnCustomizer() const
{
	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		return ControlledPawn->FindComponentByClass<UECPawnComponent_CharacterParts>();
	}

	return nullptr;
}

void UECControllerComponent_CharacterParts::AddCharacterPart(const FECCharacterPart& NewPart)
{
	AddCharacterPartInternal(NewPart);
}

void UECControllerComponent_CharacterParts::AddCharacterPartInternal(const FECCharacterPart& NewPart)
{
	FECControllerCharacterPartEntry& NewEntry = CharacterParts.AddDefaulted_GetRef();
	NewEntry.Part = NewPart;

	// Pawn이 이미 Possess된 상태면 즉시 장착합니다. (Possess 전이면 장착은 건너뜁니다 — TODO)
	if (UECPawnComponent_CharacterParts* PawnCustomizer = GetPawnCustomizer())
	{
		NewEntry.Handle = PawnCustomizer->AddCharacterPart(NewPart);
	}
}

void UECControllerComponent_CharacterParts::RemoveAllCharacterParts()
{
	if (UECPawnComponent_CharacterParts* PawnCustomizer = GetPawnCustomizer())
	{
		for (FECControllerCharacterPartEntry& Entry : CharacterParts)
		{
			PawnCustomizer->RemoveCharacterPart(Entry.Handle);
		}
	}

	CharacterParts.Reset();
}

void UECControllerComponent_CharacterParts::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	// 기존 Pawn의 파츠를 제거하고 핸들을 무효화합니다.
	if (UECPawnComponent_CharacterParts* OldCustomizer = OldPawn ? OldPawn->FindComponentByClass<UECPawnComponent_CharacterParts>() : nullptr)
	{
		for (FECControllerCharacterPartEntry& Entry : CharacterParts)
		{
			OldCustomizer->RemoveCharacterPart(Entry.Handle);
			Entry.Handle.Reset();
		}
	}

	// 새 Pawn에 동일 정의로 재장착하고 새 핸들을 발급받습니다.
	if (UECPawnComponent_CharacterParts* NewCustomizer = NewPawn ? NewPawn->FindComponentByClass<UECPawnComponent_CharacterParts>() : nullptr)
	{
		for (FECControllerCharacterPartEntry& Entry : CharacterParts)
		{
			check(!Entry.Handle.IsValid());
			Entry.Handle = NewCustomizer->AddCharacterPart(Entry.Part);
		}
	}
}
