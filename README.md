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
>> 시각 센서를 활용하여 플레이어를 감지합니다.
>> 
>> <img width="400" height="225" alt="공격" src="https://github.com/user-attachments/assets/79e459f9-ee5c-4aa3-887d-6b680e37545a" />
>>
>> [ECEnemyAIController.cpp](https://github.com/Fsbsnw/EmptyCity/blob/main/EmptyCity/Character/Enemy/AI/ECEnemyAIController.cpp#L125)
>
> <br><br>
>
> ### Enemy 스턴
>> 스턴 GE이 적용될 때 스턴 태그를 활용하여 스턴 상태를 관리하며, 추가 공격에 따라 스턴 시간을 증가시킵니다.
>>
>> [ECGameplayAbility_Stun.cpp](https://github.com/Fsbsnw/EmptyCity/blob/main/EmptyCity/AbilitySystem/Ability/Enemy/ECGameplayAbility_Stun.cpp)
>

<br><br>

## UI

### 기능 분류
* [위젯 간 계층 관계](#위젯-간-계층-관계)
* [스택 방식의 Close 기능](#스택-방식의-close-기능)
* [위젯 드래그 및 포커싱](#위젯-드래그-및-포커싱)
* [위젯의 키보드 입력 기능](#위젯의-키보드-입력-기능)
* [해금 시스템](#해금-시스템)
* [인벤토리 위젯](#인벤토리-위젯)
* [맵 위젯 확대 축소](#맵-위젯-확대-축소)
* [오브젝트 인디케이터](#오브젝트-인디케이터)
* [게임플레이 알림 시스템](#게임플레이-알림-시스템)

<br><br>

> ### 위젯 간 계층 관계
>> 부모-자식 관계를 설정하여 부모 위젯이 닫힐 때 자식도 같이 제거합니다.
>> 
>> [ECUserWidget.cpp - ClearAllChildren](https://github.com/Fsbsnw/EmptyCity/blob/main/EmptyCity/UI/Widget/ECUserWidget.cpp#L39)
>
> <br><br>
>
> ### 스택 방식의 Close 기능
>> 위젯의 Close 입력이 들어왔을 때, 자식이 있는 경우 우선적으로 처리합니다.
>> 
>> [ECUserWidget.cpp - HandleCloseRequest](https://github.com/Fsbsnw/EmptyCity/blob/main/EmptyCity/UI/Widget/ECUserWidget.cpp#L84)
>>
>> [UIManagerSubsystem.cpp - CloseTopWidget](https://github.com/Fsbsnw/EmptyCity/blob/main/EmptyCity/UI/Subsystem/UIManagerSubsystem.cpp#L369)
>
> <br><br>
> 
> ### 위젯 드래그 및 포커싱
>> 마우스 입력에 따라 Stack 구조로 최상위 위젯 UI를 관리합니다.
>>
>> <img width="400" height="225" alt="드래그 포커스" src="https://github.com/user-attachments/assets/98760d6c-b777-4efe-8498-b9898652c62d" />
>>
>> [ECUserWidget.cpp - NativeOnPreviewMouseButtonDown](https://github.com/Fsbsnw/EmptyCity/blob/main/EmptyCity/UI/Widget/ECUserWidget.cpp#L113)
>>
>> [UIManagerSubsystem.cpp - UpdateFocusStack](https://github.com/Fsbsnw/EmptyCity/blob/main/EmptyCity/UI/Subsystem/UIManagerSubsystem.cpp#L453)
>
> <br><br>
>
> ### 위젯의 키보드 입력 기능
>> 맵 위젯의 키 입력 방향에 따라 미리 정의된 맵 노드로 포커싱을 변경합니다.
>> 
>> <img width="400" height="225" alt="Video Project 2" src="https://github.com/user-attachments/assets/22bbe2c5-f105-435b-a9b1-524ef55a0788" />
>> 
>> [ECMapWidget.cpp - NativeOnKeyDown](https://github.com/Fsbsnw/EmptyCity/blob/main/EmptyCity/UI/Widget/Map/ECMapWidget.cpp#L89)
>
> <br><br>
>
>> ### 해금 시스템
>> 잠김, 해금, New 해금 상태를 관리하며 새로 해금됐지만 해금 연출을 확인하지 않은 경우를 추가했습니다.
>> 
>> <img width="400" height="225" alt="해금" src="https://github.com/user-attachments/assets/25c1533c-32a4-416d-9bdc-eedd6717f3bf" />
>>
>> [ECProgressionSubsystem.cpp](https://github.com/Fsbsnw/EmptyCity/blob/main/EmptyCity/Subsystem/ECProgressionSubsystem.cpp)
>
> <br><br>
>
>> ### 인벤토리 위젯
>> Lyra 기반 인벤토리 구조를 활용하여 Item Instance의 UI Fragment 정보를 슬롯 위젯에 표시했습니다.
>> 
>> <img width="400" height="225" alt="인벤토리" src="https://github.com/user-attachments/assets/e98e1876-4cc9-4d58-b767-a55e4d42fadf" />
>>
>> [ECInventorySlotWidget.cpp](https://github.com/Fsbsnw/EmptyCity/blob/main/EmptyCity/UI/Widget/Inventory/ECInventorySlotWidget.cpp)
>
> <br><br>
>
>> ### 맵 위젯 확대 축소
>> 마우스 위치를 기준으로 확대/축소되도록 Scale 변화 비율에 따라 Translation을 보정했습니다.
>> 
>> <img width="400" height="225" alt="pannable맵" src="https://github.com/user-attachments/assets/5d42c727-6894-4337-9cc1-31a6f9b6d998" />
>>
>> [ECPannableMapWidget.cpp](https://github.com/Fsbsnw/EmptyCity/blob/main/EmptyCity/UI/Widget/Map/ECPannableMapWidget.cpp)
>
> <br><br>
>
>> ### 오브젝트 인디케이터
>> 플레이어 입력으로 열고 닫는 기존 UI와 달리, 상호작용 대상의 월드 위치를 기준으로 표시되는 UI가 필요했습니다.
>> Lyra의 Indicator System을 분석하여 프로젝트에 필요한 구조를 이식했으며, IndicatorDescriptor에 표시 대상과 위젯 정보를 전달하고 IndicatorManagerComponent에서 생성 및 제거를 관리하도록 구성했습니다.
>> 이를 활용하여 탐색된 상호작용 대상의 위치에 상호작용 텍스트를 표시했습니다.
>> 
>> <img width="1527" height="813" alt="Indicator-Interaction2" src="https://github.com/user-attachments/assets/5b4c3ce1-cd44-4c04-980c-fefac81f482c" />
>>
>> [ECGameplayAbility_Interact.cpp - UpdateInteractions](https://github.com/Fsbsnw/EmptyCity/blob/main/EmptyCity/AbilitySystem/Ability/ECGameplayAbility_Interact.cpp#L69)
>> 
>> [ECIndicatorManagerComponent.cpp](https://github.com/Fsbsnw/EmptyCity/blob/main/EmptyCity/UI/IndicatorSystem/ECIndicatorManagerComponent.cpp)
>
> <br><br>
>
>> ### 게임플레이 알림 시스템
>> 아이템 획득, 지역 이동, 시간대 변경 등 서로 다른 게임플레이 시스템에서 발생하는 이벤트를 UI에 표시하기 위해 메시지 기반 알림 구조를 구성했습니다.
>> Lyra의 Gameplay Message System을 프로젝트에 필요한 범위로 간소화하여 이식했으며, 송신자는 GameplayTag 채널과 메시지 데이터만 전달하고 알림 Host Widget이 메시지를 수신하여 실제 알림 위젯을 생성하도록 분리했습니다.
>> 이를 통해 각 게임플레이 시스템이 알림 위젯 클래스와 생성 방식을 직접 참조하지 않도록 구성했습니다.
>>
>> <img width="1454" height="629" alt="알림시스템-지역" src="https://github.com/user-attachments/assets/f2b28efc-b6b8-4814-b22c-d8e6cbb15593" />

>> <img width="1454" height="629" alt="알림시스템-아이템획득" src="https://github.com/user-attachments/assets/a4f9f1f0-7489-4611-9a4c-2f0501efcf5e" />
>>
>> [ECGameplayMessageSubsystem.cpp](https://github.com/Fsbsnw/EmptyCity/blob/main/EmptyCity/Subsystem/ECGameplayMessageSubsystem.cpp)
>> 
>> [ECNotificationHostWidget.cpp](https://github.com/Fsbsnw/EmptyCity/blob/main/EmptyCity/UI/Widget/Notification/ECNotificationHostWidget.cpp)
