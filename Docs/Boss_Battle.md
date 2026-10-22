# 보스 패턴과 페이즈 전환

[← README](../README.md) · [GAS 전투](GAS_Combat.md)

보스의 행동 패턴과 체력 임계 이벤트, 전투 연출을 기존 GAS·StateTree·컷신 흐름에 연결했습니다.

> [GIF 삽입: Sprint → Leap → Slam 전체 동작]

## 대표 패턴

- **질주·도약·강하:** Ability 단계에 따라 이동과 애니메이션을 실행하고, NavMesh 경계를 고려해 목적지를 결정합니다. 종료·취소 시 이동 모드를 복원합니다.
- **체력 임계 장판:** 임계점에서 위험 지대 위치를 정하고 순차 폭발을 실행합니다. 서버 피해 판정과 클라이언트 VFX/SFX 표현을 구분합니다.
- **페이즈 컷신:** 체력 변화를 메시지로 전달하고, 컷신 동안 AI·Ability를 정리한 후 전투로 복귀합니다.

> [GIF 삽입: 장판 경고 → 순차 폭발]
> [이미지 삽입: 보스 체력 임계 → GameFlow → 컷신 흐름도]

## 주요 코드

- [ECEnemyBoss.cpp](../EmptyCity/Character/Enemy/ECEnemyBoss.cpp)
- [BossPatternComponent.cpp](../EmptyCity/Character/Enemy/Component/BossPatternComponent.cpp)
- [BossSprintLeapSlam.cpp](../EmptyCity/AbilitySystem/Ability/Enemy/Boss/BossSprintLeapSlam.cpp)
