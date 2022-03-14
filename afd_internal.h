/*
 *
 * Copyright (c) 2015 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * tbff_internal related tb_module defines
 *
 */

#ifndef AFD_INTERNAL_H_
#define AFD_INTERNAL_H_

#include <linux/amlogic/media/video_sink/vpp_afd.h>
//#include "vpp_afd.h"

struct afd_handle_s {
    bool afd_enable;
    unsigned int cur_afd_value;
    unsigned int cur_video_w;
    unsigned int cur_video_h;
    unsigned int cur_screen_w;
    unsigned int cur_screen_h;
    struct ar_fraction_s cur_video_ar;
    struct ar_fraction_s cur_screen_ar;
    struct crop_rect_s cur_crop_info;
    struct pos_rect_s cur_dst_info;
    struct pos_rect_s cur_disp_info;
    // unsigned char ud_buf[UD_SIZE];
    // unsigned int ud_buf_used;
    void *priv_data;
};

struct afd_ctl {
    char name[10];
    char val[128];
};

#define AFD_IOC_MAGIC 'F'
#define AFD_IOCTL_CMD_SET _IOW(AFD_IOC_MAGIC, 0x00, struct afd_ctl)
#define AFD_IOCTL_CMD_GET _IOWR(AFD_IOC_MAGIC, 0x01, struct afd_ctl)
#endif
