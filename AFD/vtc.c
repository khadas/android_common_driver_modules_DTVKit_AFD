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


/*******************
 * INCLUDE FILES    *
 ********************/
#include <linux/kernel.h>
#include <linux/string.h>
#include "vtc.h"
#include "afd_parse.h"

#ifndef NULL
#include <stddef.h>
#endif

#define TRUE true
#define FALSE false
#define ASSERT(x)

#define SD_WIDTH 720
#define SD_HEIGHT 576
#define HD_WIDTH 1280
#define HD_HEIGHT 720
#define FHD_WIDTH 1920
#define FHD_HEIGHT 1080

static E_OVER_SCAN_MODE mOverscanMode;
static S_VT_OVERSCANS_t mVtOverscan;

static const S_AFD_DESC_T afd_descriptions[] = {
    {0,  {0, 0}, BOX_CENTER, {0, 0}},
    {2,  {16, 9}, BOX_TOP, {16, 9}},
    {3,  {14, 9}, BOX_TOP, {14, 9}},
    {4,  {16, 9}, BOX_CENTER, {16, 0}},
    {8,  {0, 0}, BOX_CENTER, {0, 0}},
    {9,  {4, 3}, BOX_CENTER, {4, 3}},
    {10, {16, 9}, BOX_CENTER, {16, 9}},
    {11, {14, 9}, BOX_CENTER, {14, 9}},
    {12, {0, 0}, BOX_CENTER, {0, 0}},
    {13, {4, 3}, BOX_CENTER, {14, 9}},
    {14, {16, 9}, BOX_CENTER, {14, 9}},
    {15, {16, 9}, BOX_CENTER, {4, 3}},
};

static void InitRect(S_RECTANGLE *rect, int32_t left, int32_t top, int32_t width,
                     int32_t height);

void VT_Set_Global_Overscan(E_OVER_SCAN_MODE mode, S_VT_OVERSCANS_t *overscan) {
    mOverscanMode = mode;
    if (overscan) {
        memcpy(&mVtOverscan, overscan, sizeof(S_VT_OVERSCANS_t));
    }
}

void VT_Enter(void *context) {
    S_VT_CONVERSION_STATE *state = (S_VT_CONVERSION_STATE *)context;
    if (state != NULL) {
        state->afd_enabled = TRUE;
        state->settings_changed = TRUE;
    }
}

void VT_Leave(void *context) {
    S_VT_CONVERSION_STATE *state = (S_VT_CONVERSION_STATE *)context;
    if (state != NULL) {
        state->afd_enabled = FALSE;
        state->afd = 0;
    }
}

void VT_Reset(S_VT_CONVERSION_STATE* vtc) {
    S_VT_CONVERSION_STATE *new_state = vtc;

    if (new_state != NULL) {
        new_state->afd_enabled = FALSE;
        new_state->alignment = ASPECT_MODE_AUTO;
        new_state->afd = 0;//default is 1000 for aspect auto

        InitRect(&new_state->app_scaling_window, 0, 0, SD_WIDTH, SD_HEIGHT);

        new_state->resolution_width = SD_WIDTH;
        new_state->resolution_height = SD_HEIGHT;

        new_state->video_width = SD_WIDTH;
        new_state->video_height = SD_HEIGHT;
        new_state->virtual_video_width = SD_WIDTH;
        new_state->virtual_video_height = SD_HEIGHT;
        new_state->video_ratio.numerator = 4;
        new_state->video_ratio.denominator = 3;
        new_state->video_aspect_ratio = ASPECT_UNDEFINED;

        new_state->screen_width = SD_WIDTH;
        new_state->screen_height = SD_HEIGHT;

        new_state->out_ratio.numerator = 4;
        new_state->out_ratio.denominator = 3;

        new_state->video_scale.h.numerator = 1;
        new_state->video_scale.h.denominator = 1;
        new_state->video_scale.v.numerator = 1;
        new_state->video_scale.v.denominator = 1;

        new_state->settings_changed = FALSE;

        InitRect(&new_state->input_rectangle, 0, 0, SD_WIDTH, SD_HEIGHT);
        InitRect(&new_state->output_rectangle, 0, 0, SD_WIDTH, SD_HEIGHT);
    }
}

