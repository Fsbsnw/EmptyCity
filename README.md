# EmptyCity

> Unreal Engine 5.5 팀 프로젝트에서 **Enemy AI·Combat**과 **UI 시스템**을 담당했습니다.

AI의 상태 판단부터 Gameplay Ability 실행, 전투 판정과 피드백까지 연결하고,
GameplayTag 기반 UI 관리와 MVVM 구조로 게임플레이 데이터와 화면 로직을 분리했습니다.  

<br><br><br><br>


## 프로젝트 목차

1. [프로젝트 개요](#프로젝트-개요)
2. [프로젝트 역할](#프로젝트-역할)
3. [적 AI & 전투](#1-적-ai--전투)
4. [UI 구조 설계](#2-ui-구조-설계)
5. [이벤트 기반 Gameplay UI](#3-이벤트-기반-gameplay-ui)
6. [추가 구현](#추가-구현)

<br><br><br><br>

## 프로젝트 개요

| 항목 | 내용 |
|---|---|
| 프로젝트 형태 | Unreal Engine 5.5 기반 팀 프로젝트 |
| 담당 영역 | Enemy AI·Combat, UI |
| 언어 | C++, Blueprint |
| 주요 기술 | Gameplay Ability System, Gameplay Tags, StateTree, AI Perception, UMG, MVVM |

<br><br><br><br>

## 프로젝트 역할

| 영역 | 주요 구현 |
|---|---|
| Enemy AI & Combat | AI Perception·StateTree·GAS를 연결한 적 행동 및 전투 구조 |
| UI Architecture | GameplayTag 기반 위젯 생명주기와 레이어·포커스·닫기 순서를 관리하는 UIManager |
| Gameplay UI | 월드 위치 기반 Indicator와 메시지 기반 Notification을 프로젝트 구조에 맞게 이식·연동 |

<br><br><br><br>

## 1. 적 AI & 전투

### StateTree 기반 적 행동 구조

<img width="1072" height="685" alt="Image" src="https://github.com/user-attachments/assets/50c5ad31-b7a1-4443-85cb-052e17b27ea3" />

AI Controller는 감지 정보와 전투 대상을 관리하고, StateTree는 순찰·추적·공격과 상태 전환을 담당하도록 책임을 나눴습니다.
빙의한 적 캐릭터의 StateTree 에셋을 런타임에 적용하고, 공통 행동은 재사용 가능한 노드로 분리했습니다.

- AI Perception 결과로 시야 여부, 마지막 확인 위치, 전투 대상을 갱신하고 교전 Event 전송
- 공격 Ability의 활성 상태, 사거리, 쿨다운, GameplayTag를 Task·Condition·Evaluator에서 판단
- State 진입·이탈과 GameplayEffect의 적용·제거 시점을 연결
- 플레이어와 멀어진 AI의 StateTree와 Tick을 일시 정지하는 거리 기반 LOD 구성

<img width="1307" height="625" alt="Image" src="https://github.com/user-attachments/assets/8f5dc1d0-f1f3-43b6-8090-9b5f69076692" />

<img width="1307" height="625" alt="Image" src="https://github.com/user-attachments/assets/55f479d1-b177-49f3-989e-727e46f164a4" />

관련 코드:
[ECEnemyAIController.cpp](./EmptyCity/Character/Enemy/AI/ECEnemyAIController.cpp) ·
[STTask_ActivateAbility.cpp](./EmptyCity/Character/Enemy/AI/Statetree/STTask/STTask_ActivateAbility.cpp) ·
[STTask_ApplyGEWhileActive.cpp](./EmptyCity/Character/Enemy/AI/Statetree/STTask/STTask_ApplyGEWhileActive.cpp)

<br><br><br><br>

### GAS 기반 근접 전투 파이프라인

공격 실행, 판정, 데미지 계산, 피격 반응을 Ability, Trace Component, GameplayEffect, GameplayCue로 분리했습니다.

```text
StateTree → Attack Ability → Montage Trace Window → HitResult
          → Parry 또는 Damage GE → ExecCalc → GameplayCue
```

- 공격 구간에만 Trace를 활성화하고 한 공격에서 같은 대상을 중복 처리하지 않도록 관리
- 패링 태그와 공격 방향의 내적으로 전방 패링 여부 판정
- `SetByCaller`로 무기 공격력과 배율을 전달하고, ExecCalc에서 가드·체력 피해를 분기
- 공격자·무기·공격 타입 태그에 따라 Hit Stop, VFX, SFX 데이터 선택

관련 코드:
[ECEnemyDamageAbility_Melee.cpp](./EmptyCity/AbilitySystem/Ability/Enemy/ECEnemyDamageAbility_Melee.cpp) ·
[EnemyMeleeTraceComponent.cpp](./EmptyCity/Character/Enemy/Component/EnemyMeleeTraceComponent.cpp) ·
[ECExecCalc_Damage.cpp](./EmptyCity/AbilitySystem/ExecCalc/ECExecCalc_Damage.cpp)

<br><br><br><br>

### Troubleshooting: 패링 반응과 행동 완료 시점 동기화

#### 문제

StateTree의 공격 Task는 공격 Ability가 활성 상태인 동안 `Running`을 유지합니다.
패링 직후 공격 Ability까지 종료하면, 패링 반응 몽타주가 재생 중인데도 StateTree가 다음 이동 상태로 전환하여 이동과 애니메이션이 겹쳤습니다.
반대로 공격 Ability를 유지하기만 하면 Trace와 Weapon Trail이 남을 수 있었습니다.

<img width="1307" height="625" alt="Image" src="https://github.com/user-attachments/assets/387ef46f-e58b-4c75-a5b7-e11d739981c0" />

#### 해결

공격 판정·연출의 정리 시점과 StateTree가 판단하는 논리적 공격 종료 시점을 분리했습니다.

1. 패링으로 공격 몽타주가 취소되면 Trace와 Trail은 즉시 종료합니다.
2. 공격 Ability는 유지하여 StateTree의 공격 Task를 `Running` 상태로 둡니다.
3. 패링 반응 Ability가 몽타주 완료·취소를 관리하고, 반응 종료 시 공격 Ability를 정리합니다.

#### 결과

패링 반응 중 이동하는 현상과 공격 판정·Trail이 남는 현상을 함께 방지하고,
반응이 끝난 뒤에만 StateTree가 다음 행동으로 전환하도록 동기화했습니다.


관련 코드:
[ECEnemyDamageAbility_Melee.cpp](./EmptyCity/AbilitySystem/Ability/Enemy/ECEnemyDamageAbility_Melee.cpp) ·
[ECGameplayAbility_Parried.cpp](./EmptyCity/AbilitySystem/Ability/Enemy/ECGameplayAbility_Parried.cpp) ·
[STTask_ActivateAbility.cpp](./EmptyCity/Character/Enemy/AI/Statetree/STTask/STTask_ActivateAbility.cpp)

> **추가 구현:** Stun 상태에서 피격 Event를 받으면 남은 시간에 연장 시간을 더하도록 구성했습니다.
> [ECGameplayAbility_Stun.cpp](./EmptyCity/AbilitySystem/Ability/Enemy/ECGameplayAbility_Stun.cpp)

<br><br><br><br>

## 2. UI 구조 설계

### GameplayTag 기반 UIManager와 닫기 순서

PlayerController는 위젯 클래스를 직접 생성하지 않고 GameplayTag로 UI 열기·닫기를 요청합니다.
UIManagerSubsystem은 DataAsset 설정을 바탕으로 생성, 레이어 배치, 캐싱, 포커스와 입력 모드를 공통 경로에서 처리합니다.

- HUD·일반 Window·System UI를 MainLayout의 레이어로 분리
- 활성 위젯 스택에 따라 Z-Order, 포커스, 마우스 커서와 입력 모드 갱신
- 위젯별 캐시 정책에 따라 닫을 때 숨기거나 제거
- 최상위 창은 전역 스택, 상세 팝업은 부모의 자식 스택으로 관리하여 최근 팝업부터 닫기
- Fade가 필요한 UI도 연출 완료 후 동일한 Toggle 경로 실행

<img width="1307" height="625" alt="Image" src="https://github.com/user-attachments/assets/12fc3517-bed0-48ff-b06b-7aebfbe0f9b3" />

관련 코드:
[UIManagerSubsystem.cpp](./EmptyCity/UI/Subsystem/UIManagerSubsystem.cpp) ·
[ECUserWidget.cpp](./EmptyCity/UI/Widget/ECUserWidget.cpp) ·
[UIConfigDataAsset.h](./EmptyCity/Data/UI/UIConfigDataAsset.h)

<br><br><br><br>

### ContextActor 기반 ViewModel 생성과 주입

위젯이 Subsystem이나 Actor를 직접 탐색하지 않도록 ViewModel 생성 경로를 공통화했습니다.

```text
UIManager → Widget 생성 → ContextActor 기반 ViewModel 생성·주입 → 초기값 Broadcast
```

- ViewModel의 Outer를 소유 위젯으로 지정하여 생명주기를 위젯에 귀속
- 보관함, 침대 등 상호작용 대상을 `ContextActor`로 전달
- Inventory 변경 Delegate를 ViewModel이 UI 갱신 Event로 변환하고 소멸 시 연결 해제

관련 코드:
[ECViewModelFactoryLibrary.cpp](./EmptyCity/UI/ViewModel/ECViewModelFactoryLibrary.cpp) ·
[InventoryInteractionViewModel.cpp](./EmptyCity/UI/ViewModel/InventoryInteractionViewModel.cpp) ·
[TimeViewModel.cpp](./EmptyCity/UI/ViewModel/TimeViewModel.cpp)

<br><br><br><br>

## 3. 이벤트 기반 Gameplay UI

### Object Indicator

월드 위치를 기준으로 상호작용 UI를 표시하기 위해 Lyra의 Indicator System을 프로젝트 범위에 맞게 이식했습니다.
상호작용 Ability는 Descriptor에 대상과 위젯 정보만 전달하고, IndicatorManagerComponent가 생성과 제거를 담당합니다.

<img width="720" alt="월드 위치 기반 상호작용 Indicator" src="https://github.com/user-attachments/assets/5b4c3ce1-cd44-4c04-980c-fefac81f482c" />

관련 코드:
[ECGameplayAbility_Interact.cpp](./EmptyCity/AbilitySystem/Ability/Player/ECGameplayAbility_Interact.cpp) ·
[ECIndicatorManagerComponent.cpp](./EmptyCity/UI/IndicatorSystem/ECIndicatorManagerComponent.cpp)

<br><br><br><br>

### Gameplay Message 기반 Notification

아이템 획득, 지역 이동, 시간대 변경 Event를 같은 방식으로 표시하기 위해 Lyra의 Gameplay Message 구조를 필요한 범위로 단순화했습니다.
송신자는 GameplayTag 채널과 Payload만 전달하고, Notification Host Widget이 알림 위젯 생성을 담당합니다.

<p>
  <img width="48%" alt="지역 이동 알림" src="https://github.com/user-attachments/assets/f2b28efc-b6b8-4814-b22c-d8e6cbb15593" />
  <img width="48%" alt="아이템 획득 알림" src="https://github.com/user-attachments/assets/a4f9f1f0-7489-4611-9a4c-2f0501efcf5e" />
</p>

관련 코드:
[ECGameplayMessageSubsystem.cpp](./EmptyCity/Subsystem/ECGameplayMessageSubsystem.cpp) ·
[ECNotificationHostWidget.cpp](./EmptyCity/UI/Widget/Notification/ECNotificationHostWidget.cpp)

<br><br><br><br>

### Progression과 해금 상태

콘텐츠를 잠김, 해금, 새로 해금됐지만 아직 연출을 확인하지 않은 상태로 구분하여 해금 여부와 최초 확인 연출을 별도로 관리했습니다.

<img width="520" alt="콘텐츠 해금 연출" src="https://github.com/user-attachments/assets/25c1533c-32a4-416d-9bdc-eedd6717f3bf" />

관련 코드:
[ECProgressionSubsystem.cpp](./EmptyCity/Subsystem/ECProgressionSubsystem.cpp)

<br><br><br><br>

## 추가 구현

### Inventory UI와 보관함 상호작용

Lyra 기반 Item Definition·Instance·Fragment의 UI 정보를 슬롯과 상세정보 위젯에 표시했습니다.
플레이어와 보관함 사이의 개별 이동, 전체 이동, 버리기를 ViewModel로 호출하고 변경 Event에 따라 양쪽 UI를 갱신합니다.

<img width="1314" height="629" alt="Image" src="https://github.com/user-attachments/assets/5535fff6-1924-461c-b218-5153dac5288b" />

<img width="1308" height="629" alt="Image" src="https://github.com/user-attachments/assets/a27c52c3-cb9f-45b6-a583-9eb397255b82" />

<img width="1310" height="629" alt="Image" src="https://github.com/user-attachments/assets/d8852574-8fc9-41fa-a7bd-bb53411c21d1" />

관련 코드:
[ECInventoryManagerComponent.cpp](./EmptyCity/Inventory/ECInventoryManagerComponent.cpp) ·
[ECPlayerInventoryWidget.cpp](./EmptyCity/UI/Widget/Inventory/Player/ECPlayerInventoryWidget.cpp) ·
[InventoryInteractionViewModel.cpp](./EmptyCity/UI/ViewModel/InventoryInteractionViewModel.cpp)

<br><br><br><br>

### 맵 노드 선택 기반 확대·포커싱

맵 노드를 선택하면 중심 좌표를 계산하고, 해당 위치가 고정 화면 중앙에 오도록 Scale과 Translation 목표값을 산출합니다.
현재 Transform에서 목표값까지 EaseInOut 보간하며, 빈 영역이 노출되지 않도록 배율과 이동 범위를 제한했습니다.
배경을 클릭하면 초기 상태로 돌아가고, 선택한 노드의 위치에 따라 정보창을 화면 반대편에 배치합니다.

<img width="1307" height="625" alt="Image" src="https://github.com/user-attachments/assets/f9bdd125-e85f-46a4-8d01-70059e6e1e50" />

관련 코드:
[ECPannableMapWidget.cpp](./EmptyCity/UI/Widget/Map/ECPannableMapWidget.cpp) ·
[ECMapNodeWidget.cpp](./EmptyCity/UI/Widget/Map/ECMapNodeWidget.cpp) ·
[ECMapNodeInfoWidget.cpp](./EmptyCity/UI/Widget/Map/ECMapNodeInfoWidget.cpp)
