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


#ifndef AML_VTC_H
#define AML_VTC_H

#include <linux/types.h>

#define ENABLE_INTERNAL_OVERSCAN

#define SD_OVERSCAN_H 22
#define SD_OVERSCAN_V 14
#define HD_OVERSCAN_H 42
#define HD_OVERSCAN_V 22

typedef struct {
    int hs;
    int vs;
    int re;
    int be;
} S_VT_CROP_t;

typedef struct {
    S_VT_CROP_t uhd;
    S_VT_CROP_t fhd;
    S_VT_CROP_t hd;
    S_VT_CROP_t sd;
} S_VT_OVERSCANS_t;

typedef struct s_vt_options {
    bool mheg_required;
    bool afd_required;
    bool hbbtv_required;
    uint8_t path;
} S_VT_OPTIONS;

typedef void (*F_VT_NOTIFICATION_CALLBACK)(void *userdata);

/*******************
 * EXPORTED DATA    *
 ********************/

/*******************
 * LOCAL TYPEDEFS   *
 ********************/

typedef enum {
    FORMAT_CONVERSION_UNKNOWN,
    FORMAT_CONVERSION_IGNORE,
    FORMAT_CONVERSION_PANSCAN,
    FORMAT_CONVERSION_LETTERBOX,
    FORMAT_CONVERSION_LETTERBOX_14_9,
    FORMAT_CONVERSION_PILLAR_BOX,
    FORMAT_CONVERSION_ZOOM_4_3,
    FORMAT_CONVERSION_ZOOM_14_9,
    FORMAT_CONVERSION_PANSCAN_14_9,
    FORMAT_CONVERSION_FULL_4_3,
    FORMAT_CONVERSION_CENTRE_14_9,
    FORMAT_CONVERSION_CENTRE_4_3,
    FORMAT_CONVERSION_PILLAR_BOX_4_3,
    FORMAT_CONVERSION_PILLAR_BOX_14_9,
} E_FORMAT_CONVERSION;

typedef enum {
    ASPECT_RATIO_4_3,
    ASPECT_RATIO_16_9,
    ASPECT_UNDEFINED = 255
} E_ASPECT_RATIO;

typedef enum {
    AFD_PREFERENCE_AUTO,
    AFD_PREFERENCE_16_9_LB,
    AFD_PREFERENCE_14_9_LB,
    AFD_PREFERENCE_CCO
} E_AFD_PREFERENCE;

typedef enum {
    MHEG_SCALING_NONE,
    MHEG_SCALING_QUARTER,
    MHEG_SCALING_OFFSET,
    MHEG_SCALING_OTHER,
} E_MHEG_SCALING;

typedef enum {
    ASPECT_MODE_AUTO,
    ASPECT_MODE_4_3,  /* 16:9 video, on 4:3 display/scene -> Centre Cut-Out */
    ASPECT_MODE_16_9, /* 16:9 video, on 4:3 display/scene -> Letter Box */
    ASPECT_MODE_14_9,
    ASPECT_MODE_ZOOM,
    ASPECT_MODE_CUSTOM,
} E_VIDEO_ASPECT_MODE;

typedef struct {
    uint8_t afd;
    E_ASPECT_RATIO video_aspect_ratio;
    E_ASPECT_RATIO display_aspect_ratio;
    E_AFD_PREFERENCE afd_preference;
    E_FORMAT_CONVERSION transformation;
} S_AFD_TRANS;

typedef struct s_fraction {
    int32_t numerator;
    int32_t denominator;
} S_VT_FRACTION;

/* (a 0 b)
   (0 c d)
   (0 0 1)*/
typedef struct s_matrix {
    S_VT_FRACTION a;
    S_VT_FRACTION b;
    S_VT_FRACTION c;
    S_VT_FRACTION d;
} S_VT_MATRIX;

/* (width ) (left)
   (height) (top )
   (  0   ) ( 1  )*/
typedef struct s_fract_rect {
    S_VT_FRACTION left;
    S_VT_FRACTION top;
    S_VT_FRACTION width;
    S_VT_FRACTION height;
} S_VT_FRACT_RECT;

typedef struct {
    int32_t left;
    int32_t top;
    uint32_t width;
    uint32_t height;
} S_RECTANGLE;

typedef struct {
    uint32_t video_width;
    uint32_t video_height;
    uint32_t screen_width;
    uint32_t screen_height;
} S_FRAME_DIS_INFO;

typedef enum {
    DECODER_STATUS_NONE,
    DECODER_STATUS_VIDEO,
    DECODER_STATUS_IFRAME
} E_STB_AV_DECODER_STATUS;

typedef enum {
    SCALING_NONE,
    SCALING_APP,
    SCALING_MHEG,
    SCALING_HBBTV
} E_APP_SCALING_TYPE;

