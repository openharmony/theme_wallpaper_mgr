# AGENTS.md

本文件是 AI Agent 处理本仓库任务的轻量入口。先读本文件，再按任务类型加载匹配文档。

## 阅读策略

默认只读本文件。涉及代码开发时按任务类型加载专题文档（见知识路由）。

## 仓库定位

`theme_wallpaper_mgr` 是 OpenHarmony 主题框架子系统的壁纸管理服务组件。

```text
//base/theme/wallpaper_mgr
```

- 子系统：`theme`
- 部件：`wallpaper_mgr`
- Bundle：`@ohos/wallpaper_mgr`

## 核心能力

| 能力 | 说明 | 关键实现 |
| --- | --- | --- |
| 壁纸设置 | 支持图片/PixelMap/视频壁纸设置，通过 fd 或 PixelMap 传递数据 | `WallpaperService::SetWallpaper` |
| 壁纸获取 | 获取壁纸 ID、颜色、文件描述符、PixelMap 等 | `WallpaperService::GetPixelMap`, `GetColors` |
| 壁纸监听 | 支持颜色变化和壁纸变化事件回调 | `NapiWallpaperAbility`, `WallpaperEventListener` |
| 多用户支持 | 基于 userId 管理不同用户的壁纸数据 | `WallpaperService::OnInitUser`, `OnSwitchedUser` |
| Extension 能力 | 支持壁纸扩展能力连接与生命周期管理 | `WallpaperExtensionAbilityConnection` |

## 代码地图

| 目录 | 职责 |
| --- | --- |
| `frameworks/js/napi/` | NAPI 绑定：JS API 解析、类型转换、回调管理 |
| `frameworks/native/` | Native 客户端：WallpaperManager、IPC 代理 |
| `frameworks/kits/extension/` | Extension 扩展能力 |
| `services/` | 服务端实现：WallpaperService、壁纸文件管理、用户数据管理 |
| `utils/` | 公共工具：日志、文件处理、内存保护、DFX 上报 |
| `test/` | 单元测试和模糊测试 |

## 知识路由

| 任务类型 | 读取文档 |
| --- | --- |
| NAPI 层实现、JS API | `frameworks/js/napi/napi_wallpaper_ability.cpp` |
| 服务端核心逻辑、壁纸存储 | `services/src/wallpaper_service.cpp` |
| 错误码定义 | `utils/include/wallpaper_common.h` |
| IPC 接口定义 | `frameworks/native/IWallpaperService.idl` |

## 影响面分析

改动前需确认：
- 影响哪个 API 面：NAPI/InnerKit/服务端
- 是否需要处理多用户场景
- 涉及壁纸类型：WALLPAPER_SYSTEM / WALLPAPER_LOCK
- 权限检查：SET_WALLPAPER / GET_WALLPAPER
- 错误码是否符合 `ErrorCode` 枚举定义

## 依赖方向

保持向下流动：NAPI → Native Client → Service → Utils

- NAPI：解析 JS 参数、转换类型、管理回调
- Native Client：封装 IPC 调用、提供同步/异步接口
- Service：处理壁纸文件、管理用户数据、发送事件
- Utils：日志打印、文件操作、DFX 上报

## 已知陷阱

- 壁纸文件操作需正确关闭 fd，避免资源泄漏
- 多用户场景需使用 currentUserId_ 和 userId 参数
- IPC 调用需处理服务不可用（E_SA_DIED）

## 构建验证

> Linux 环境。Windows 仅文本验证。

| 目标 | 命令 |r
| --- | --- |
| 构建组件 | `./build.sh --product-name <product> --build-target wallpaper_mgr` |
| 构建测试 | `./build.sh --product-name <product> --build-target wallpaper_mgr_test` |