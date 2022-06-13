/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */

#ifndef VTC_H
#define VTC_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NO_STDINT_H
typedef unsigned char U8BIT;
typedef unsigned short U16BIT;
typedef signed char S8BIT;
typedef signed short S16BIT;
typedef unsigned long U32BIT;
typedef signed long S32BIT;
typedef unsigned long long U64BIT;
typedef bool BOOLEAN;
#else
typedef uint8_t U8BIT;
typedef int8_t S8BIT;
typedef uint16_t U16BIT;
typedef int16_t S16BIT;
typedef uint32_t U32BIT;
typedef int32_t S32BIT;
typedef uint64_t U64BIT;
typedef bool BOOLEAN;
#endif

typedef struct s_vt_options {
    BOOLEAN mheg_required;
    BOOLEAN afd_required;
    BOOLEAN hbbtv_required;
    U8BIT path;
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
    FORMAT_CONVERSION_CENTRE_4_3
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

/*typedef enum
{
    SD_WIDTH,
    SD_HEIGHT,
    HD_WIDTH,
    HD_HEIGHT,
} E_VIDEO_PARM;//undeclared value,temporary addition-chenfei.dou
*/

typedef struct {
    U8BIT afd;
    E_ASPECT_RATIO video_aspect_ratio;
    E_ASPECT_RATIO display_aspect_ratio;
    E_AFD_PREFERENCE afd_preference;
    E_FORMAT_CONVERSION transformation;
} S_AFD_TRANS;

typedef struct s_fraction {
    S32BIT numerator;
    S32BIT denominator;
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
    S32BIT left;
    S32BIT top;
    U32BIT width;
    U32BIT height;
} S_RECTANGLE;

typedef struct {
    U32BIT video_width;
    U32BIT video_height;
    U32BIT screen_width;
    U32BIT screen_height;
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
    BOOLEAN afd_enabled;
    E_APP_SCALING_TYPE scaling_mode;
    BOOLEAN mheg_scaling_given;
    BOOLEAN settings_changed;
    U16BIT mheg_resolution_width;
    U16BIT mheg_resolution_height;
    U16BIT resolution_width;
    U16BIT resolution_height;
    U16BIT video_width;
    U16BIT video_height;
    U16BIT screen_width;
    U16BIT screen_height;
    E_ASPECT_RATIO video_aspect_ratio;
    E_ASPECT_RATIO display_aspect_ratio;
    E_STB_AV_DECODER_STATUS decoder_status;
    U8BIT afd;
    U8BIT wss;
    S_RECTANGLE input_rectangle, output_rectangle;
} S_VT_CONVERSION_STATE;

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
 * @parm     U8BIT afd_value
 ****************************************************************************/
void AFDHandle(void *context, S_FRAME_DIS_INFO *frame_info,
               E_ASPECT_RATIO frame_aspectratio, U8BIT afd_value);
/*!**************************************************************************
 * @brief    get input_rectangle /sys/class/video/crop
 * @param    context - transformation calculator context
 ****************************************************************************/
S_RECTANGLE getInrectangle(void *context);
/*!**************************************************************************
 * @brief    get out_rectangle /sys/class/video/axis
 * @param    context - transformation calculator context
 ****************************************************************************/
S_RECTANGLE getOutrectangle(void *context);
/*!**************************************************************************
 * @brief    get current scaling rectangle
 * @param    context - transformation calculator context
 ****************************************************************************/
S_RECTANGLE getScalingRect(void *context);
/*!**************************************************************************
 * @brief    check scaling result for vpp
 * @param    context - transformation calculator context
 ****************************************************************************/
BOOLEAN checkInScaling(void *context);
void print_vt_state(void *context, char* buf, int count);


#ifdef __cplusplus
}
#endif

#endif /*VTC_H*/
