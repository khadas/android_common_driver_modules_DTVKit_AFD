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
    userdata_type mType = INVALID_TYPE;

    vformat_t mFormat = (vformat_t)((ud->meta_info.flags & 0x78) >> 3);
    switch (mFormat) {
        case VFORMAT_H264:
            if (IS_H264_AFD(buf)) mType = H264_AFD_TYPE;
            break;
        case VFORMAT_MPEG12:
            if (len >= (int)sizeof(aml_ud_header_t)) {
                aml_ud_header_t *hdr = (aml_ud_header_t *)buf;
                if (IS_AFD(hdr->atsc_flag)) {
                    mType = MPEG_AFD_TYPE;
                }
            }
            break;
        default:
            break;
    }
    return mType;
}

uint8_t processMpegData(uint8_t *data, int len) {
    uint8_t *pd = data;
    int left = len;
    uint8_t AF = 0;
    if (left >= (int)sizeof(aml_ud_header_t)) {
        aml_ud_header_t *hdr = (aml_ud_header_t *)pd;
        uint8_t *pafd_hdr = (uint8_t *)hdr->cc_data_start;
        AM_USERDATA_AFD_t afd = *((AM_USERDATA_AFD_t *)(pafd_hdr));
        AF = afd.af;
    }
    return AF;
}
uint8_t processH264Data(uint8_t *data, int len) {
    uint8_t *pd = data;
    uint8_t AF = 0;

    AM_USERDATA_AFD_t afd = *((AM_USERDATA_AFD_t *)(pd + 7));
    AF = afd.af;
    return AF;
}

static void aml_swap_data(uint8_t *user_data, int ud_size) {
    int swap_blocks, i, j, k, m;
    unsigned char c_temp;

    /* swap byte order */
    swap_blocks = ud_size >> 3;
    for (i = 0; i < swap_blocks; i++) {
        j = i << 3;
        k = j + 7;
        for (m = 0; m < 4; m++) {
            c_temp = user_data[j];
            user_data[j++] = user_data[k];
            user_data[k--] = c_temp;
        }
    }
}

uint8_t processData(uint8_t *rawData, uint32_t *inst_id, uint32_t *vpts) {
    uint8_t af = 0xFF;
    int left = 0, r = 0;
    struct userdata_param_t *ud = (struct userdata_param_t *)rawData;
    uint8_t *pd = NULL;
    userdata_type mType;

    *inst_id = ud->instance_id;
    *vpts = ud->meta_info.vpts;
    if (ud->pbuf_addr == NULL || ud->buf_len == 0)
        return 0xff;

    pd = kzalloc(MAX_CC_DATA_LEN, GFP_ATOMIC);
    if (!pd) return af;

    r = ud->buf_len;
    r = (r > MAX_CC_DATA_LEN) ? MAX_CC_DATA_LEN : r;
    memcpy(pd, ud->pbuf_addr, r);
    aml_swap_data(pd + left, r);
    left += r;
    mType = checkFormat(ud, pd, left);
    switch (mType) {
        case MPEG_AFD_TYPE: {
            af = processMpegData(pd, left);
            break;
        }
        case H264_AFD_TYPE: {
            af = processH264Data(pd, left);
            break;
        }
        case INVALID_TYPE:
        default: {
            break;
        }
    }
    kfree(pd);

    return af;
}

uint8_t getAfdFromMetaInfo(uint8_t *rawData, uint32_t *inst_id, uint32_t *vpts) {
    uint8_t mAf = 0;
    uint8_t *pd = rawData;
    mAf = processData(pd, inst_id, vpts);
    return mAf;
}
