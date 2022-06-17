// Copyright (C) 2015 Amlogic, Inc. All rights reserved.
//
// All information contained herein is Amlogic confidential.
//
// This software is provided to you pursuant to Software License
// Agreement (SLA) with Amlogic Inc ("Amlogic"). This software may be
// used only in accordance with the terms of this agreement.
//
// Redistribution and use in source and binary forms, with or without
// modification is strictly prohibited without prior written permission
// from Amlogic.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


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
