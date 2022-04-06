/*
 *
 * Copyright (c) 2015 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 *
 */
#ifndef AFD_PARSE_H
#define AFD_PARSE_H

/* There is some amount of overlap with <sys/types.h> as known by inet code */
#ifndef __int8_t_defined
#define __int8_t_defined
typedef signed char int8_t;
typedef short int int16_t;
typedef int int32_t;
#if __WORDSIZE == 64
typedef long int int64_t;
#else
__extension__ typedef long long int int64_t;
#endif
#endif

/* Unsigned.  */
typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
#ifndef __uint32_t_defined
typedef unsigned int uint32_t;
#define __uint32_t_defined
#endif
#if __WORDSIZE == 64
typedef unsigned long int uint64_t;
#else
__extension__ typedef unsigned long long int uint64_t;
#endif

#define MAX_CC_DATA_LEN (1024 * 5 + 4)

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
} vformat_t;  // defined in <vcodec.h> ,move to this file-chenfei.dou

#define AMSTREAM_IOC_MAGIC 'S'
#define AMSTREAM_IOC_UD_LENGTH _IOR(AMSTREAM_IOC_MAGIC, 0x54, unsigned long)
#define AMSTREAM_IOC_UD_POC _IOR(AMSTREAM_IOC_MAGIC, 0x55, int)
#define AMSTREAM_IOC_UD_FLUSH_USERDATA _IOR(AMSTREAM_IOC_MAGIC, 0x56, int)
#define AMSTREAM_IOC_UD_BUF_READ _IOR(AMSTREAM_IOC_MAGIC, 0x57, int)
#define AMSTREAM_IOC_UD_AVAIBLE_VDEC \
    _IOR(AMSTREAM_IOC_MAGIC, 0x5c, unsigned int)
#define AMSTREAM_IOC_GET_MVDECINFO _IOR(AMSTREAM_IOC_MAGIC, 0xcb, int)

#define QOS_FRAME_NUM 8

struct vframe_qos_s {
    uint32_t num;
    uint32_t type;
    uint32_t size;
    uint32_t pts;
    int32_t max_qp;
    int32_t avg_qp;
    int32_t min_qp;
    int32_t max_skip;
    int32_t avg_skip;
    int32_t min_skip;
    int32_t max_mv;
    int32_t min_mv;
    int32_t avg_mv;
    int32_t decode_buffer;  // For padding currently
} /*vframe_qos */;

struct vframe_comm_s {
    int32_t vdec_id;
    uint8_t vdec_name[16];
    uint32_t vdec_type;
};

struct vframe_counter_s {
    struct vframe_qos_s qos;
    uint32_t decode_time_cost; /*us*/
    uint32_t frame_width;
    uint32_t frame_height;
    uint32_t frame_rate;
    uint32_t bit_depth_luma;  // original bit_rate;
    uint32_t frame_dur;
    uint32_t bit_depth_chroma;  // original frame_data;
    uint32_t error_count;
    uint32_t status;
    uint32_t frame_count;
    uint32_t error_frame_count;
    uint32_t drop_frame_count;
    uint64_t total_data;         // this member must be 8 bytes alignment
    uint32_t double_write_mode;  // original samp_cnt;
    uint32_t offset;
    uint32_t ratio_control;
    uint32_t vf_type;
    uint32_t signal_type;
    uint32_t pts;
    uint64_t pts_us64;
#if ANDROID_PLATFORM_SDK_VERSION >= 29
    /*mediacodec report*/
    unsigned int i_decoded_frames;    // i frames decoded
    unsigned int i_lost_frames;       // i frames can not be decoded
    unsigned int i_concealed_frames;  // i frames decoded but have some error
    unsigned int p_decoded_frames;
    unsigned int p_lost_frames;
    unsigned int p_concealed_frames;
    unsigned int b_decoded_frames;
    unsigned int b_lost_frames;
    unsigned int b_concealed_frames;
    unsigned int av_resynch_counter;
#endif
};

struct av_param_mvdec_t {
    int32_t vdec_id;

    /*This member is used for versioning this structure.
     *When passed from userspace, its value must be
     *sizeof(struct av_param_mvdec_t)
     */
    int32_t struct_size;

    int32_t slots;

    struct vframe_comm_s comm;
    struct vframe_counter_s minfo[QOS_FRAME_NUM];
};

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
    0: pts is invalid, please use duration to calcuate
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

static void aml_swap_data(uint8_t *user_data, int ud_size);
uint8_t processData(uint8_t *rawData, uint32_t *inst_id, uint32_t *vpts);
userdata_type checkFormat(struct userdata_param_t *ud, uint8_t *buf, int len);
uint8_t processMpegData(uint8_t *data, int len);
uint8_t processH264Data(uint8_t *data, int len);

/*!**************************************************************************
 * @brief    get afd value
 * @param    rawdata - user data include afd info
 * @Param    inst_id - pionter to store inst_id
 * @Param    vpts    - pionter to store vpts
 * @return afd value ,if return 255 , means can't get afd value
 ****************************************************************************/
uint8_t getaf(uint8_t *rawData, uint32_t *inst_id, uint32_t *vpts);

#endif  // AMCODEC_USERDATA_DEVICE_H
