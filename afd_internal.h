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

struct afd_ctl_create_t {
    int path;
    int instance_id;
};

struct afd_rect_t {
    int a;
    int b;
    int c;
    int d;
};

struct afd_scaling_t {
    unsigned int type;
    struct afd_rect_t scaling_rect;
    int resolution_width;
    int resolution_height;
};

struct afd_ctl_scaling_t {
    int path;
    struct afd_scaling_t scaling;
};

struct afd_recv_list_t {
    int list[10];
    unsigned char size;
};

struct afd_ctl_overscan_t {
    struct afd_rect_t uhd_overscan;
    struct afd_rect_t fhd_overscan;
    struct afd_rect_t hd_overscan;
    struct afd_rect_t sd_overscan;
};

struct afd_recv_state_t {
    unsigned int valid;
    int path;
    int instance_id;
    unsigned char enable;
    int aspect;
    int video_aspect;
    unsigned int afd_value;
    struct afd_scaling_t scaling;
    unsigned int screen_width;
    unsigned int screen_height;
    unsigned int video_width;
    unsigned int video_height;
    struct afd_rect_t video_out;
    struct afd_rect_t display_out;
};

#define AFD_IOC_MAGIC 'F'
#define AFD_IOCTl_CREATE_CONTEXT  _IOW(AFD_IOC_MAGIC, 0x00, struct afd_ctl_create_t)
#define AFD_IOCTL_RELEASE_CONTEXT _IO(AFD_IOC_MAGIC, 0x01)
#define AFD_IOCTL_SET_ASPECT      _IO(AFD_IOC_MAGIC, 0x02)
#define AFD_IOCTL_SET_SCALE_TYPE  _IOW(AFD_IOC_MAGIC, 0x03, struct afd_ctl_scaling_t)
#define AFD_IOCTL_GET_PATHS       _IOR(AFD_IOC_MAGIC, 0x04, struct afd_recv_list_t)
#define AFD_IOCTL_SET_OVERSCAN    _IOW(AFD_IOC_MAGIC, 0x05, struct afd_ctl_overscan_t)
#define AFD_IOCTL_GET_STATE       _IOR(AFD_IOC_MAGIC, 0x06, struct afd_recv_state_t)


#endif