void VT_SetVideoAlignmentPref(void *context, E_VIDEO_ASPECT_MODE alignment) {
    S_VT_CONVERSION_STATE *state;

    if (context == NULL) return;

    state = (S_VT_CONVERSION_STATE *)context;

    if (state->alignment != alignment) {
        state->alignment = alignment;
        state->settings_changed = TRUE;
    }
}

void VT_SetAppScaling(void *context, S_RECTANGLE *window, int resolution_width, int resolution_height) {
    S_VT_CONVERSION_STATE *state;

    ASSERT(context != NULL);
    if (context == NULL) return;

    state = (S_VT_CONVERSION_STATE *)context;

    if (window == NULL) {
        if (state->scaling_mode == 1) {
            state->scaling_mode = 0;
            state->settings_changed = TRUE;
        }
    } else {
        if (window ->width >= resolution_width || window->height >= resolution_height) {
            state->scaling_mode = 0;
            return;
        }
        state->scaling_mode = 1;
        state->app_scaling_window = *window;
        state->resolution_width = resolution_width;
        state->resolution_height = resolution_height;
        state->settings_changed = TRUE;
    }
}

void VT_DisableScalingMode(void *context){
    S_VT_CONVERSION_STATE *state;

    if (context == NULL) return;

    state = (S_VT_CONVERSION_STATE *)context;
    if (state->scaling_mode != 0) {
        state->scaling_mode = 0;
        state->settings_changed = TRUE;
    }
}

static int32_t gcd(int32_t a, int32_t b) {
    int32_t r;

    while (b != 0) {
        r = b;
        b = a % b;
        a = r;
    }

    return a;
}

static void findAfdDesc(int afd, S_AFD_DESC_T * afdDesc) {
    int i = 0;

    for (i = 0; i < sizeof(afd_descriptions)/sizeof(S_AFD_DESC_T); i ++) {
        S_AFD_DESC_T a = afd_descriptions[i];
        if (a.afd == afd) {
            memcpy(afdDesc, &a, sizeof(S_AFD_DESC_T));
            break;
        }
    }
}

uint16_t scaleInt(uint16_t x, uint16_t ty, uint16_t tx) {
    uint16_t y = x * ty / tx;
    if (tx * y < ty * x)
        y += 1;
    return y;
}

static int compareRatio(S_VT_FRACTION a, S_VT_FRACTION b) {
    return (a.numerator * b.denominator - a.denominator * b.numerator);
}

static void scaleFrameToAr(S_VT_CONVERSION_STATE *state) {
    S_VT_FRACTION frameRatio = {state->video_width, state->video_height};
    int frameCmpAr = compareRatio(frameRatio, state->video_ratio);

    if (frameCmpAr > 0) {
        int h = scaleInt(state->video_width,
                            state->video_ratio.denominator,
                            state->video_ratio.numerator);
        int g = gcd(h, state->video_height);
        state->virtual_video_height = h;
        state->video_scale.v.numerator = state->video_height / g;
        state->video_scale.v.denominator = h / g;
    } else if (frameCmpAr < 0) {
        int w = scaleInt(state->video_height,
                            state->video_ratio.numerator,
                            state->video_ratio.denominator);
        int g = gcd(w, state->video_width);
        state->virtual_video_width = w;
        state->video_scale.h.numerator = state->video_width / g;
        state->video_scale.h.denominator = w / g;
    }
}

static void scaleArToFrame(S_VT_CONVERSION_STATE *state, S_RECTANGLE *videoRet) {
    videoRet->left =
        videoRet->left * state->video_scale.h.numerator / state->video_scale.h.denominator;
    videoRet->width = state->video_width - (videoRet->left * 2);
    videoRet->top =
        videoRet->top * state->video_scale.v.numerator / state->video_scale.v.denominator;
    videoRet->height =
        videoRet->height * state->video_scale.v.numerator / state->video_scale.v.denominator;
}

