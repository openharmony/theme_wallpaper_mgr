# AGENTS.md

本文件是 AI Agent 处理本仓库任务的轻量入口。先读本文件，再按任务类型加载匹配文档。

## 阅读策略

默认只读本文件。涉及代码开发时按任务类型加载专题文档（见知识路由）。

编辑前必须声明：**任务类别、已读文档、发现的约束**。未声明则不得开始编辑。

## 仓库定位

`theme_wallpaper_mgr` 是 OpenHarmony 主题框架子系统的壁纸管理服务组件。

```text
//base/theme/wallpaper_mgr
```

- 子系统（OpenHarmony 子系统，即功能分组）：`theme`
- 部件（OpenHarmony 部件，即独立编译/分发单元）：`wallpaper_mgr`
- Bundle（OpenHarmony 应用包标识）：`@ohos/wallpaper_mgr`
- SysCap（System Capability，OpenHarmony 系统能力声明）：`SystemCapability.MiscServices.Wallpaper`
- SA ID（System Ability ID，OpenHarmony 系统能力服务标识）：`3705`

## 核心能力

| 能力 | 说明 | 关键实现 | 常见任务 |
| --- | --- | --- | --- |
| 壁纸设置 | 支持图片/PixelMap/视频壁纸设置，通过 fd 或 PixelMap 传递数据 | `WallpaperService::SetWallpaper` | 新增壁纸类型、修改设置逻辑 |
| 壁纸获取 | 获取壁纸 ID、颜色、文件描述符、PixelMap 等 | `WallpaperService::GetPixelMap`, `GetColors` | 新增获取接口、返回值变更 |
| 壁纸监听 | 支持颜色变化和壁纸变化事件回调 | `NapiWallpaperAbility`, `WallpaperEventListener` | 新增事件类型、回调参数变更 |
| 多用户支持 | 基于 userId 管理不同用户的壁纸数据 | `WallpaperService::OnInitUser`, `OnSwitchedUser` | 多用户隔离修复、用户切换逻辑 |
| Extension 能力 | 支持壁纸扩展能力连接与生命周期管理 | `WallpaperExtensionAbilityConnection` | Extension 生命周期、连接管理 |

## 代码地图

| 目录 | 职责 | 常见任务 | 高频修改文件 |
| --- | --- | --- | --- |
| `frameworks/js/napi/` | NAPI 绑定（OpenHarmony Node-API 桥接层，非 Node.js N-API）：JS API 解析、类型转换、回调管理 | JS API 新增/修改 | `napi_wallpaper_ability.cpp` |
| `frameworks/native/` | Native 客户端：WallpaperManager、IPC 代理、IDL 生成代码 | 客户端接口新增、IPC 代理 | `wallpaper_manager.cpp`, `IWallpaperService.idl` |
| `frameworks/ets/taihe/` | Taihe/ArkTS 跨语言桥接（ANI 为 ArkTS Native Interface，ABC 为 ArkTS ByteCode，均由 Taihe 工具链生成） | ArkTS API 桥接 | `ohos.wallpaper.taihe` |
| `frameworks/kits/extension/` | Extension 扩展能力 | Extension 生命周期、Stub | `wallpaper_extension_ability_stub.h` |
| `services/` | 服务端实现：WallpaperService、壁纸文件管理、用户数据管理 | 服务端核心逻辑、权限校验 | `wallpaper_service.cpp`, `wallpaper_service_cb_proxy.cpp` |
| `services/etc/init/` | 服务进程配置（rc/cfg） | 进程权限、SELinux 标签 | `wallpaperservice.cfg` |
| `services/profile/` | SA Profile（3705.json） | SA 注册配置 | `3705.json` |
| `utils/` | 公共工具：日志、文件处理、内存保护、DFX 上报 | 错误码、权限常量、工具函数 | `wallpaper_common.h`, `wallpaper_manager_common_info.h`, `memory_guard.h` |
| `utils/dfx/` | DFX 适配：HiSysEvent 故障上报、HiDumper 运维诊断 | DFX 事件定义、故障上报 | `fault_reporter.h`, `hisysevent.yaml` |
| `interfaces/inner_api/` | InnerKit 头文件（`wallpaper_manager_client.h`） | InnerKit API 变更 | `wallpaper_manager_client.h` |
| `test/` | 单元测试（gtest）和模糊测试（libFuzzer） | 测试用例补充 | `wallpaper_test.cpp`, `wallpaper_permission_test.cpp` |
| `skill/` | Agent 技能定义（质量审查技能等） | Agent 指令质量审查 | `SKILL.md` |

