# Vulkan Dynamic States (Core + Extended)

## 1. Core Dynamic States (Vulkan 1.0)
| Dynamic State | Command |
|---------------|---------|
| `VK_DYNAMIC_STATE_VIEWPORT` | `vkCmdSetViewport` |
| `VK_DYNAMIC_STATE_SCISSOR` | `vkCmdSetScissor` |
| `VK_DYNAMIC_STATE_LINE_WIDTH` | `vkCmdSetLineWidth` |
| `VK_DYNAMIC_STATE_DEPTH_BIAS` | `vkCmdSetDepthBias` |
| `VK_DYNAMIC_STATE_BLEND_CONSTANTS` | `vkCmdSetBlendConstants` |
| `VK_DYNAMIC_STATE_DEPTH_BOUNDS` | `vkCmdSetDepthBounds` |
| `VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK` | `vkCmdSetStencilCompareMask` |
| `VK_DYNAMIC_STATE_STENCIL_WRITE_MASK` | `vkCmdSetStencilWriteMask` |
| `VK_DYNAMIC_STATE_STENCIL_REFERENCE` | `vkCmdSetStencilReference` |

---

## 2. VK_EXT_extended_dynamic_state
| Dynamic State | Command |
|---------------|---------|
| `VK_DYNAMIC_STATE_CULL_MODE_EXT` | `vkCmdSetCullModeEXT` |
| `VK_DYNAMIC_STATE_FRONT_FACE_EXT` | `vkCmdSetFrontFaceEXT` |
| `VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY_EXT` | `vkCmdSetPrimitiveTopologyEXT` |
| `VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT_EXT` | `vkCmdSetViewportWithCountEXT` |
| `VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT_EXT` | `vkCmdSetScissorWithCountEXT` |
| `VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE_EXT` | `vkCmdBindVertexBuffers2EXT` |
| `VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE_EXT` | `vkCmdSetDepthTestEnableEXT` |
| `VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE_EXT` | `vkCmdSetDepthWriteEnableEXT` |
| `VK_DYNAMIC_STATE_DEPTH_COMPARE_OP_EXT` | `vkCmdSetDepthCompareOpEXT` |
| `VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE_EXT` | `vkCmdSetDepthBoundsTestEnableEXT` |
| `VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE_EXT` | `vkCmdSetStencilTestEnableEXT` |
| `VK_DYNAMIC_STATE_STENCIL_OP_EXT` | `vkCmdSetStencilOpEXT` |

---

## 3. VK_EXT_extended_dynamic_state2
| Dynamic State | Command |
|---------------|---------|
| `VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT` | `vkCmdSetPatchControlPointsEXT` |
| `VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE_EXT` | `vkCmdSetRasterizerDiscardEnableEXT` |
| `VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE_EXT` | `vkCmdSetDepthBiasEnableEXT` |
| `VK_DYNAMIC_STATE_LOGIC_OP_EXT` | `vkCmdSetLogicOpEXT` |
| `VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE_EXT` | `vkCmdSetPrimitiveRestartEnableEXT` |

---

## 4. VK_EXT_extended_dynamic_state3
### Rasterization
| Dynamic State | Command |
|---------------|---------|
| `VK_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT` | `vkCmdSetTessellationDomainOriginEXT` |
| `VK_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT` | `vkCmdSetDepthClampEnableEXT` |
| `VK_DYNAMIC_STATE_POLYGON_MODE_EXT` | `vkCmdSetPolygonModeEXT` |
| `VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT` | `vkCmdSetRasterizationSamplesEXT` |
| `VK_DYNAMIC_STATE_SAMPLE_MASK_EXT` | `vkCmdSetSampleMaskEXT` |
| `VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT` | `vkCmdSetAlphaToCoverageEnableEXT` |
| `VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT` | `vkCmdSetAlphaToOneEnableEXT` |
| `VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT` | `vkCmdSetLogicOpEnableEXT` |

### Color Blend
| Dynamic State | Command |
|---------------|---------|
| `VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT` | `vkCmdSetColorBlendEnableEXT` |
| `VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT` | `vkCmdSetColorBlendEquationEXT` |
| `VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT` | `vkCmdSetColorWriteMaskEXT` |
| `VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT` | `vkCmdSetColorBlendAdvancedEXT` |

### Depth / Stencil
| Dynamic State | Command |
|---------------|---------|
| `VK_DYNAMIC_STATE_DEPTH_CLIP_ENABLE_EXT` | `vkCmdSetDepthClipEnableEXT` |
| `VK_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT` | `vkCmdSetConservativeRasterizationModeEXT` |
| `VK_DYNAMIC_STATE_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT` | `vkCmdSetDepthClipNegativeOneToOneEXT` |

### Coverage / Shading
| Dynamic State | Command |
|---------------|---------|
| `VK_DYNAMIC_STATE_PROVOKING_VERTEX_MODE_EXT` | `vkCmdSetProvokingVertexModeEXT` |
| `VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT` | `vkCmdSetLineStippleEnableEXT` |
| `VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT` | `vkCmdSetSampleLocationsEnableEXT` |


### 지원 여부

```cpp
uint32_t extCount = 0;
vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, nullptr);
std::vector<VkExtensionProperties> exts(extCount);
vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, exts.data());

auto has = [&](const char* name){
    return std::any_of(exts.begin(), exts.end(),
        [&](const VkExtensionProperties& e){ return strcmp(e.extensionName, name) == 0; });
};

if (!has("VK_EXT_extended_dynamic_state3")) {
    // 사용 불가 처리
}
```

