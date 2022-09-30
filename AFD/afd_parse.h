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


#ifndef AFD_PARSE_H
#define AFD_PARSE_H

#include <linux/types.h>
#include <linux/slab.h>

#define MAX_CC_DATA_LEN (1024 * 5)

//#define IS_H264(p)  ((p[0] == 0xb5 && p[3] == 0x47 && p[4] == 0x41 && p[5] ==
//0x39 && p[6] == 0x34)) #define IS_DIRECTV(p)   ((p[0] == 0xb5 && p[1] == 0x00
//&& p[2] == 0x2f)) #define IS_AVS(p)   ((p[0] == 0x47) && (p[1] == 0x41) &&
//(p[2] == 0x39) && (p[3] == 0x34)) #define IS_ATSC(p)    ((p[0] == 0x47) &&
//(p[1] == 0x41) && (p[2] == 0x39) && (p[3] == 0x34) && (p[4] == 0x3)) #define
//IS_ATSC(p, q)   ((p[0] == 0x47) && (p[1] == 0x41) && (p[2] == 0x39) && (p[3]
//== 0x34) && (q[0] == 0x3)) #define IS_SCTE(p)  ((p[0]==0x3) && ((p[1]&0x7f) ==
//1))

#define IS_AFD(p) \
    ((p[0] == 0x44) && (p[1] == 0x54) && (p[2] == 0x47) && (p[3] == 0x31))
#define IS_H264_AFD(p)                                                       \
    ((p[0] == 0xb5) && (p[3] == 0x44) && (p[4] == 0x54) && (p[5] == 0x47) && \
     (p[6] == 0x31))

enum AM_USERDATA_Mode {
    AM_USERDATA_MODE_CC = 0x1,
    AM_USERDATA_MODE_AFD = 0x2,
};

#define MOD_ON(__mod, __mask) (__mod & __mask)
#define MOD_ON_CC(__mod) MOD_ON(__mod, AM_USERDATA_MODE_CC)
#define MOD_ON_AFD(__mod) MOD_ON(__mod, AM_USERDATA_MODE_AFD)

typedef enum {
    INVALID_TYPE = 0,
    CC_TYPE_FOR_JUDGE = 10,
    MPEG_CC_TYPE,
    H264_CC_TYPE,
    DIRECTV_CC_TYPE,
    AVS_CC_TYPE,
    SCTE_CC_TYPE,
    AFD_TYPE_FOR_JUDGE = 100,
    MPEG_AFD_TYPE,
    H264_AFD_TYPE,
    USERDATA_TYPE_MAX,
} userdata_type;

#define IS_AFD_TYPE(p) (p > AFD_TYPE_FOR_JUDGE && p < USERDATA_TYPE_MAX)
#define IS_CC_TYPE(p) (p > CC_TYPE_FOR_JUDGE && p < AFD_TYPE_FOR_JUDGE)

typedef enum {
    /* 0 forbidden */
    I_TYPE = 1,
    P_TYPE = 2,
    B_TYPE = 3,
    D_TYPE = 4,
    /* 5 ... 7 reserved */
} picture_coding_type;

typedef enum {
    VFORMAT_UNKNOWN = -1,
    VFORMAT_MPEG12 = 0,
    VFORMAT_MPEG4 = 1,
    VFORMAT_H264 = 2,
    VFORMAT_MJPEG = 3,
    VFORMAT_REAL = 4,
    VFORMAT_JPEG = 5,
    VFORMAT_VC1 = 6,
    VFORMAT_AVS = 7,
    VFORMAT_SW = 8,
    VFORMAT_H264MVC = 9,
    VFORMAT_H264_4K2K = 10,
    VFORMAT_HEV = 11,
    VFORMAT_H264_EN = 12,
    VFORMAT_JPEG_ENC = 13,
    VFORMAT_VP9 = 14,
    VFORMAT_AVS2 = 15,
    VFORMAT_AV1 = 16,
    VFORMAT_MAX = 9999999,
} vformat_t;