typedef struct s_conversion_state {
    E_AFD_PREFERENCE afd_preference;
    E_VIDEO_ASPECT_MODE alignment;
    E_ASPECT_RATIO mheg_aspect_ratio;
    E_VIDEO_ASPECT_MODE mheg_wam;
    S_RECTANGLE mheg_scaling_rect;
    S_RECTANGLE app_scaling_window;
    S_RECTANGLE hbb_window_rect;
    bool afd_enabled;
    E_APP_SCALING_TYPE scaling_mode;
    bool mheg_scaling_given;
    bool settings_changed;
    uint16_t mheg_resolution_width;
    uint16_t mheg_resolution_height;
    uint16_t resolution_width;
    uint16_t resolution_height;
    uint16_t video_width;
    uint16_t video_height;
    uint16_t screen_width;
    uint16_t screen_height;
    E_ASPECT_RATIO video_aspect_ratio;
    E_ASPECT_RATIO display_aspect_ratio;
    E_STB_AV_DECODER_STATUS decoder_status;
    uint8_t afd;
    uint8_t wss;
    S_RECTANGLE input_rectangle, output_rectangle;
} S_VT_CONVERSION_STATE;

void VT_Set_Global_Overscan(S_VT_OVERSCANS_t *overscan);
void VT_Enter(void *context);
void VT_Leave(void *context);
void VT_Rest(S_VT_CONVERSION_STATE* vtc);


/*!**************************************************************************
 * @brief    Set MHEG-5 scaling information
 * @param    context - transformation calculator context
 * @param    scaling - scaling and positioning transformation
 * @Param    resolution_width
 * @Param    resolution_height
 * @note     When scaling is NULL, scaling is ignored and the behaviour will
 *           be as if full screen video is mapped to the full screen.
 ****************************************************************************/
void VT_SetMhegScaling(void *context, S_RECTANGLE *scaling, int resolution_width, int resolution_height);

/*!**************************************************************************
 * @brief    Set application scaling information
 * @param    context - transformation calculator context
 * @param    window - output window (screen CS)
 * @Param    resolution_width
 * @Param    resolution_height
 * @note     When window is NULL, application scaling is turned off
 ****************************************************************************/
void VT_SetAppScaling(void *context, S_RECTANGLE *window, int resolution_width, int resolution_height);

/*!**************************************************************************
 * @brief    Set user preference for video aspect ratio
 *
 * @param    context - transformation calculator context
 * @param    alignment - New video alignment preference:
 ****************************************************************************/
void VT_SetVideoAlignmentPref(void *context, E_VIDEO_ASPECT_MODE alignment);

/*!**************************************************************************
 * @brief    Disable scaling mode
 * @param    context - transformation calculator context
 * @return
 ****************************************************************************/
void VT_DisableScalingMode(void *context);

/*!**************************************************************************
 * @brief    get aspect ratio
 * @param    numerator - 4/16 , denominator - 3/9
 * @return   0:ASPECT_RATIO_4_3, 1:ASPECT_RATIO_16_9
 *           255:ASPECT_UNDEFINED
 ****************************************************************************/
int getAspect(int numerator, int denominator);
/*!**************************************************************************
 * @brief    get aspect ratio
 * @param    width - video width, height - video height
 * @return   0:ASPECT_RATIO_4_3, 1:ASPECT_RATIO_16_9
 *           255:ASPECT_UNDEFINED
 ****************************************************************************/
int getDisplayAspect(int width, int height);
/*!**************************************************************************
 * @brief    AFD  Handle algorithm
 * @param    context - transformation calculator context
 * @parm     frame_info - FRAME_DIS_INFO
 * @parm     frame_aspectratio - aspect ratio
 * @parm     uint8_t afd_value
 ****************************************************************************/
void AFDHandle(void *context, S_FRAME_DIS_INFO *frame_info,
               E_ASPECT_RATIO frame_aspectratio, uint8_t afd_value);
/*!**************************************************************************
 * @brief    get input_rectangle /sys/class/video/crop
 * @param    context - transformation calculator context
 ****************************************************************************/
S_RECTANGLE getInRectangle(void *context);
/*!**************************************************************************
 * @brief    get out_rectangle /sys/class/video/axis
 * @param    context - transformation calculator context
 ****************************************************************************/
S_RECTANGLE getOutRectangle(void *context);
/*!**************************************************************************
 * @brief    get current scaling rectangle
 * @param    context - transformation calculator context
 ****************************************************************************/
S_RECTANGLE getScalingRect(void *context);
/*!**************************************************************************
 * @brief    check scaling result for vpp
 * @param    context - transformation calculator context
 ****************************************************************************/
bool checkInScaling(void *context);
void print_vt_state(void *context, char* buf, int count);

#ifdef __cplusplus
}
#endif

#endif /*AML_VTC_H*/