> **嵌套指引**：本仓库无子目录级 `AGENTS.md` 或 `CLAUDE.md`。`skill/` 目录下有 Agent 指令质量审查技能，用于审查本文件及同类指令文件的质量。

## 知识路由

### 任务路由

| 任务类型 | 读取文档 |
| --- | --- |
| NAPI 层实现、JS API | `frameworks/js/napi/napi_wallpaper_ability.cpp` |
| 服务端核心逻辑、壁纸存储 | `services/src/wallpaper_service.cpp` |
| 错误码定义 | `utils/include/wallpaper_common.h` |
| IPC 接口定义 | `frameworks/native/IWallpaperService.idl` |
| 权限校验逻辑 | `services/src/wallpaper_service.cpp`（`CheckCallingPermission`、`IsSystemApp`、`IsNativeSa`） |
| DFX 故障上报 | `utils/dfx/hisysevent_adapter/fault_reporter.h`、`hisysevent.yaml` |
| 多用户数据隔离 | `services/src/wallpaper_service.cpp`（`CheckUserPermissionById`、`currentUserId_`） |
| InnerKit API | `interfaces/inner_api/include/wallpaper_manager_client.h` |
| Extension 生命周期 | `frameworks/kits/extension/` 全目录、`services/src/wallpaper_extension_ability_connection.cpp` |
| Taihe/ArkTS 桥接 | `frameworks/ets/taihe/wallpapermgr/BUILD.gn`、`frameworks/ets/taihe/wallpapermgr/idl/ohos.wallpaper.taihe` |

### 路径路由

| 修改目录 | 必读文档 |
| --- | --- |
| `frameworks/js/napi/` | `napi_wallpaper_ability.cpp`、`wallpaper_common.h`（错误码）、`IWallpaperService.idl`（接口契约） |
| `frameworks/native/` | `IWallpaperService.idl`、`wallpaper_manager.cpp`、`wallpaper_common.h` |
| `services/` | `wallpaper_service.cpp`、`wallpaper_common.h`、`wallpaperservice.cfg`（进程权限） |
| `services/etc/init/` | `wallpaperservice.cfg`（进程权限、SELinux 标签） |
| `services/profile/` | `3705.json`（SA 注册配置） |
| `utils/` | `wallpaper_common.h`（错误码/权限常量）、`hisysevent.yaml`（DFX 事件定义） |
| `utils/dfx/` | `fault_reporter.h`（故障上报）、`hisysevent.yaml`（仓库根目录，事件定义） |
| `frameworks/ets/taihe/` | `frameworks/ets/taihe/wallpapermgr/idl/ohos.wallpaper.taihe`、`frameworks/ets/taihe/wallpapermgr/BUILD.gn` |
| `frameworks/kits/extension/` | `wallpaper_extension_ability_stub.h`、`wallpaper_extension_ability_connection.cpp` |
| `interfaces/inner_api/` | `wallpaper_manager_client.h`、`IWallpaperService.idl` |
| `test/` | 对应被测源文件、`wallpaper_common.h`（错误码断言） |

### 词汇路由

| 术语/关键词 | 含义 | 必读文档 |
| --- | --- | --- |
| `fd` / `file descriptor` | 壁纸数据通过 fd 传递，必须正确关闭 | `wallpaper_service.cpp`（fd 生命周期管理）、已知陷阱：fd 泄漏 |
| `PixelMap` | 图片像素数据，用于 SetWallpaperByPixelMap | `IWallpaperService.idl`（`WallpaperRawData`）、`napi_wallpaper_ability.cpp` |
| `Extension` | 壁纸扩展能力，连接生命周期管理 | `wallpaper_extension_ability_connection.cpp`、`wallpaper_extension_ability_stub.h` |
| `userId` / `currentUserId_` | 多用户隔离标识 | `wallpaper_service.cpp`（`CheckUserPermissionById`）、已知陷阱：多用户场景 |
| `WALLPAPER_SYSTEM` / `WALLPAPER_LOCKSCREEN` | 壁纸类型枚举（`WallpaperType`） | `wallpaper_manager_common_info.h`（定义处）、`wallpaper_common.h`（间接包含） |
| `E_SA_DIED` | 服务不可用错误码 | `wallpaper_common.h`、已知陷阱：IPC 调用处理 |
| `AccessToken` / `VerifyAccessToken` | OpenHarmony 权限校验框架 | `wallpaper_service.cpp`（`CheckCallingPermission`） |
| `CFI` / `PAC_RET` / `BTI` | 构建安全加固选项（BUILD.gn 中配置） | `BUILD.gn` 各目标 sanitize 配置 |
| `idl_gen_interface` | IDL 代码生成构建规则，生成 proxy/stub | `frameworks/native/BUILD.gn`、生成代码边界 |
| `Taihe` / `ANI` | ArkTS 跨语言桥接代码生成 | `frameworks/ets/taihe/wallpapermgr/BUILD.gn` |
| `FaultType` / `SET_WALLPAPER_FAULT` / `LOAD_WALLPAPER_FAULT` | DFX 故障类型枚举，`SERVICE_FAULT` 为服务故障，`SET_WALLPAPER_FAULT`/`LOAD_WALLPAPER_FAULT` 为业务故障 | `utils/dfx/hisysevent_adapter/dfx_types.h`（枚举定义）、`fault_reporter.h`（上报 API） |
| `FoldState` / `RotateState` | 折叠屏状态和旋转状态枚举，`WallpaperPictureInfo` 结构体依赖 | `wallpaper_manager_common_info.h`（定义处） |
| `WallpaperResourceType` | 壁纸资源类型枚举（`DEFAULT`/`PICTURE`/`VIDEO`/`PACKAGE`），`SetWallpaper` 内部方法使用 | `wallpaper_manager_common_info.h`（定义处）、`wallpaper_service.cpp`（使用处） |