struct userdata_meta_info_t {
    uint32_t poc_number;
    /************ flags bit defination ***********
    bit 0:		//used for mpeg2
            1, group start
            0, not group start
    bit 1-2:	//used for mpeg2
            0, extension_and_user_data( 0 )
            1, extension_and_user_data( 1 )
            2, extension_and_user_data( 2 )
    bit 3-6:	//video format
            0,	VFORMAT_MPEG12
            1,	VFORMAT_MPEG4
            2,	VFORMAT_H264
            3,	VFORMAT_MJPEG
            4,	VFORMAT_REAL
            5,	VFORMAT_JPEG
            6,	VFORMAT_VC1
            7,	VFORMAT_AVS
            8,	VFORMAT_SW
            9,	VFORMAT_H264MVC
            10, VFORMAT_H264_4K2K
            11, VFORMAT_HEVC
            12, VFORMAT_H264_ENC
            13, VFORMAT_JPEG_ENC
            14, VFORMAT_VP9
    bit 7-9:	//frame type
            0, Unknown Frame Type
            1, I Frame
            2, B Frame
            3, P Frame
            4, D_Type_MPEG2
    bit 10:  //top_field_first_flag valid
            0: top_field_first_flag is not valid
            1: top_field_first_flag is valid
    bit 11: //top_field_first bit val
    bit 12-13: //picture_struct, used for H264
            0: Invalid
            1: TOP_FIELD_PICTURE
            2: BOT_FIELD_PICTURE
            3: FRAME_PICTURE
    **********************************************/
    uint32_t flags;
    uint32_t vpts; /*video frame pts*/
    /******************************************
    0: pts is invalid, please use duration to calculate
    1: pts is valid
    ******************************************/
    uint32_t vpts_valid;
    /*duration for frame*/
    uint32_t duration;
    /* how many records left in queue waiting to be read*/
    uint32_t records_in_que;
    unsigned long long priv_data;
    uint32_t padding_data[4];
};

struct userdata_param_t {
    uint32_t version;
    uint32_t instance_id;                  /*input, 0~9*/
    uint32_t buf_len;                      /*input*/
    uint32_t data_size;                    /*output*/
    void *pbuf_addr;                       /*input*/
    struct userdata_meta_info_t meta_info; /*output*/
};

typedef struct {
    uint32_t picture_structure : 16;
    uint32_t temporal_reference : 10;
    uint32_t picture_coding_type : 3;
    uint32_t reserved : 3;
    uint32_t index : 16;
    uint32_t offset : 16;
    uint8_t atsc_flag[4];
    uint8_t cc_data_start[4];
} aml_ud_header_t;

typedef struct {
    uint8_t : 6;
    uint8_t af_flag : 1;
    uint8_t : 1;
    uint8_t af : 4;
    uint8_t : 4;
    uint16_t reserved;
    uint32_t pts;
} AM_USERDATA_AFD_t;

struct CCData {
    uint8_t *buf;
    uint32_t pts;
    uint32_t duration;
    int32_t pts_valid;
    int32_t size;
    int32_t poc;
};

//static void aml_swap_data(uint8_t *user_data, int ud_size);
uint8_t processData(uint8_t *rawData, uint32_t *inst_id, uint32_t *vpts, uint32_t debug);
userdata_type checkFormat(struct userdata_param_t *ud, uint8_t *buf, int len);
uint8_t processMpegData(uint8_t *data, int len);
uint8_t processH264Data(uint8_t *data, int len);

/*!**************************************************************************
 * @brief    get afd value
 * @param    rawdata - user data include afd info
 * @Param    inst_id - pointer to store inst_id
 * @Param    vpts    - pointer to store vpts
 * @return afd value ,if return 255 , means can't get afd value
 ****************************************************************************/
uint8_t getAfdFromMetaInfo(uint8_t *rawData, uint32_t *inst_id, uint32_t *vpts, uint32_t debug);

#endif  // AMCODEC_USERDATA_DEVICE_H
