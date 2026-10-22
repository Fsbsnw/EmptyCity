# 파밍·인벤토리·강화

[← README](../README.md)

팀의 초기 아이템 Definition/Instance·인벤토리 기반을 확장해 파밍부터 획득, 보관, 장비 강화로 이어지는 플레이 흐름을 연결했습니다.

> [GIF 삽입: 채집 → 월드 아이템 드롭 → 획득]

## 주요 구현

- **획득 가능 수량 계산:** 기존 스택, 빈 슬롯, 소지 무게를 고려해 추가 가능한 수량을 계산합니다.
- **부분 획득:** 수용 가능한 수량만 가져오고 잔여 아이템을 월드에 유지합니다.
- **드롭 배치:** 지면 탐색과 아이템 크기·간격을 고려해 드롭 위치를 계산합니다.
- **보관함 연동 강화:** 플레이어·보관함 재료를 합산 검증하고 소비한 뒤 강화 수치를 관련 시스템에 반영합니다.

> [GIF 삽입: 무게 제한으로 일부만 획득하는 장면]
> [이미지 삽입: 작업대 강화 화면과 플레이어/보관함 재료 소비 예시]

## 주요 코드

- [ECInventoryManagerComponent.cpp](../EmptyCity/Inventory/ECInventoryManagerComponent.cpp)
- [ItemDropComponent.cpp](../EmptyCity/Character/Enemy/Component/ItemDropComponent.cpp)
- [ECPlayerState.cpp](../EmptyCity/Character/Player/ECPlayerState.cpp)