## 影响面分析

改动前需确认：
- 影响哪个 API 面：NAPI/InnerKit/服务端
- 是否需要处理多用户场景
- 涉及壁纸类型：WALLPAPER_SYSTEM / WALLPAPER_LOCKSCREEN
- 权限检查：SET_WALLPAPER / GET_WALLPAPER / CAPTURE_SCREEN（注：`CAPTURE_SCREEN` 权限常量已定义但当前未在服务端使用）
- 错误码是否符合 `ErrorCode` 枚举定义（`wallpaper_common.h`）
- 是否影响 DFX 上报行为（`hisysevent.yaml` 中定义的 `SERVICE_FAULT`/`RUNTIME_FAULT`）

## 约束与边界

### Do-not（禁止）

- **不得修改公共 API 签名或语义**：`IWallpaperService.idl` 中定义的 23 个方法、`wallpaper_common.h` 中的 `ErrorCode` 枚举值、`WallpaperType` 枚举值，未经兼容性审查不得变更
- **不得删除或绕过权限检查**：`CheckCallingPermission`、`IsSystemApp`、`IsNativeSa`、`CheckUserPermissionById` 调用不得被移除或条件性跳过
- **不得删除 DFX 日志和故障上报**：`fault_reporter.h` 中的 `ReportServiceFault`/`ReportRuntimeFault` 上报调用不得移除；`dfx_types.h` 中的 `FaultType` 枚举值（`SERVICE_FAULT`/`SET_WALLPAPER_FAULT`/`LOAD_WALLPAPER_FAULT`）不得删除
- **不得修改生成代码**：`idl_gen_interface` 生成的 `*_proxy.cpp`/`*_stub.cpp`、Taihe 生成的 `ohos.wallpaper.ani.cpp`/`ohos.wallpaper.abi.c`，修改需改源文件（`.idl`/`.taihe`）后重新生成
- **不得变更壁纸文件存储格式**：持久化数据格式变更需跨版本兼容方案
- **不得在 NAPI 层直接调用服务端**：必须通过 Native Client（WallpaperManager）IPC 代理调用
- **不得引入未经合规审查的第三方依赖**：新增依赖需确认许可证和供应链安全
- **不得破坏 V9 接口兼容性**：`IWallpaperService.idl` 中 `SetWallpaperV9`/`GetPixelMapV9`/`GetColorsV9`/`ResetWallpaperV9`/`SetWallpaperV9ByPixelMap` 等 V9 方法必须保持签名和语义不变，V9 与当前接口并存，修改时需确认两端影响

### Ask-before（需确认）

- 修改 `IWallpaperService.idl`：需确认 API 版本兼容策略（V9 接口 vs 新接口）
- 修改 `wallpaperservice.cfg` 中的进程权限或 SELinux 标签
- 修改 `ErrorCode` 枚举：新增值需确认不影响已有错误码数值
- 修改 `3705.json` SA Profile 配置
- 修改 BUILD.gn 中的 sanitize 安全加固选项（CFI/PAC_RET/BTI/整数溢出等）
- 新增或修改权限常量（`wallpaper_common.h` 中的 `WALLPAPER_PERMISSION_NAME_*`）

### 依赖方向

保持向下流动：NAPI → Native Client → Service → Utils

- NAPI：解析 JS 参数、转换类型、管理回调
- Native Client：封装 IPC 调用、提供同步/异步接口
- Service：处理壁纸文件、管理用户数据、发送事件
- Utils：日志打印、文件操作、DFX 上报

