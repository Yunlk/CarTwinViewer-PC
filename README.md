# CarTwinViewer-PC

`CarTwinViewer-PC` 是迷你小车数字孪生演示项目的 PC 上位机。

本项目是对原始 `CarTwinViewer` 上位机的重构版本：原项目主要使用 EasyX 和偏 C 风格的组织方式；当前版本改为 SDL2 / SDL2_image + C++17，并重新整理了窗口渲染、资源加载、串口通信和仪表盘显示。

配套 STM32 固件仓库：`MiniCar-STM32`。

## 当前功能

- 显示道路、主车、障碍车和右侧驾驶仪表盘。
- 自动扫描串口，支持 USB-TTL / 常见 USB 串口设备热插拔。
- PC 周期性向 STM32 发送障碍车坐标。
- STM32 回传主车坐标后，PC 平滑动画同步更新红色主车位置和车道。
- 双击本地调试按键 `1`/`2`/`3` 也同样享受平滑动画。
- 仪表盘显示 MCU 心跳、串口收发帧计数、估算电机转速、碰撞次数、MCU 供电电压估算值和串口状态。
- 支持自动驾驶模式开关：点击仪表盘开关或按 `M`，PC 会向 STM32 发送模式切换命令。自动驾驶开启时若串口断开，MODE 指示灯和按钮变为红色。

## 目录结构

```text
assets/        图像资源
src/           C++ 源码
sdl2/          随项目携带的 SDL2 / SDL2_image MinGW 库
build.ps1      PowerShell 一键构建脚本
CMakeLists.txt CMake 构建配置
```

## 环境要求

- Windows
- MinGW g++，推荐路径：`D:\mingw64\bin\g++.exe`
- PowerShell
- 项目自带 `sdl2/` 目录，不需要单独安装 SDL2

如果 `D:\mingw64\bin\g++.exe` 不存在，`build.ps1` 会尝试使用 PATH 中的 `g++.exe`。

## 编译

推荐使用项目自带脚本：

```powershell
cd D:\git\CarTwinViewer-PC
powershell -ExecutionPolicy Bypass -File .\build.ps1
```

构建成功后会生成：

```text
build\CarTwinViewer.exe
build\SDL2.dll
build\SDL2_image.dll
build\assets\
```

也可以使用 CMake：

```powershell
cd D:\git\CarTwinViewer-PC
cmake -S . -B build-cmake -G "MinGW Makefiles"
cmake --build build-cmake
```

## 运行

```powershell
cd D:\git\CarTwinViewer-PC
.\build\CarTwinViewer.exe
```

程序启动后会自动扫描串口。推荐使用 USB-TTL 连接 STM32 的 USART2：

```text
USB-TTL TX -> STM32 PA3 RX
USB-TTL RX -> STM32 PA2 TX
USB-TTL GND -> STM32 GND
```

串口参数：

```text
9600 baud, 8 data bits, 1 stop bit, no parity
```

注意：TX/RX 要交叉连接，GND 必须共地。不要把 5V 直接接到 STM32 的串口 IO。

## 操作

- `M`：切换自动/手动模式，并向 STM32 发送模式命令。
- 鼠标点击仪表盘中的 `MODE` 开关：同上。
- `1` / `2` / `3`：本地调试用，切换 PC 画面中主车车道。
- `Esc`：退出程序。

手动驾驶的真实控制以 STM32 小车按键为准。STM32 按下左右转按键后，固件会更新主车坐标并回传给 PC，PC 根据回传坐标同步显示。

## 串口协议

PC 发送障碍车坐标：

```text
%xxx\0yyy$
```

其中 `xxx` 是障碍车 X 坐标，`yyy` 是障碍车 Y 坐标。

PC 发送模式命令：

```text
%MODE=0$  自动驾驶
%MODE=1$  手动驾驶
```

STM32 回传 telemetry：

```text
%X=10,Y=420,HB=1,RX=10,TX=20,RL=2100,RR=2100,V=3300,ST=7,PC=1,AGE=3$
```

字段说明：

| 字段 | 含义 |
| --- | --- |
| `X` / `Y` | STM32 当前主车坐标 |
| `HB` | STM32 心跳计数 |
| `RX` / `TX` | STM32 侧接收/发送帧计数 |
| `RL` / `RR` | 左/右电机估算转速 |
| `V` | STM32 自身 VDDA/3.3V 供电估算值，单位 mV |
| `ST` | MCU 状态位，`0x04` 表示自动驾驶开启 |
| `PC` | STM32 认为 PC 链路是否在线 |
| `AGE` | 距离上次收到 PC 帧的周期数 |

## 常见问题

- 程序显示 `SCAN`：还没有找到可用串口，检查 USB-TTL 驱动、线序和端口占用。
- 有 TX 没有 RX：PC 正在发帧，但没有收到 STM32 回传，检查 STM32 是否烧录最新固件、PA2/PA3 是否接反、GND 是否共地。
- 自动驾驶状态不更新：确认 STM32 telemetry 中 `ST` 是否包含 `0x04`，以及 STM32 模式按键是否真的触发。
- 电压显示为 `0`：STM32 ADC 读取超时或内部参考电压读取失败；这不代表电池电压为 0。

## 配套项目

- STM32 固件：`MiniCar-STM32`
