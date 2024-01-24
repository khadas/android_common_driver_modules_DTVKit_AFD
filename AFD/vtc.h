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
//#define ADD_CROP_RATIO_TO_OUT

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

typedef enum {
    OVER_SCAN_OFF,
    OVER_SCAN_AUTO,
    OVER_SCAN_FIXED,
} E_OVER_SCAN_MODE;

typedef enum {
    ASPECT_MODE_AUTO,
    ASPECT_MODE_4_3,
    ASPECT_MODE_16_9,
    ASPECT_MODE_14_9,
    ASPECT_MODE_ZOOM,
    ASPECT_MODE_CUSTOM,
} E_VIDEO_ASPECT_MODE;

typedef enum {
    ASPECT_RATIO_1_1,
    ASPECT_RATIO_4_3,
    ASPECT_RATIO_14_9,
    ASPECT_RATIO_16_9,
    ASPECT_RATIO_18_9,
    ASPECT_UNDEFINED = 255
} E_ASPECT_RATIO;

typedef struct s_fraction {
    int32_t numerator;
    int32_t denominator;
} S_VT_FRACTION;

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

typedef struct {
    S_VT_FRACTION h;
    S_VT_FRACTION v;
} S_VIDEO_SCALE;

typedef enum {
    BOX_TOP = 0,
    BOX_CENTER = 1,
} E_BOX_POS;

typedef struct {
    uint8_t afd;
    S_VT_FRACTION srcRatio;//src in frame
    E_BOX_POS posType;
    S_VT_FRACTION activeRatio;//active aspect in src (non protect)
} S_AFD_DESC_T;

typedef struct {
    E_VIDEO_ASPECT_MODE alignment;
    uint8_t scaling_mode;
    S_RECTANGLE app_scaling_window;
    bool afd_enabled;
    bool settings_changed;
    uint16_t resolution_width;
    uint16_t resolution_height;
    uint16_t video_width;
    uint16_t video_height;
    uint16_t virtual_video_width;
    uint16_t virtual_video_height;
    uint16_t screen_width;
    uint16_t screen_height;
    S_VT_FRACTION video_ratio;
    E_ASPECT_RATIO video_aspect_ratio;
    uint8_t afd;
    uint8_t frame_type;
    S_VT_FRACTION out_ratio;
    S_VIDEO_SCALE video_scale;
    S_RECTANGLE input_rectangle, output_rectangle;
} S_VT_CONVERSION_STATE;

void VT_Set_Global_Overscan(E_OVER_SCAN_MODE mode, S_VT_OVERSCANS_t *overscan);
void VT_Enter(void *context);
void VT_Leave(void *context);
void VT_Reset(S_VT_CONVERSION_STATE* vtc);
void VT_SetAppScaling(void *context, S_RECTANGLE *window, int resolution_width, int resolution_height);
void VT_SetVideoAlignmentPref(void *context, E_VIDEO_ASPECT_MODE alignment);
void VT_DisableScalingMode(void *context);
int getAspect(int numerator, int denominator);
void AFDHandle(void *context, S_FRAME_DIS_INFO *frame_info, int video_numerator, int video_denominator, uint8_t afd_value, uint8_t frame_type);
S_RECTANGLE getInRectangle(void *context);
S_RECTANGLE getOutRectangle(void *context);
S_RECTANGLE getScalingRect(void *context);
S_VT_FRACTION getOutRatio(void *context);

bool checkInScaling(void *context);
void print_vt_state(void *context, char* buf, int count);


#endif /*AML_VTC_H*/
