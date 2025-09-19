# Vulkan 이미지 레이아웃 정리

## 요약 표

| 레이아웃                         | 대표 사용 예시                                    | 비고/주의                |
| -------------------------------- | ------------------------------------------------ | ------------------------ |
| `UNDEFINED`                      | 초기 상태, 새로 쓰기                              | 내용 보존 안 됨          |
| `GENERAL`                        | 스토리지 이미지(읽기/쓰기), 다목적 임시           | 유연하지만 성능 불리 가능 |
| `TRANSFER_DST_OPTIMAL`           | Clear, Copy 대상                                 | `TRANSFER_DST_BIT` 필요  |
| `TRANSFER_SRC_OPTIMAL`           | Copy, Blit 출발지                                | `TRANSFER_SRC_BIT` 필요  |
| `COLOR_ATTACHMENT_OPTIMAL`       | 렌더패스/동적 렌더링 컬러 어태치먼트              | Clear, Draw, Resolve     |
| `DEPTH_STENCIL_ATTACHMENT_OPTIMAL` | 깊이/스텐실 어태치먼트                          | Depth/Stencil 쓰기 단계   |
| `DEPTH_STENCIL_READ_ONLY_OPTIMAL`  | 읽기 전용 깊이/스텐실 샘플링·테스트             | 다음 패스 read-only 용도  |
| `SHADER_READ_ONLY_OPTIMAL`       | 텍스처 샘플링, 읽기 전용 이미지/UBO              | 셰이더 읽기 전용         |
| `PRESENT_SRC_KHR`                | 스왑체인 이미지 프레젠트 직전                    | WSI 필수                 |
| Depth/Stencil 분리 레이아웃 (KHR) | `DEPTH_ATTACHMENT_OPTIMAL` / `STENCIL_ATTACHMENT_OPTIMAL` / `DEPTH_READ_ONLY_OPTIMAL` / `STENCIL_READ_ONLY_OPTIMAL` | 세밀한 D/S 제어 |

---

## 상세 설명

### `UNDEFINED`
- **주 용도**: 이미지 생성 직후, 기존 내용을 버리고 새로 쓸 때.
- **예시 작업**: `vkCmdClearColorImage`로 초기화할 때 첫 전환 단계.
- **주의**: 이전 데이터 보존 불가 → 실제 값이 필요할 땐 절대 사용 불가.

---

### `GENERAL`
- **주 용도**: 다목적 접근이 필요한 경우. 주로 **스토리지 이미지(read/write)**.
- **예시 작업**: 
  - 컴퓨트 셰이더에서 `imageLoad`/`imageStore`.
  - 프래그먼트 셰이더에서 UAV처럼 동시에 읽고 쓰는 경우.
- **주의**: 범용성이 높지만 캐시 최적화가 깨져 성능 불리 가능.

---

### `TRANSFER_DST_OPTIMAL`
- **주 용도**: 다른 자원으로부터 데이터를 받을 때.
- **예시 작업**:
  - `vkCmdClearColorImage` (컬러 클리어 대상)
  - `vkCmdCopyBufferToImage` (버퍼 → 이미지 업로드)
- **주의**: 이미지 생성 시 `VK_IMAGE_USAGE_TRANSFER_DST_BIT` 필요.

---

### `TRANSFER_SRC_OPTIMAL`
- **주 용도**: 데이터를 다른 자원으로 복사할 때.
- **예시 작업**:
  - `vkCmdCopyImage` (이미지 → 이미지 복사)
  - `vkCmdBlitImage` (리사이즈/필터링 복사)
- **주의**: 이미지에 `VK_IMAGE_USAGE_TRANSFER_SRC_BIT` 필요.

---

### `COLOR_ATTACHMENT_OPTIMAL`
- **주 용도**: 렌더패스/동적 렌더링에서 컬러 타겟.
- **예시 작업**:
  - 프레임버퍼 렌더링 시 컬러 출력
  - `loadOp = CLEAR` → Clear 연산
  - `storeOp = STORE` → 출력 결과 보존
- **주의**: MRT(Multiple Render Target) 지원.

---

### `DEPTH_STENCIL_ATTACHMENT_OPTIMAL`
- **주 용도**: 깊이/스텐실 버퍼 **쓰기 + 테스트**.
- **예시 작업**:
  - Depth Test / Depth Write
  - Stencil Test / Write
- **주의**: 읽기 전용으로 쓸 때는 READ_ONLY 레이아웃 필요.

---

### `DEPTH_STENCIL_READ_ONLY_OPTIMAL`
- **주 용도**: 깊이/스텐실 값을 **읽기 전용**으로 참조할 때.
- **예시 작업**:
  - 프래그먼트 셰이더에서 `sampler2DShadow` 로 그림자맵 참조
  - 후처리 패스에서 깊이 텍스처 읽기
- **주의**: 같은 리소스를 다른 패스에서 공유할 때 유용.

---

### `SHADER_READ_ONLY_OPTIMAL`
- **주 용도**: 텍스처/이미지를 셰이더에서 읽을 때.
- **예시 작업**:
  - 프래그먼트 셰이더에서 텍스처 샘플링
  - 컴퓨트 셰이더에서 SRV 읽기
  - UBO(Uniform Buffer Object) 참조
- **주의**: 쓰기는 불가능 → UAV 용도는 `GENERAL` 사용.

---

### `PRESENT_SRC_KHR`
- **주 용도**: 스왑체인 이미지를 화면에 출력하기 직전.
- **예시 작업**:
  - `vkQueuePresentKHR` 호출 전 마지막 전환.
- **주의**: 반드시 이 레이아웃으로 바꿔야 화면 표시 가능.

---

### Depth/Stencil 분리 레이아웃 (KHR)
- **주 용도**: `VK_KHR_separate_depth_stencil_layouts` 확장 기능.
- **세부 레이아웃**:
  - `DEPTH_ATTACHMENT_OPTIMAL`: 깊이만 어태치먼트로 사용
  - `STENCIL_ATTACHMENT_OPTIMAL`: 스텐실만 어태치먼트로 사용
  - `DEPTH_READ_ONLY_OPTIMAL`: 깊이만 읽기 전용
  - `STENCIL_READ_ONLY_OPTIMAL`: 스텐실만 읽기 전용
- **예시 작업**:
  - G-Buffer에서 깊이만 쓰고, 스텐실은 다음 패스에서 샘플링
  - 스텐실 마스크만 갱신하는 별도 패스