static void updateActiveInfo(S_VT_CONVERSION_STATE *state,
                                    S_AFD_DESC_T * afdDesc,
                                    S_VT_FRACTION frameRatio,
                                    S_RECTANGLE *videoRet) {
    if (afdDesc->afd == 0 || afdDesc->afd == 8 || afdDesc->afd == 12) {
        afdDesc->srcRatio.numerator = frameRatio.numerator;
        afdDesc->activeRatio.numerator = frameRatio.numerator;
        afdDesc->srcRatio.denominator = frameRatio.denominator;
        afdDesc->activeRatio.denominator = frameRatio.denominator;
    }
    scaleFrameToAr(state);
    videoRet->width = state->virtual_video_width;
    videoRet->height = state->virtual_video_height;
}

static void applyOverscan(S_VT_CONVERSION_STATE *state, S_AFD_DESC_T *afdDesc, S_RECTANGLE *videoRet) {
    S_VT_CROP_t cropInfo;
    int tmp;
    uint8_t ratio;
    uint8_t apply = 0;
    uint16_t videoWidth = state->video_width;
    uint16_t videoHeight = state->video_height;
    S_VT_FRACTION frameRatio = {videoWidth, videoHeight};

    if (mOverscanMode == OVER_SCAN_FIXED) {
        apply = 1;
        if (videoWidth > 1920 || videoHeight > 1080) {
            cropInfo.hs = mVtOverscan.uhd.hs;
            cropInfo.vs = mVtOverscan.uhd.vs;
            cropInfo.re = mVtOverscan.uhd.re;
            cropInfo.be = mVtOverscan.uhd.be;
        } else if (videoWidth == 1920 && videoHeight == 1080) {
            cropInfo.hs = mVtOverscan.fhd.hs;
            cropInfo.vs = mVtOverscan.fhd.vs;
            cropInfo.re = mVtOverscan.fhd.re;
            cropInfo.be = mVtOverscan.fhd.be;
        } else if (videoWidth > SD_WIDTH) {
            cropInfo.hs = mVtOverscan.hd.hs;
            cropInfo.vs = mVtOverscan.hd.vs;
            cropInfo.re = mVtOverscan.hd.re;
            cropInfo.be = mVtOverscan.hd.be;
            if (videoWidth == videoHeight) {
                cropInfo.hs = cropInfo.vs;
                cropInfo.re = cropInfo.be;
            }
        } else if (videoWidth <= SD_WIDTH) {
            cropInfo.hs = mVtOverscan.sd.hs;
            cropInfo.vs = mVtOverscan.sd.vs;
            cropInfo.re = mVtOverscan.sd.re;
            cropInfo.be = mVtOverscan.sd.be;
        }
    } else if (mOverscanMode == OVER_SCAN_AUTO) {
        apply = 1;
        ratio = 25; //25 in 1000
        if (state->afd == 11 && compareRatio(state->video_ratio, frameRatio)) {
            //DTG AFD 2.0 ts, afd 11 (720x576, AR 16:9) has wrong overscan area
            //It has been fixed in 12.60, but the old stream still using
            cropInfo.vs = cropInfo.be = videoHeight * 20 / 1000;
            cropInfo.hs = cropInfo.re = cropInfo.vs;
        } else {
            cropInfo.hs = cropInfo.re = videoWidth * ratio / 1000;
            cropInfo.vs = cropInfo.be = videoHeight * ratio / 1000;
        }
    }
    if (videoHeight >= 2160) {
        //no overscan for 4k/8k
        cropInfo.hs = cropInfo.re = 0;
        cropInfo.vs = cropInfo.be = 0;
    }

    if (apply) {
        tmp = scaleInt(cropInfo.hs,
                    state->video_scale.h.denominator,
                    state->video_scale.h.numerator);
        videoRet->left += tmp;
        videoRet->width -= tmp * 2;
        tmp = scaleInt(cropInfo.vs,
                    state->video_scale.v.denominator,
                    state->video_scale.v.numerator);
        videoRet->top += tmp;
        if (afdDesc->posType == BOX_TOP &&
                (compareRatio(afdDesc->srcRatio, state->video_ratio) > 0))
            videoRet->height -= tmp;
        else
            videoRet->height -= tmp * 2;
    }
}

