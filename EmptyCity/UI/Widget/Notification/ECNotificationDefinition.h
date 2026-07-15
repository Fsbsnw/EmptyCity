// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DataTable.h"

#include "ECNotificationDefinition.generated.h"

class UObject;
class USoundBase;

USTRUCT(BlueprintType)
struct FNotificationDefinitionRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 알림 위젯에 표시할 내용입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName;

	/** 알림 위젯에서 실행할 사운드 데이터입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<USoundBase> Sound;

	/** 알림 위젯에 표시할 아이콘 데이터입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(DisplayThumbnail="true", AllowedClasses="Texture,MaterialInterface,SlateTextureAtlasInterface", DisallowedClasses="MediaTexture"))
	TSoftObjectPtr<UObject> Icon;

	/** 알림 위젯이 뷰포트에 표시될 시간입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DisplayDuration = 2.0f;
};