/*
 * @Author: ischen.x ischen.x@foxmail.com
 * @Date: 2026-08-21 17:59:22
 * @LastEditors: ischen.x ischen.x@foxmail.com
 * @LastEditTime: 2026-08-21 18:06:20
 * 
 * Copyright (c) 2026 by fhchengz, All Rights Reserved. 
 */
#include "fh_bootloader.h"

int set_upgrade_flag(int flag)
{
    fh_bl_info_t info;
    fh_bl_info_read(&info);
    info.upgrade_flag = 1;
    fh_bl_info_write(&info);
    return 0;
}