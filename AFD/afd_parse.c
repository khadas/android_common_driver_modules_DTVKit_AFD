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


#include "afd_parse.h"

#include <linux/kernel.h>
#include <linux/string.h>

#define UD_SIZE (8 * 1024)

userdata_type checkFormat(struct userdata_param_t *ud, uint8_t *buf, int len) {
    userdata_type type = INVALID_TYPE;
    vformat_t vFormat = VFORMAT_UNKNOWN;

    if (!buf || len < 16) return type;

    vFormat = (vformat_t)((ud->meta_info.flags & 0x78) >> 3);
    switch (vFormat) {
        case VFORMAT_H264:
            if (IS_H264_AFD(buf)) type = H264_AFD_TYPE;
            break;
        case VFORMAT_MPEG12:
            if (IS_AFD(buf)) type = MPEG_AFD_TYPE;
            break;
        default:
            break;
    }
    return type;
}

uint8_t processData(uint8_t *rawData, afd_extra_info *extra, uint32_t debug) {
    uint8_t af = 0xFF;
    struct userdata_param_t *ud = (struct userdata_param_t *)rawData;
    uint8_t *pd = NULL;
    userdata_type uType;

    extra->inst_id = ud->instance_id;
    extra->pts = ud->meta_info.vpts;
    extra->poc = ud->meta_info.poc_number;
    extra->frame_type = (ud->meta_info.flags >> 12) & 0x03;
    extra->video_format = (ud->meta_info.flags & 0x78) >> 3;
    if (extra->video_format == VFORMAT_MPEG12) {
        extra->frame_type = (ud->meta_info.flags >> 7) & 0x03;
    } else {
        extra->frame_type = FRAME_TYPE_MPEG_NUM + ((ud->meta_info.flags >> 12) & 0x03);
    }

    if (debug)
        pr_err("[AFD] Got ud, addr:%p, size:%d", ud->pbuf_addr, ud->buf_len);

    if (debug)
        pr_err("[AFD] Meta info(poc:%u, flags:%u, pts:%u-%u)",
            ud->meta_info.poc_number, ud->meta_info.flags, ud->meta_info.vpts, ud->meta_info.vpts_valid);

    if (ud->meta_info.flags == 0 && ud->meta_info.vpts == 0 && ud->meta_info.vpts_valid == 0) {
        //invalid frame, skip it
        return 0xff;
    }
    if (ud->pbuf_addr == NULL || ud->buf_len <= 0 || ud->buf_len < 16) {
        //valid frame, none ud
        return 0;//default to 0 for auto
    }

    pd = ud->pbuf_addr;
    if (debug) {
        pr_err("[AFD] ud data: [0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x," \
                               "0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x]",
                               pd[0], pd[1], pd[2], pd[3], pd[4], pd[5], pd[6], pd[7],
                               pd[8], pd[9], pd[10], pd[11], pd[12], pd[13], pd[14], pd[15]);
        pr_err("[AFD] meta info flags: %u", ud->meta_info.flags);
    }
    uType = checkFormat(ud, pd, ud->buf_len);
    switch (uType) {
        case MPEG_AFD_TYPE: {
            af = (pd[10] & 0x0f);
            break;
        }
        case H264_AFD_TYPE: {
            af = (pd[15] & 0x0f);
            break;
        }
        case INVALID_TYPE:
        default: {
            break;
        }
    }

    return af;
}

uint8_t getAfdFromMetaInfo(uint8_t *rawData, afd_extra_info *extra, uint32_t debug) {
    uint8_t mAf = 0;
    uint8_t *pd = rawData;
    mAf = processData(pd, extra, debug);
    return mAf;
}