static void cropToDisplay(S_VT_CONVERSION_STATE *state,
                                S_AFD_DESC_T * afdDesc,
                                S_VT_FRACTION frameRatio,
                                S_RECTANGLE *videoRet,
                                S_VT_FRACTION *outRatio) {
    int dstCmpFrame = 0;
    uint16_t screenWidth = state->screen_width;
    uint16_t screenHeight = state->screen_height;

    //for afd 4, src ratio > 16:9 or more, crop based to display ratio
    if (afdDesc->activeRatio.denominator == 0) {
        S_VT_FRACTION displayRatio = {screenWidth, screenHeight};
        S_VT_FRACTION ratio16_9 = {16, 9};

        if (compareRatio(displayRatio, ratio16_9)) {
            displayRatio.numerator = 16;
            displayRatio.denominator = 9;
        }

        dstCmpFrame = compareRatio(displayRatio, frameRatio);
        if (dstCmpFrame > 0) {
            //crop to display
            uint16_t h = scaleInt(videoRet->width,
                displayRatio.denominator, displayRatio.numerator);
            videoRet->top += (videoRet->height - h) / 2;
            videoRet->height = h;
            outRatio->numerator = displayRatio.numerator;
            outRatio->denominator = displayRatio.denominator;
        }
    }
}

static void cropBlueArea(S_AFD_DESC_T * afdDesc,
                                S_VT_FRACTION frameRatio,
                                S_RECTANGLE *videoRet,
                                S_VT_FRACTION *outRatio) {
    int srcCmpFrame = 0;

    if (afdDesc->activeRatio.denominator == 0)
        return;

    srcCmpFrame = compareRatio(afdDesc->srcRatio, frameRatio);
    if (srcCmpFrame > 0) {
        //crop vertical
        uint16_t h = scaleInt(videoRet->width,
                                afdDesc->srcRatio.denominator,
                                afdDesc->srcRatio.numerator);
        if (afdDesc->posType == BOX_CENTER) {
            videoRet->top += (videoRet->height - h) / 2;
        }
        videoRet->height = h;
    } else if (srcCmpFrame < 0) {
        //crop horizontal
        uint16_t w = scaleInt(videoRet->height,
                                afdDesc->srcRatio.numerator,
                                afdDesc->srcRatio.denominator);
        videoRet->left += (videoRet->width - w) / 2;
        videoRet->width = w;
    }

    outRatio->numerator = afdDesc->srcRatio.numerator;
    outRatio->denominator = afdDesc->srcRatio.denominator;
}

static void cropProtectArea(S_AFD_DESC_T * afdDesc,
                                    uint16_t screenWidth,
                                    uint16_t screenHeight,
                                    S_RECTANGLE *videoRet,
                                    S_VT_FRACTION *outRatio) {
    int srcCmpActive, dstCmpSrc, dstCmpActive;
    S_VT_FRACTION displayRatio = {screenWidth, screenHeight};

    if (afdDesc->activeRatio.denominator == 0)
        return;

    srcCmpActive = compareRatio(afdDesc->srcRatio, afdDesc->activeRatio);
    dstCmpSrc = compareRatio(displayRatio, afdDesc->srcRatio);
    dstCmpActive = compareRatio(displayRatio, afdDesc->activeRatio);

    if (srcCmpActive > 0) {
        //horizontal protect
        uint16_t w = videoRet->width;
        if (dstCmpActive <= 0) {
            //crop to active
            w = scaleInt(videoRet->height,
                            afdDesc->activeRatio.numerator,
                            afdDesc->activeRatio.denominator);
            outRatio->numerator = afdDesc->activeRatio.numerator;
            outRatio->denominator = afdDesc->activeRatio.denominator;
        } else if (dstCmpSrc < 0) {
            //src ratio > display ratio
            //crop protect to display ratio
            w = scaleInt(videoRet->height, screenWidth, screenHeight);
            outRatio->numerator = screenWidth;
            outRatio->denominator = screenHeight;
        }
        videoRet->left += (videoRet->width - w) / 2;
        videoRet->width = w;
    } else if (srcCmpActive < 0) {
        //vertical protect
        uint16_t h = videoRet->height;
        if (dstCmpActive >= 0) {
            //crop to active
            h = scaleInt(videoRet->width,
                            afdDesc->activeRatio.denominator,
                            afdDesc->activeRatio.numerator);
            outRatio->numerator = afdDesc->activeRatio.numerator;
            outRatio->denominator = afdDesc->activeRatio.denominator;
        } else if (dstCmpSrc > 0) {
            //crop to display ratio
            h = scaleInt(videoRet->width, screenHeight, screenWidth);
            outRatio->numerator = screenWidth;
            outRatio->denominator = screenHeight;
        }
        videoRet->top += (videoRet->height - h) / 2;
        videoRet->height = h;
    }
}

