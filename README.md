# fh_bootloader

STM32F407 系列串口升级 Bootloader，基于 STM32 HAL 库。支持通过 UART 接收固件、写入内部 Flash、CRC 校验后跳转运行 APP。

## 功能特性

- 串口（UART2）接收固件并写入内部 Flash
- 升级方式：按键触发 / 升级标志触发 / 正常启动
- 固件与 Bootloader 信息区均带 CRC32 校验
- 启动时打印版本、APP 信息（Hello 信息）
- 升级完成后自动跳转 APP（校验栈顶与 Reset Handler）

## Flash 布局

| 区域        | 地址              | 扇区       | 说明                          |
| ----------- | ----------------- | ---------- | ----------------------------- |
| Bootloader  | `0x08000000`      | Sector 0~2 | Bootloader 程序本体           |
| Info 信息区 | `0x0800C000`      | Sector 3   | 升级标志、版本、CRC 等信息    |
| APP 固件区  | `0x08010000` 起   | Sector 4~7 | APP 程序，最大 448 KB         |

> 地址定义见 `fh_bootloader.h`（`FH_BL_INFO_ADDR` / `FH_BL_APP_ADDR`）。

## 文件结构

| 文件                  | 说明                                    |
| --------------------- | --------------------------------------- |
| `fh_bootloader.h`     | 接口与信息区结构体定义                  |
| `fh_bootloader.c`     | Bootloader 主体：升级流程、Flash 读写、跳转 APP |
| `fh_bootloader_app.c` | APP 侧接口：设置升级标志                |

## 依赖

- [fh_stream](https://github.com/ischenz/fh_stream) — 帧封装/解析（升级数据传输协议）
- fh_sw_crc — 软件 CRC32
- ringbuff — 串口接收环形缓冲区
- STM32 HAL 库（UART、FLASH）

## 升级流程

1. 上电后调用 `fh_bl_boot()`，读取 Info 区并打印 Hello 信息
2. 检测升级条件：按键按下 或 Info 区 `upgrade_flag == 1` 则进入升级模式，否则直接跳转 APP
3. 升级模式：擦除 APP 区，通过 UART 按包接收固件，逐包写 Flash 并回 ACK
4. 接收完成后对 APP 区做 CRC32 校验，校验通过则清除升级标志、记录 APP 大小并跳转；失败则擦除 APP 区复位系统
5. 正常启动：校验栈顶指针与 Reset Handler 合法性后跳转 APP

### 传输协议

基于 fh_stream 帧格式，每包数据 = `pack_id`（4 字节）+ 固件数据：

| 帧 Tag | 方向      | 内容                          | 说明                     |
| ------ | --------- | ----------------------------- | ------------------------ |
| DATA   | 上位机→板 | `pack_id` + 固件数据          | 固件数据包，从 0 递增    |
| ACK    | 板→上位机 | `pack_id`                     | 应答；升级结果（0 成功） |
| CMD    | 上位机→板 | 固件整体 CRC32                | 传输结束，触发校验       |

## 使用方法

### Bootloader 侧

1. 集成依赖模块（fh_stream、fh_sw_crc、ringbuff）
2. 将 UART2 接收到的字节写入 `Uart2_RingBuff`（RX 中断中调用 ringbuff 写入接口）
3. 在 `main()` 中调用：

```c
#include "fh_bootloader.h"

int main(void)
{
    /* 初始化硬件与 UART2 后 */
    fh_bl_boot();   // 不会返回，正常启动时直接跳转 APP
}
```

### APP 侧

APP 程序需链接到 `0x08010000`（修改链接脚本 FLASH 起始地址），并可调用 `set_upgrade_flag()` 主动请求进入升级模式：

```c
#include "fh_bootloader.h"

/* 请求下次上电进入升级模式 */
set_upgrade_flag(1);
```