### 生成代码边界

| 源文件 | 生成规则 | 生成产物 | 消费方 |
| --- | --- | --- | --- |
| `IWallpaperService.idl` | `idl_gen_interface`（`frameworks/native/BUILD.gn`） | `*_proxy.cpp`, `*_stub.cpp` | `wallpaper_service_proxy`（服务端）、`wallpapermanager`（客户端） |
| `ohos.wallpaper.taihe` | `ohos_taihe`（`frameworks/ets/taihe/wallpapermgr/BUILD.gn`） | `ohos.wallpaper.ani.cpp`, `ohos.wallpaper.abi.c` | ArkTS 桥接模块 |
| Taihe 生成 `.ets` | `generate_static_abc` | `wallpapermgr_abc`（字节码） | ArkTS 运行时 |

手写的 Proxy/Stub（非 IDL 生成，可修改）：
- `services/src/wallpaper_service_cb_proxy.cpp` / `services/include/wallpaper_service_cb_proxy.h`
- `services/src/wallpaper_event_listener_proxy.cpp` / `services/include/wallpaper_event_listener_proxy.h`
- `frameworks/native/src/wallpaper_event_listener_stub.cpp` / `frameworks/native/include/wallpaper_event_listener_stub.h`
- `frameworks/native/src/wallpaper_service_cb_stub.cpp` / `frameworks/native/include/wallpaper_service_cb_stub.h`

## 已知陷阱

- 壁纸文件操作需正确关闭 fd，避免资源泄漏
- 多用户场景需使用 currentUserId_ 和 userId 参数，`CheckUserPermissionById` 校验用户身份
- IPC 调用需处理服务不可用（E_SA_DIED）
- V9 接口与当前接口并存（如 `SetWallpaperV9`/`SetWallpaper`），修改时需确认影响范围
- `ConcurrentMap`（`utils/include/concurrent_map.h`）使用 `recursive_mutex`，修改并发逻辑需注意死锁风险
- `MemoryGuard`（`utils/include/memory_guard.h`）RAII 保护，确保作用域内内存安全
- `WallpaperPictureInfo` 结构体包含 `foldState`/`rotateState`，折叠屏/旋转场景需正确处理
- 壁纸设置操作会影响设备锁屏/主屏显示，测试时需注意回退
- 视频壁纸（`SetVideo`）内部委托 4 参数 `SetWallpaper()` 并传入 `VIDEO` 资源类型，公共 API 层关闭 fd（内部 4 参数方法不关闭 fd），需确保 fd 有效且 length 合法，设置失败不影响已有壁纸
- `ResetWallpaper` 会重置设备壁纸为默认，属于破坏性操作，测试时需确认回退方案
- 并发壁纸设置请求可能产生竞态，服务端通过 `ConcurrentMap` 管理状态，修改时需确认并发安全
- 部分 BUILD.gn 目标存在重复 `branch_protector_ret` 赋值（`pac_ret` 被 `bti` 覆盖），修改构建配置时需确认实际生效值

## 验证闭环

### 构建与测试

> Linux 环境。Windows 仅文本验证。

| 目标 | 命令 |
| --- | --- |
| 构建组件 | `./build.sh --product-name <product> --build-target wallpaper_mgr` |
| 构建测试 | `./build.sh --product-name <product> --build-target wallpaper_mgr_test` |
| 构建模糊测试 | `./build.sh --product-name <product> --build-target wallpaper_mgr_fuzztest` |
| 构建JS单元测试 | `./build.sh --product-name <product> --build-target wallpaper_js_test` |
> 注：具体静态分析工具和命令视产品构建环境而定，上述为参考命令。

### 任务特定验证

| 任务类型 | 最低验证要求 |
| --- | --- |
| NAPI 层修改 | 构建通过 + JS 单元测试通过 + 对应模糊测试通过 |
| 服务端修改 | 构建通过 + `wallpaper_test` 通过 + `wallpaper_permission_test` 通过 + 对应模糊测试通过 |
| 权限/安全修改 | 构建通过 + `wallpaper_permission_test` 通过 + `wallpaper_service_fuzzer` 通过 |
| DFX 修改 | 构建通过 + `wallpaper_dfx_test` 通过 + `hisysevent.yaml` 事件定义一致 |
| 错误码修改 | 构建通过 + 所有测试通过 + `ErrorCode` 枚举值无冲突 |
| IDL 修改 | 构建通过 + 确认生成代码正确编译 + 所有测试通过 |

### Done 定义

任务完成 = 构建通过 + 相关测试通过 + 无新增 lint/静态分析错误 + 影响面分析已确认。