### 구조체 체인닝

```cpp
VkPhysicalDeviceFeatures2 features2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};

// VK_EXT_extended_dynamic_state3 
VkPhysicalDeviceExtendedDynamicState3FeaturesEXT eds3{
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT
};
// 실제로 쓸 것만 VK_TRUE.
eds3.extendedDynamicState3TessellationDomainOrigin = VK_TRUE;
eds3.extendedDynamicState3DepthClampEnable         = VK_TRUE;
eds3.extendedDynamicState3PolygonMode              = VK_TRUE;
eds3.extendedDynamicState3RasterizationSamples     = VK_TRUE;
eds3.extendedDynamicState3SampleMask               = VK_TRUE;
eds3.extendedDynamicState3AlphaToCoverageEnable    = VK_TRUE;
eds3.extendedDynamicState3AlphaToOneEnable         = VK_TRUE;
eds3.extendedDynamicState3LogicOpEnable            = VK_TRUE;
eds3.extendedDynamicState3ColorBlendEnable         = VK_TRUE;
eds3.extendedDynamicState3ColorBlendEquation       = VK_TRUE;
eds3.extendedDynamicState3ColorWriteMask           = VK_TRUE;
// 드라이버가 지원하는 항목만 VK_TRUE로 두세요.

features2.pNext = &eds3;
vkGetPhysicalDeviceFeatures2(physicalDevice, &features2);

```

### 디바이스 확장 추가
```cpp
const char* deviceExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    "VK_EXT_extended_dynamic_state3",
    // 필요 시 다른 확장들…
};

VkDeviceCreateInfo dci{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
dci.enabledExtensionCount   = uint32_t(std::size(deviceExtensions));
dci.ppEnabledExtensionNames = deviceExtensions;

// 위에서 채운 features2/eds3 체인을 pNext로 연결
dci.pNext = &features2;

VkDevice device = VK_NULL_HANDLE;
VK_CHECK(vkCreateDevice(physicalDevice, &dci, nullptr, &device));

```

### 파이프라인 + 다이나믹 

```cpp
std::vector<VkDynamicState> dyn = {
    // Core
    VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR,

    // extended_dynamic_state(1/2) 예시
    VK_DYNAMIC_STATE_CULL_MODE_EXT,
    VK_DYNAMIC_STATE_FRONT_FACE_EXT,
    VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY_EXT,

    // extended_dynamic_state3 예시
    VK_DYNAMIC_STATE_POLYGON_MODE_EXT,
    VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT,
    VK_DYNAMIC_STATE_SAMPLE_MASK_EXT,
    VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT,
    VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT,
    VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT,
    VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT,
    VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT,
    VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT,
    VK_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT,
    VK_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT,
    VK_DYNAMIC_STATE_DEPTH_CLIP_ENABLE_EXT,                // 드라이버 지원 시
    VK_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT,  // 드라이버 지원 시
    VK_DYNAMIC_STATE_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT,   // 드라이버 지원 시
    VK_DYNAMIC_STATE_PROVOKING_VERTEX_MODE_EXT,
    VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT,
    VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT,
};

VkPipelineDynamicStateCreateInfo dynInfo{
    VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO
};
dynInfo.dynamicStateCount = uint32_t(dyn.size());
dynInfo.pDynamicStates    = dyn.data();

// 그래픽 파이프라인 createInfo에 연결
graphicsPipelineCI.pDynamicState = &dynInfo;

```

### 초기 사용 (디폴트 값)

```cpp

VkPipelineRasterizationStateCreateInfo rast{};
rast.sType       = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
rast.polygonMode = VK_POLYGON_MODE_FILL;   // 기본값 (FILL / LINE / POINT)
rast.cullMode    = VK_CULL_MODE_BACK_BIT;
rast.frontFace   = VK_FRONT_FACE_CLOCKWISE;
rast.lineWidth   = 1.0f;

VkDynamicState dynStates[] = {
    VK_DYNAMIC_STATE_POLYGON_MODE_EXT
};

VkPipelineDynamicStateCreateInfo dynInfo{};
dynInfo.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
dynInfo.dynamicStateCount = 1;
dynInfo.pDynamicStates    = dynStates;

VkGraphicsPipelineCreateInfo pipeCI{};
pipeCI.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
pipeCI.pRasterizationState = &rast;     // 여기 기본값 들어감
pipeCI.pDynamicState       = &dynInfo;  // 여기서 dynamic 활성화

// ... etc
// on runtime
vkCmdSetPolygonModeEXT(cmd, VK_POLYGON_MODE_LINE); // 라인 모드로 변경

```

### 사용

* 다이내믹 상태 명령은 파이프라인 없이도 기록 가능
* 적용 시점은 드로우 때 실제 반영은 드로우 시점에, 바인드된 파이프라인이 해당 상태를 다이내믹으로 선언했을 때만 유효
* 상태는 유지됨(sticky) 한 번 세팅하면 이후 파이프라인에도 그대로 남아 있습니다. 정적 상태만 쓰는 파이프라인이면 무시
* 헷갈리지 않으려면 보통 vkCmdBindPipeline 직후에 필요한 vkCmdSet* 호출을 붙여주는 패턴 사용