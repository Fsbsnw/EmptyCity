# EmptyCity

개발 파트 : Enemy, UI 담당

## 목차
1. [Enemy](#enemy)
2. [UI](#ui)

## Enemy

### 기능 분류
* [AI Perception](#ai-perception-기반의-플레이어-탐지-및-공격)
* [Enemy 스턴](#enemy-스턴)

<br><br>

> ### AI Perception 기반의 플레이어 탐지 및 공격
>> <img width="400" height="225" alt="공격" src="https://github.com/user-attachments/assets/79e459f9-ee5c-4aa3-887d-6b680e37545a" />
>>
>> [ECEnemyAIController.cpp](https://github.com/Fsbsnw/EmptyCity/blob/main/EmptyCity/Character/Enemy/AI/ECEnemyAIController.cpp#L125)
>
> <br><br>
>
> ### Enemy 스턴
>> GIF
>>
>> [ECGameplayAbility_Stun.cpp](https://github.com/Fsbsnw/EmptyCity/blob/main/EmptyCity/AbilitySystem/Ability/Enemy/ECGameplayAbility_Stun.cpp)
>

<br><br>

## UI

### 기능 분류
* [위젯 간 계층 관계](#위젯-간-계층-관계)
* [스택 방식의 Close 기능](#스택-방식의-close-기능)
* [위젯의 포커싱 기능](#위젯의-포커싱-기능)
* [위젯의 키보드 입력 기능](#위젯의-키보드-입력-기능)

<br><br>

> ### 위젯 간 계층 관계
>> 부모-자식 관계를 설정하여 부모 위젯이 닫힐 때 자식도 같이 제거합니다.
>>
>> GIF
>> 
>> [ECUserWidget.cpp - ClearAllChildren](https://github.com/Fsbsnw/EmptyCity/blob/main/EmptyCity/UI/Widget/ECUserWidget.cpp#L39)
>
> <br><br>
>
> ### 스택 방식의 Close 기능
>> 위젯의 Close 입력이 들어왔을 때, 자식이 있는 경우 우선적으로 처리합니다.
>> 
>> GIF
>> 
>> [ECUserWidget.cpp - HandleCloseRequest](https://github.com/Fsbsnw/EmptyCity/blob/main/EmptyCity/UI/Widget/ECUserWidget.cpp#L67)
>>
>> [UIManagerSubsystem.cpp - CloseTopWidget](https://github.com/Fsbsnw/EmptyCity/blob/main/EmptyCity/UI/Subsystem/UIManagerSubsystem.cpp#L275)
>
> <br><br>
> 
> ### 위젯의 포커싱 기능
>> 위젯에 마우스 입력이 들어왔을 때, 포커스를 획득/해제합니다.
>> 
>> <img width="400" height="225" alt="포커싱" src="https://github.com/user-attachments/assets/4bd3ef7e-4045-452f-83c0-609a976dd0e0" />
>> 
>> [ECUserWidget.cpp - HandleCloseRequest](https://github.com/Fsbsnw/EmptyCity/blob/main/EmptyCity/UI/Widget/ECUserWidget.cpp#L67)
>>
>> [UIManagerSubsystem.cpp - CloseTopWidget](https://github.com/Fsbsnw/EmptyCity/blob/main/EmptyCity/UI/Subsystem/UIManagerSubsystem.cpp#L275)
>
> <br><br>
>
> ### 위젯의 키보드 입력 기능
>> 키보드 입력이 필요한 위젯이 열려 있는 경우, 해당 위젯에 필요한 IMC를 바탕으로 입력을 처리합니다.
>> 
>> <img width="400" height="225" alt="Video Project 2" src="https://github.com/user-attachments/assets/22bbe2c5-f105-435b-a9b1-524ef55a0788" />
>> 
>> [ECMapWidget.cpp](https://github.com/Fsbsnw/EmptyCity/blob/main/EmptyCity/UI/Widget/Map/ECMapWidget.cpp#L64)