static void calculateDisplayPosition(uint16_t screenWidth, uint16_t screenHeight,
                                            S_RECTANGLE *displayRet,
                                            S_VT_FRACTION *outRatio) {
    S_VT_FRACTION displayRatio = {screenWidth, screenHeight};
    int dstCmpActive = compareRatio(displayRatio, *outRatio);

    if (dstCmpActive > 0) {
        uint16_t w = scaleInt(screenHeight, outRatio->numerator, outRatio->denominator);
        displayRet->left = (screenWidth - w) / 2;
        displayRet->width = w;
    } else if (dstCmpActive < 0) {
        uint16_t h = scaleInt(screenWidth, outRatio->denominator, outRatio->numerator);
        displayRet->top = (screenHeight - h) / 2;
        displayRet->height = h;
    }
}

static void doAppScaling(S_VT_CONVERSION_STATE *state, S_RECTANGLE * output) {
    if (state->scaling_mode) {
        output->left = scaleInt(state->app_scaling_window.left,
                            state->screen_width, state->resolution_width);
        output->width = scaleInt(state->app_scaling_window.width,
                            state->screen_width, state->resolution_width);
        output->top = scaleInt(state->app_scaling_window.top,
                            state->screen_height, state->resolution_height);
        output->height = scaleInt(state->app_scaling_window.height,
                            state->screen_height, state->resolution_height);
    }
}

static void processAfd(S_VT_CONVERSION_STATE *state) {
    S_RECTANGLE input_rectangle, output_rectangle;
    S_VT_FRACTION frameRatio;
    S_VT_FRACTION outputRatio;
    S_AFD_DESC_T afdDesc;
    int g;

    input_rectangle.top = 0;
    input_rectangle.left = 0;
    input_rectangle.width = state->video_width;
    input_rectangle.height = state->video_height;

    output_rectangle.left = 0;
    output_rectangle.top = 0;
    output_rectangle.width = state->screen_width;
    output_rectangle.height = state->screen_height;

    outputRatio.numerator = state->video_ratio.numerator;
    outputRatio.denominator = state->video_ratio.denominator;

    if (state->scaling_mode) {
        doAppScaling(state, &output_rectangle);
    } else {
        frameRatio.numerator = state->video_ratio.numerator;
        frameRatio.denominator = state->video_ratio.denominator;

        afdDesc.afd = 0xff;
        findAfdDesc(state->afd, &afdDesc);
        if (afdDesc.afd != 0xff) {
            //update dynamic src info for afd 8, and scale video frame to math ar
            updateActiveInfo(state, &afdDesc, frameRatio, &input_rectangle);
            //apply overscan if enabled
            applyOverscan(state, &afdDesc, &input_rectangle);
            //for afd 4, directly scale to display
            cropToDisplay(state, &afdDesc, frameRatio, &input_rectangle, &outputRatio);
            //crop extra area in the src to frame
            cropBlueArea(&afdDesc, frameRatio, &input_rectangle, &outputRatio);
            //crop shoot and protect area base on display ratio
            cropProtectArea(&afdDesc, state->screen_width,
                state->screen_height, &input_rectangle, &outputRatio);
            //place to screen
            calculateDisplayPosition(state->screen_width, state->screen_height,
                &output_rectangle, &outputRatio);
            //scale from virtual video to frame
            scaleArToFrame(state, &input_rectangle);
            //calculate output ratio
            g = gcd(outputRatio.numerator, outputRatio.denominator);
            outputRatio.numerator = (outputRatio.numerator / g);
            outputRatio.denominator = (outputRatio.denominator / g);
        }
    }

    state->input_rectangle = input_rectangle;
    state->output_rectangle = output_rectangle;
    state->out_ratio = outputRatio;

    state->settings_changed = FALSE;
}

