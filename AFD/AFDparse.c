/*
 *
 * Copyright (c) 2015 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 *
 */

#include "AFDparse.h"

#include <linux/kernel.h>
#include <string.h>

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
    int left = len;
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

uint8_t processData(uint8_t *rawData) {
    uint8_t af = 0xFF;
    int left = 0, r = 0;
    struct userdata_param_t *ud = (struct userdata_param_t *)rawData;
    uint8_t data[MAX_CC_DATA_LEN];
    uint8_t *pd = data;

    memset(pd, 0, MAX_CC_DATA_LEN);
    memcpy(pd, ud->pbuf_addr, ud->buf_len);
    r = ud->buf_len;
    r = (r > MAX_CC_DATA_LEN) ? MAX_CC_DATA_LEN : r;
    aml_swap_data(data + left, r);
    left += r;
    pd = data;
    userdata_type mType = checkFormat(ud, pd, left);
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
    return af;
}

uint8_t getaf(uint8_t *rawData) {
    uint8_t mAf = 0;
    uint8_t *pd = rawData;
    mAf = processData(pd);
    return mAf;
}
