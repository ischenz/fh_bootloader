/*
 * @Author: ischen.x ischen.x@foxmail.com
 * @Date: 2026-08-21 15:48:10
 * @LastEditors: ischen.x ischen.x@foxmail.com
 * @LastEditTime: 2026-11-27 13:34:03
 * 
 * Copyright (c) 2026 by fhchengz, All Rights Reserved. 
 */
#ifndef __FH_BOOTLOADER_H__
#define __FH_BOOTLOADER_H__

#include <stdint.h>
#include <stddef.h>

#define FH_BL_VERSION 0x00010000 // 版本号1.0.0，格式为0x00MMmmpp，MM主版本号，mm次版本号，pp补丁版本号
#define FH_BL_INFO_MAGIC (0x5A5A5A5A)
#define FH_BL_APP_ADDR  (0x08010000) // app存放地址
#define FH_BL_INFO_ADDR (0x0800C000) // bootloader信息存放地址
#define FH_BL_INFO_SECTOR FLASH_SECTOR_3 // bootloader信息存放扇区
#define FH_BL_APP_SECTOR  FLASH_SECTOR_4 // app存放扇区
#define FH_BL_APP_SECTOR_NUM 4

typedef enum { // 开机 或者升级
    FH_BL_UPGRADE_TYPE_NONE = 0, // 正常启动
    FH_BL_UPGRADE_TYPE_KEY,      // 按键触发升级
    FH_BL_UPGRADE_TYPE_FLAG,     // 升级标志触发升级
    FH_BL_UPGRADE_TYPE_SD,       // SD卡固件文件触发升级
} fh_bl_upgrade_type_e;

typedef struct
{
    uint32_t magic;              // 标识 (固定值，如 0x5A5A5A5A)

    /* 基础信息 */
    uint32_t boot_version;       // Bootloader版本
    uint32_t app_version;        // 应用版本
    uint32_t app_build_time;      // 应用编译时间戳
    uint32_t app_size;           // 应用大小
    uint32_t app_addr;           // 应用地址

    /* 升级控制 */
    uint32_t upgrade_flag;       // 升级标志
    uint32_t upgrade_type;       // 升级方式

    /* 预留扩展 */
    uint32_t reserved[8];

    /* 校验 */
    uint32_t app_crc;            // 固件CRC
    uint32_t info_crc;           // 本结构CRC

} fh_bl_info_t;

void fh_bl_boot(void);
int fh_bl_info_read(fh_bl_info_t *info);
int fh_bl_info_write(fh_bl_info_t *info);

// 用户自定义升级检测回调: 返回非FH_BL_UPGRADE_TYPE_NONE表示进入对应升级流程
typedef fh_bl_upgrade_type_e (*fh_bl_upgrade_check_cb_t)(fh_bl_info_t *info);
// 用户自定义升级执行回调: 数据源由回调自己负责(如SD卡), 返回0表示升级成功
typedef int (*fh_bl_upgrade_exec_cb_t)(size_t *app_size);

// 注册自定义升级源, 传NULL表示不注册; 未注册时SD类型会回退到默认UART升级
void fh_bl_register_upgrade(fh_bl_upgrade_check_cb_t check_cb, fh_bl_upgrade_exec_cb_t exec_cb);

// 固件写入流水线, 供外部升级源(如SD卡)复用: 内部缓冲并按4字节对齐写Flash
int fh_bl_firmware_write(uint8_t *data, uint16_t len);
int fh_bl_firmware_finish(size_t *app_size); // 写入剩余数据并计算APP大小
int fh_bl_clear_app(void);                   // 擦除APP区域FLASH

#endif /* __FH_BOOTLOADER_H__ */