static void InitRect(S_RECTANGLE *rect, int32_t left, int32_t top, int32_t width,
                     int32_t height) {
    rect->left = left;
    rect->top = top;
    rect->width = width;
    rect->height = height;
}

static void UpdateResolution(S_VT_CONVERSION_STATE *state) {
    state->virtual_video_width = state->video_width;
    state->virtual_video_height = state->video_height;
    state->video_scale.h.numerator = state->video_scale.h.denominator = 1;
    state->video_scale.v.numerator = state->video_scale.v.denominator = 1;
}

void AFDHandle(void *context, S_FRAME_DIS_INFO *frame_info,
                    int video_numerator, int video_denominator, uint8_t afd_value, uint8_t frame_type) {
    bool changed = FALSE;
    S_VT_CONVERSION_STATE *state;
    uint8_t afd_new = afd_value & 0x0f;
    int video_aspect_ratio = 255;

    if (context == NULL) return;

    state = (S_VT_CONVERSION_STATE *)context;
    if (state->video_width != frame_info->video_width) {
        state->video_width = frame_info->video_width;
        changed = TRUE;
    }
    if (state->video_height != frame_info->video_height) {
        state->video_height = frame_info->video_height;
        changed = TRUE;
    }
    if (state->screen_width != frame_info->screen_width) {
        state->screen_width = frame_info->screen_width;
        changed = TRUE;
    }
    if (state->screen_height != frame_info->screen_height) {
        state->screen_height = frame_info->screen_height;
        changed = TRUE;
    }
    if (state->video_ratio.numerator != video_numerator
        || state->video_ratio.denominator != video_denominator) {
        state->video_ratio.numerator = video_numerator;
        state->video_ratio.denominator = video_denominator;
        changed = TRUE;
    }
    video_aspect_ratio = getAspect(video_numerator, video_denominator);
    if (state->video_aspect_ratio != video_aspect_ratio) {
        state->video_aspect_ratio = video_aspect_ratio;
        changed = TRUE;
    }
    if (state->afd != afd_new) {
        state->afd = afd_new;
        state->frame_type = frame_type;
        if (frame_type != FIELD_TOP)
            changed = TRUE;
    } else if (state->frame_type == FIELD_TOP) {
        state->frame_type = frame_type;
        changed = TRUE;
    }
    if (changed) state->settings_changed = TRUE;

    if (state->settings_changed) {
        UpdateResolution(state);
        processAfd(state);
        state->settings_changed = FALSE;
    }
}

S_RECTANGLE getInRectangle(void *context) {
    S_RECTANGLE input;
    S_VT_CONVERSION_STATE *state;
    InitRect(&input, 0, 0, 0, 0);

    if (context == NULL) return input;

    state = (S_VT_CONVERSION_STATE *)context;
    input = state->input_rectangle;

    return input;
}

S_RECTANGLE getOutRectangle(void *context) {
    S_RECTANGLE output;
    S_VT_CONVERSION_STATE *state;
    InitRect(&output, 0, 0, 0, 0);

    if (context == NULL) return output;

    state = (S_VT_CONVERSION_STATE *)context;
    output = state->output_rectangle;
    return output;
}

