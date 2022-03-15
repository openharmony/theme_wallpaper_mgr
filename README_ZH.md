# 杂散子系统/壁纸服务

## 简介
1、为系统提供壁纸服务能力，支持系统显示、设置、切换壁纸等功能；
2、为开发者提供开发壁纸的框架和接口，支持开发者开发壁纸应用；

**图 1** 子系统架构图  
![](figures/subsystem_architecture_zh.png "子系统架构图")

#### 仓路径

/base/miscservices/wallpaper

## 目录

#### 框架代码介绍

```
/base/miscservices/wallpaper
├── figures                  # 构架图
├── frameworks/innerkitsimpl      # 对应用提供的接口
├── interfaces/kits          # 组件对外提供的接口代码
│   ├── jskits               # 服务间接口
│   └── napi                 # js接口解析成napi接口
├── profile               # 组件包含的系统服务的配置文件和进程的配置文件
├── services                 # 壁纸管理服务实现
├── test                     # 接口的单元测试
└── utils                    # 组件包含日志打印和有序公共事件定义的常量
```
## 说明

#### 接口说明
**表 1**  壁纸服务开放的主要方法

<a name="table033515471012"></a>
<table><thead align="left"><tr id="row143351854201012"><th class="cellrowborder" valign="top" width="50%" id="mcps1.2.3.1.1"><p id="p103351154121010"><a name="p103351154121010"></a><a name="p103351154121010"></a>接口名</p>
</th>
<th class="cellrowborder" valign="top" width="50%" id="mcps1.2.3.1.2"><p id="p1033585416105"><a name="p1033585416105"></a><a name="p1033585416105"></a>描述</p>
</th>
</tr>
</thead>
<tbody><tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>function getColors(wallpaperType: WallpaperType): Promise&lt;Array&lt;RgbaColor&gt;&gt;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>获取壁纸图片主颜色（桌面或者锁屏壁纸），Promise方式</p>
</td>
</tr>
<tr id="row13335054111018"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p12832214151418"><a name="p12832214151418"></a><a name="p12832214151418"></a>function getId(wallpaperType: WallpaperType): Promise&lt;number&gt;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p3335145451011"><a name="p3335145451011"></a><a name="p3335145451011"></a>获取壁纸id（桌面或者锁屏壁纸），Promise方式</p>
</td>
</tr>
<tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>function getPixelMap(wallpaperType: WallpaperType): Promise&lt;image.PixelMap&gt;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>获取壁纸图片的pixelmap（桌面或者锁屏壁纸），Promise方式</p>
</td>
</tr>
<tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>function setWallpaper(source: string | image.PixelMap, wallpaperType: WallpaperType): Promise&lt;void&gt;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>设置壁纸（图片路径或pixelmap），Promise方式</p>
</td>
</tr>
<tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>function on(type: 'colorChange', callback: (colors: Array&lt;RgbaColor&gt;, wallpaperType: WallpaperType) => void): void;
</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>监听壁纸图片主颜色变化，callback方式</p>
</td>
</tr>
<tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>function off(type: 'colorChange', callback?: (colors: Array&lt;RgbaColor&gt;, wallpaperType: WallpaperType) => void): void;
</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>取消监听壁纸图片主颜色变化，callback方式</p>
</td>
</tr>
</tbody>
</table>

js 接口使用说明
```
//获取壁纸callback方式
wallpaper.getPixelMap(WALLPAPER_SYSTEM, function (err, data) {
            console.info('wallpaperXTS ===> testGetPixelMapCallbackSystem err : ' + JSON.stringify(err));
            console.info('wallpaperXTS ===> testGetPixelMapCallbackSystem data : ' + JSON.stringify(data));
        })

//获取壁纸Promise方式
wallpaper.getPixelMap(WALLPAPER_SYSTEM).then((data) => {
            console.info('wallpaperXTS ===> testGetPixelMapPromiseSystem data : ' + data);
            console.info('wallpaperXTS ===> testGetPixelMapPromiseSystem data : ' + JSON.stringify(data));

        }).catch((err) => {
            console.info('wallpaperXTS ===> testGetPixelMapPromiseSystem err : ' + err);
            console.info('wallpaperXTS ===> testGetPixelMapPromiseSystem err : ' + JSON.stringify(err));

        });

//设置壁纸callback方式
wallpaper.setWallpaper(pixelmap, WALLPAPER_SYSTEM, function (err, data) {
                console.info('wallpaperXTS ===> testSetWallpaperPixelMapCallbackSystem err : ' + JSON.stringify(err));
                console.info('wallpaperXTS ===> testSetWallpaperPixelMapCallbackSystem data : ' + JSON.stringify(data));
            });

//设置壁纸Promise方式
wallpaper.setWallpaper(pixelmap, WALLPAPER_SYSTEM).then((data) => {
                console.info('wallpaperXTS ===> testSetWallpaperPixelMapPromiseSystem data : ' + JSON.stringify(data));

            }).catch((err) => {
                console.info('wallpaperXTS ===> testSetWallpaperPixelMapPromiseSystem err : ' + JSON.stringify(err));

            });
```

#### 应用接口使用说明
**表 2**  应用extension的主要接口

<a name="table033515471012"></a>
<table><thead align="left"><tr id="row143351854201012"><th class="cellrowborder" valign="top" width="50%" id="mcps1.2.3.1.1"><p id="p103351154121010"><a name="p103351154121010"></a><a name="p103351154121010"></a>接口名</p>
</th>
<th class="cellrowborder" valign="top" width="50%" id="mcps1.2.3.1.2"><p id="p1033585416105"><a name="p1033585416105"></a><a name="p1033585416105"></a>描述</p>
</th>
</tr>
</thead>
<tbody><tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>onCreated(want: object): void</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>wallpaper extension 初始化的回调</p>
</td>
</tr>
<tr id="row13335054111018"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p12832214151418"><a name="p12832214151418"></a><a name="p12832214151418"></a>onWallpaperChanged(wallpaperType: number): void</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p3335145451011"><a name="p3335145451011"></a><a name="p3335145451011"></a>壁纸发生了变化的回调</p>
</td>
</tr>
<tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>setUiContent(url:string): void</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>应用设置壁纸的布局文件路径</p>
</td>
</tr>
<tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>onDestroy(): void
</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>wallpaper extension 销毁回调</p>
</td>
</tr>
</tbody>
</table>

js 应用接口使用说明
```
应用需要继承wallpaperextension，实现里面的接口，其中setUiContent不需要重新

onCreated(want) {
    // 回调部分， 如静态壁纸先获取壁纸图片
    wallpaper.getPixelMap(WALLPAPER_SYSTEM).then((data) => {
            // 使用data更新布局数据
            console.info('wallpaperXTS ===> testGetPixelMapPromiseSystem data : ' + data);
            console.info('wallpaperXTS ===> testGetPixelMapPromiseSystem data : ' + JSON.stringify(data));

        }).catch((err) => {
            console.info('wallpaperXTS ===> testGetPixelMapPromiseSystem err : ' + err);
            console.info('wallpaperXTS ===> testGetPixelMapPromiseSystem err : ' + JSON.stringify(err));

        });

    //加载页面布局
    wallpaper.setUiContent(url);

}
```

## 相关仓
Misc软件服务子系统

#### 参与贡献

1.  Fork 本仓库
2.  新建 Feat_xxx 分支
3.  提交代码
4.  新建 Pull Request

