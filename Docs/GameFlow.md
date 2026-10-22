# 지역·시간·저장·컷신 GameFlow

[← README](../README.md)

지역 이동과 사망·재도전에서 서로 다른 서브시스템이 담당하는 처리를 하나의 플레이 흐름으로 연결했습니다.

> [GIF 삽입: 지도 선택 → 페이드 → 지역 도착]

## 대표 흐름: 지역 이동

`이동 요청 → 이동 조건 확인 → Fade Out → 텔레포트·지역 상태 반영 → 컷신 → Fade In·입력 복구`

- RegionSubsystem이 목적지 조건과 전환 순서를 관리하고 TimeSubsystem에 이동 규칙을 전달합니다.
- GameFlowSubsystem은 사망·보스·아침 이벤트와 Save·Cutscene 처리를 연결합니다.
- Saveable 인터페이스를 통해 각 시스템의 상태 저장·복원을 분리합니다.
- 일반 사망과 보스 지역 재도전은 체크포인트 및 복원 정책을 달리합니다.

> [이미지 삽입: Region / GameFlow / Time / Save / Cutscene 간 메시지 흐름]

## 주요 코드

- [ECGameFlowSubsystem.cpp](../EmptyCity/Subsystem/ECGameFlowSubsystem.cpp)
- [ECRegionSubsystem.cpp](../EmptyCity/Subsystem/ECRegionSubsystem.cpp)
- [ECTimeSubsystem.cpp](../EmptyCity/Subsystem/ECTimeSubsystem.cpp)
- [ECSaveSubsystem.cpp](../EmptyCity/Save/ECSaveSubsystem.cpp)