S_VT_FRACTION getOutRatio(void *context) {
    S_VT_FRACTION ret = {0, 0};
    S_VT_CONVERSION_STATE *state;
    if (context == NULL) return ret;

    state = (S_VT_CONVERSION_STATE *)context;
    ret = state->out_ratio;
    return ret;
}


S_RECTANGLE getScalingRect(void *context) {
    S_RECTANGLE scaling;
    S_VT_CONVERSION_STATE *state;
    InitRect(&scaling, 0, 0, 0, 0);

    if (context == NULL) return scaling;

    state = (S_VT_CONVERSION_STATE *)context;
     if (state->scaling_mode) {
        scaling = state->app_scaling_window;
    }
    return scaling;
}

int getAspect(int numerator, int denominator) {
    int aspect = ASPECT_UNDEFINED;

    //0.85 - 1.0 = 1:1
    //0.66 - 0.83 = 4:3
    //    [720x480, 1440x1080, 720x576, 702x576]
    //0.63 - 0.65 = 14:9
    //    [1680x1080, 1440x900]
    //0.54 - 0.625 = 16:9 - 16:10
    //    [1280x702, 1366x768, 1360x768, 1920x1080, 1280x768, 1280x800]
    //0.42 - 0.53 = 18:9
    //    [1680x720, 1768x800, 1360x720, 1440x720, 920x1920]
    int tmpHeight = denominator * 100;
    if (tmpHeight >= numerator * 85 && tmpHeight <= numerator * 100)
        aspect = ASPECT_RATIO_1_1;
    else if (tmpHeight >= numerator * 66 && tmpHeight < numerator * 85)
        aspect = ASPECT_RATIO_4_3;
    else if (tmpHeight >= numerator * 63 && tmpHeight < numerator * 66)
        aspect = ASPECT_RATIO_14_9;
    else if (tmpHeight >= numerator * 54 && tmpHeight < numerator * 63)
        aspect = ASPECT_RATIO_16_9;
    else if (tmpHeight >= numerator * 42 && tmpHeight < numerator * 54)
        aspect = ASPECT_RATIO_18_9;

    return aspect;
}

bool checkInScaling(void *context) {
    S_VT_CONVERSION_STATE *state = (S_VT_CONVERSION_STATE *)context;

    if (state && state->afd_enabled) {
        if (state->alignment == ASPECT_MODE_AUTO)
            return TRUE;
        if (state->scaling_mode
            && (state->app_scaling_window.width != 0)
            && (state->app_scaling_window.height != 0))
            return TRUE;
    }
    return FALSE;
}

void print_vt_state(void *context, char* buf, int count) {
    S_RECTANGLE crop;
    S_VT_FRACTION outRatio;
    S_RECTANGLE scalling_rect;
    S_VT_CONVERSION_STATE *state = (S_VT_CONVERSION_STATE *)context;

    crop = getInRectangle(state);
    outRatio = getOutRatio(state);
    scalling_rect = getScalingRect(state);
    snprintf(buf, count, "  enable : %d\n"
                         "  aspect : %d(0 auto, 5:cus)\n"
                         "  ov scan: %d\n"
                         "  value  : %d\n"
                         "  scaling: %d %d %d %d\n"
                         "  res    : %d %d\n"
                         "  video  : %d %d (%d:%d)\n"
                         "  screen : %d %d\n"
                         "  v out  : %d %d %d %d (%d:%d)\n"
                         "  dis out: %d %d %d %d\n",
                         state->afd_enabled,
                         state->alignment,
                         mOverscanMode,
                         state->afd,
                         scalling_rect.left, scalling_rect.top,
                         scalling_rect.width, scalling_rect.height,
                         state->resolution_width, state->resolution_height,
                         state->video_width, state->video_height,
                         state->video_ratio.numerator,
                         state->video_ratio.denominator,
                         state->screen_width, state->screen_height,
                         crop.left, crop.top, crop.width, crop.height,
                         outRatio.numerator, outRatio.denominator,
                         state->output_rectangle.left, state->output_rectangle.top,
                         state->output_rectangle.width, state->output_rectangle.height);
}

