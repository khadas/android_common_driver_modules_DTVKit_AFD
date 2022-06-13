/*
 *
 * Copyright (c) 2015 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 *
 */


/*******************
 * INCLUDE FILES    *
 ********************/
#include <linux/kernel.h>
#include "vtc.h"

#ifndef NULL
#include <stddef.h>
#endif
/*******************
 * LOCAL MACROS     *
 ********************/

/*#define PRINT_STATE*/

#define VT_DBG(x, ...)
#define DBG(x)

#define TRUE true
#define FALSE false
#define ASSERT(x)

#define SD_WIDTH 720
#define SD_HEIGHT 576
#define HD_WIDTH 1280
#define HD_HEIGHT 720
#define FHD_WIDTH 1920
#define FHD_HEIGHT 1080

/*******************
 * STATIC DATA      *
 ********************/
static const S_AFD_TRANS afd_table[] = {
    {0, ASPECT_RATIO_4_3, ASPECT_RATIO_16_9, AFD_PREFERENCE_AUTO,
     FORMAT_CONVERSION_PILLAR_BOX},
    {0, ASPECT_RATIO_16_9, ASPECT_RATIO_4_3, AFD_PREFERENCE_16_9_LB,
     FORMAT_CONVERSION_LETTERBOX},
    {0, ASPECT_RATIO_16_9, ASPECT_RATIO_4_3, AFD_PREFERENCE_14_9_LB,
     FORMAT_CONVERSION_LETTERBOX_14_9},
    {0, ASPECT_RATIO_16_9, ASPECT_RATIO_4_3, AFD_PREFERENCE_CCO,
     FORMAT_CONVERSION_PANSCAN},
    {0, ASPECT_RATIO_16_9, ASPECT_RATIO_4_3, AFD_PREFERENCE_AUTO,
     FORMAT_CONVERSION_LETTERBOX},

    {1, ASPECT_RATIO_4_3, ASPECT_RATIO_16_9, AFD_PREFERENCE_AUTO,
     FORMAT_CONVERSION_PILLAR_BOX},
    {1, ASPECT_RATIO_16_9, ASPECT_RATIO_4_3, AFD_PREFERENCE_AUTO,
     FORMAT_CONVERSION_PANSCAN},

    {2, ASPECT_RATIO_4_3, ASPECT_RATIO_16_9, AFD_PREFERENCE_AUTO,
     FORMAT_CONVERSION_ZOOM_4_3},
    {2, ASPECT_RATIO_16_9, ASPECT_RATIO_4_3, AFD_PREFERENCE_AUTO,
     FORMAT_CONVERSION_LETTERBOX},

    {3, ASPECT_RATIO_4_3, ASPECT_RATIO_16_9, AFD_PREFERENCE_AUTO,
     FORMAT_CONVERSION_ZOOM_14_9},
    {3, ASPECT_RATIO_16_9, ASPECT_RATIO_4_3, AFD_PREFERENCE_16_9_LB,
     FORMAT_CONVERSION_LETTERBOX_14_9},
    {3, ASPECT_RATIO_16_9, ASPECT_RATIO_4_3, AFD_PREFERENCE_CCO,
     FORMAT_CONVERSION_PANSCAN},
    {3, ASPECT_RATIO_16_9, ASPECT_RATIO_4_3, AFD_PREFERENCE_AUTO,
     FORMAT_CONVERSION_LETTERBOX_14_9},

    {5, ASPECT_RATIO_4_3, ASPECT_RATIO_16_9, AFD_PREFERENCE_AUTO,
     FORMAT_CONVERSION_ZOOM_14_9},
    {5, ASPECT_RATIO_16_9, ASPECT_RATIO_4_3, AFD_PREFERENCE_AUTO,
     FORMAT_CONVERSION_PANSCAN},
    {5, ASPECT_RATIO_16_9, ASPECT_RATIO_16_9, AFD_PREFERENCE_AUTO,
     FORMAT_CONVERSION_PANSCAN_14_9},

    {6, ASPECT_RATIO_4_3, ASPECT_RATIO_16_9, AFD_PREFERENCE_AUTO,
     FORMAT_CONVERSION_ZOOM_4_3},
    {6, ASPECT_RATIO_4_3, ASPECT_RATIO_4_3, AFD_PREFERENCE_AUTO,
     FORMAT_CONVERSION_CENTRE_14_9},
    {6, ASPECT_RATIO_16_9, ASPECT_RATIO_4_3, AFD_PREFERENCE_16_9_LB,
     FORMAT_CONVERSION_LETTERBOX},
    {6, ASPECT_RATIO_16_9, ASPECT_RATIO_4_3, AFD_PREFERENCE_14_9_LB,
     FORMAT_CONVERSION_LETTERBOX_14_9},
    {6, ASPECT_RATIO_16_9, ASPECT_RATIO_4_3, AFD_PREFERENCE_CCO,
     FORMAT_CONVERSION_PANSCAN},
    {6, ASPECT_RATIO_16_9, ASPECT_RATIO_4_3, AFD_PREFERENCE_AUTO,
     FORMAT_CONVERSION_LETTERBOX_14_9},

    {7, ASPECT_RATIO_4_3, ASPECT_RATIO_16_9, AFD_PREFERENCE_AUTO,
     FORMAT_CONVERSION_ZOOM_4_3},
    {7, ASPECT_RATIO_4_3, ASPECT_RATIO_4_3, AFD_PREFERENCE_AUTO,
     FORMAT_CONVERSION_CENTRE_4_3},
    {7, ASPECT_RATIO_16_9, ASPECT_RATIO_4_3, AFD_PREFERENCE_16_9_LB,
     FORMAT_CONVERSION_LETTERBOX},
    {7, ASPECT_RATIO_16_9, ASPECT_RATIO_4_3, AFD_PREFERENCE_14_9_LB,
     FORMAT_CONVERSION_LETTERBOX_14_9},
    {7, ASPECT_RATIO_16_9, ASPECT_RATIO_4_3, AFD_PREFERENCE_CCO,
     FORMAT_CONVERSION_PANSCAN},
    {7, ASPECT_RATIO_16_9, ASPECT_RATIO_4_3, AFD_PREFERENCE_AUTO,
     FORMAT_CONVERSION_PANSCAN}};

/**********************
 * FUNCTION PROTOTYPES *
 ***********************/
static void Recalculate(S_VT_CONVERSION_STATE *state);
static E_MHEG_SCALING MhegScalingType(S_VT_CONVERSION_STATE *state);
static BOOLEAN RectanglesDiffer(S_RECTANGLE *a, S_RECTANGLE *b);
static void CalculateMhegScaling(S_VT_CONVERSION_STATE *state);
static void InitRect(S_RECTANGLE *rect, S32BIT left, S32BIT top, S32BIT width,
                     S32BIT height);
static U8BIT GetWss(S_VT_CONVERSION_STATE *state);
static E_FORMAT_CONVERSION GetVideoTransformation(S_VT_CONVERSION_STATE *state,
                                                  S_VT_MATRIX *transform,
                                                  S_VT_MATRIX *clip_transform);
static E_FORMAT_CONVERSION GetIframeTransformation(S_VT_CONVERSION_STATE *state,
                                                   S_VT_MATRIX *transform,
                                                   S_VT_MATRIX *clip_transform);
#ifdef PRINT_STATE
static void PrintState(S_VT_CONVERSION_STATE *state);
#endif

static void ClipRectangle(S_VT_FRACT_RECT *clip_rect, S_VT_FRACT_RECT *rect);
static void MultiplyMatrices(S_VT_MATRIX *matrix_a, S_VT_MATRIX *matrix_b,
                             S_VT_MATRIX *output_matrix);
static void InvertMatrix(S_VT_MATRIX *matrix, S_VT_MATRIX *inverse);
static BOOLEAN ReduceFraction(S_VT_FRACTION *input, S_VT_FRACTION *output);

static void AddFractions(S_VT_FRACTION *in_a, S_VT_FRACTION *in_b,
                         S_VT_FRACTION *out);
static void SubtractFraction(S_VT_FRACTION *in_a, S_VT_FRACTION *in_b,
                             S_VT_FRACTION *out);
static int CompareFractions(S_VT_FRACTION *in_a, S_VT_FRACTION *in_b);

static E_FORMAT_CONVERSION FindAfdTransformation(S_VT_CONVERSION_STATE *state);

static void ApplyTransformation(S_VT_MATRIX *matrix, S_VT_FRACT_RECT *input,
                                S_VT_FRACT_RECT *output);

static void VTC_IdentityMatrix(S_VT_MATRIX *matrix);

static void VTC_VideoCsScaling(S_VT_CONVERSION_STATE *state,
                               S_VT_MATRIX *current);

static E_FORMAT_CONVERSION VTC_UserPreferenceScaling(
    S_VT_CONVERSION_STATE *state, S_VT_MATRIX *current);

static E_FORMAT_CONVERSION VTC_AfdScaling(S_VT_CONVERSION_STATE *state,
                                          S_VT_MATRIX *current);

static void VTC_MhegScaling(S_VT_CONVERSION_STATE *state, S_VT_MATRIX *current);

static void VTC_HbbScaleToScreen(S_VT_CONVERSION_STATE *state,
                                 S_VT_MATRIX *current);

static void VTC_AppScaling(S_VT_CONVERSION_STATE *state, S_VT_MATRIX *current);

static E_FORMAT_CONVERSION VTC_QuarterScreenScaling(
    S_VT_CONVERSION_STATE *state, S_VT_MATRIX *current);

static void VTC_SceneArScaling(S_VT_CONVERSION_STATE *state,
                               S_VT_MATRIX *current);

static void VTC_ToScreen(S_VT_CONVERSION_STATE *state, S_VT_MATRIX *current);

static E_FORMAT_CONVERSION VTC_AppScalingTransform(S_VT_CONVERSION_STATE *state,
                                                   S_VT_MATRIX *current);

static void VTC_ApplyWSS(S_VT_CONVERSION_STATE *state, S_VT_FRACT_RECT *output);

static void VTC_TransformRectangles(S_VT_CONVERSION_STATE *state,
                                    S_VT_MATRIX *transform,
                                    S_VT_MATRIX *clip_transform,
                                    S_VT_FRACT_RECT *input_rect,
                                    S_VT_FRACT_RECT *output_rect);

static void VTC_FractRectangleToRectangle(S_VT_FRACT_RECT *fract_rect,
                                          S_RECTANGLE *rectangle);

static void VTC_RectangleToFractRectangle(S_RECTANGLE *rectangle,
                                          S_VT_FRACT_RECT *fract_rect);

static void PrintMatrix(S_VT_MATRIX *matrix);

static void PrintFractRect(S_VT_FRACT_RECT *fract_rect);

/*Scaling functions*/
/*video resolution -> MHEG coordinate space*/
static void Scale_352_288_Video(S_VT_CONVERSION_STATE *state,
                                S_VT_MATRIX *transform);

static void Scale_352_576_Video(S_VT_CONVERSION_STATE *state,
                                S_VT_MATRIX *transform);

static void Scale_480_576_Video(S_VT_CONVERSION_STATE *state,
                                S_VT_MATRIX *transform);

static void Scale_544_576_Video(S_VT_CONVERSION_STATE *state,
                                S_VT_MATRIX *transform);

static void Scale_Video(S_VT_CONVERSION_STATE *state, S_VT_MATRIX *transform);

/* presentation coordinate space -> screen resolution*/
static void Scale_ToScreen(S_VT_CONVERSION_STATE *state,
                           S_VT_MATRIX *transform);

/* hbbtv coordinate space -> screen resolution*/
static void Scale_HbbToScreen(S_VT_CONVERSION_STATE *state,
                              S_VT_MATRIX *transform);

/*AFDs*/
static void Scale_Pillarbox(S_VT_CONVERSION_STATE *state, S32BIT width,
                            S32BIT height, S_VT_MATRIX *transform);

static void Scale_4_3_Zoom(S_VT_CONVERSION_STATE *state, S32BIT width,
                           S32BIT height, S_VT_MATRIX *transform);

static void Scale_14_9_Zoom(S_VT_CONVERSION_STATE *state, S32BIT width,
                            S32BIT height, S_VT_MATRIX *transform);

static void Scale_14_9_Centre(S_VT_CONVERSION_STATE *state, S32BIT width,
                              S32BIT height, S_VT_MATRIX *transform);

static void Scale_4_3_Centre(S_VT_CONVERSION_STATE *state, S32BIT width,
                             S32BIT height, S_VT_MATRIX *transform);

static void Scale_16_9_Letterbox(S_VT_CONVERSION_STATE *state, S32BIT width,
                                 S32BIT height, S_VT_MATRIX *transform);

static void Scale_14_9_Letterbox(S_VT_CONVERSION_STATE *state, S32BIT width,
                                 S32BIT height, S_VT_MATRIX *transform);

static void Scale_CentreCutOut(S_VT_CONVERSION_STATE *state, S32BIT width,
                               S32BIT height, S_VT_MATRIX *transform);

static void Scale_16_9_Zoom(S_VT_CONVERSION_STATE *state, S32BIT width,
                            S32BIT height, S_VT_MATRIX *transform);

static void UpdateResolution(S_VT_CONVERSION_STATE *state);

/* MHEG scaling and positioning */
static void MhegScaling(S_VT_CONVERSION_STATE *state, S_VT_MATRIX *transform);

static void HbbScaling(S_VT_CONVERSION_STATE *state, S_VT_MATRIX *transform);

/* Application scaling and positioning */
static void AppScaling(S_VT_CONVERSION_STATE *state, S_VT_MATRIX *transform);

/*1:1 scaling, for use in some default cases*/
static void NoScaling(S_VT_CONVERSION_STATE *state, S_VT_MATRIX *transform);

/* Create a new fraction */
static S_VT_FRACTION MakeFraction(S32BIT numerator, S32BIT denominator);

static S_VT_FRACT_RECT MakeRectangle(S32BIT left, S32BIT top, S32BIT width,
                                     S32BIT height);

/***********************
 * FUNCTION DEFINITIONS *
 ************************/

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

void VT_Rest(S_VT_CONVERSION_STATE* vtc) {
    S_VT_CONVERSION_STATE *new_state = vtc;

    if (new_state != NULL) {
        new_state->afd_enabled = FALSE;
        new_state->scaling_mode = SCALING_NONE;
        new_state->mheg_scaling_given = FALSE;

        new_state->afd_preference = AFD_PREFERENCE_AUTO;
        new_state->alignment = ASPECT_MODE_AUTO;
        new_state->afd = 0;//default is 0 for aspect auto
        new_state->mheg_aspect_ratio = ASPECT_UNDEFINED;
        new_state->mheg_wam = ASPECT_MODE_4_3;

        InitRect(&new_state->mheg_scaling_rect, 0, 0, SD_WIDTH, SD_HEIGHT);
        new_state->mheg_resolution_width = SD_WIDTH;
        new_state->mheg_resolution_height = SD_HEIGHT;

        InitRect(&new_state->app_scaling_window, 0, 0, SD_WIDTH, SD_HEIGHT);

        InitRect(&new_state->hbb_window_rect, 0, 0, HD_WIDTH, HD_HEIGHT);

        new_state->resolution_width = SD_WIDTH;
        new_state->resolution_height = SD_HEIGHT;

        new_state->video_width = SD_WIDTH;
        new_state->video_height = SD_HEIGHT;

        new_state->screen_width = SD_WIDTH;
        new_state->screen_height = SD_HEIGHT;

        new_state->video_aspect_ratio = ASPECT_RATIO_4_3;
        new_state->display_aspect_ratio = ASPECT_RATIO_16_9;

        new_state->wss = 0;

        new_state->decoder_status = DECODER_STATUS_VIDEO;

        new_state->settings_changed = FALSE;

        InitRect(&new_state->input_rectangle, 0, 0, SD_WIDTH, SD_HEIGHT);
        InitRect(&new_state->output_rectangle, 0, 0, SD_WIDTH, SD_HEIGHT);
    }
}

/*!**************************************************************************
 * @brief    Set user preference for video aspect ratio alignment
 *
 * @param    context - transformation calculator context
 * @param    alignment - New video alignment preference:
 ****************************************************************************/
void VT_SetVideoAlignmentPref(void *context, E_VIDEO_ASPECT_MODE alignment) {
    S_VT_CONVERSION_STATE *state;

    ASSERT(context != NULL);
    if (context == NULL) return;

    state = (S_VT_CONVERSION_STATE *)context;

    if (state->alignment != alignment) {
        if (state->display_aspect_ratio == ASPECT_RATIO_4_3) {
            switch (alignment) {
                default:
                case ASPECT_MODE_16_9:
                    state->afd_preference = AFD_PREFERENCE_16_9_LB;
                    break;

                case ASPECT_MODE_ZOOM:
                case ASPECT_MODE_4_3:
                    state->afd_preference = AFD_PREFERENCE_CCO;
                    break;

                case ASPECT_MODE_14_9:
                    state->afd_preference = AFD_PREFERENCE_14_9_LB;
                    break;
            }
        } else {
            state->afd_preference = AFD_PREFERENCE_AUTO;
        }

        state->alignment = alignment;
        state->settings_changed = TRUE;
    }
}

/*!**************************************************************************
 * @brief    Set MHEG-5 scaling information
 * @param    context - transformation calculator context
 * @param    scaling - scaling and positioning transformation
 * @Param    resolution_width
 * @Param    resolution_height
 * @note     When scaling is NULL, scaling is ignored and the behaviour will
 *           be as if full screen video is mapped to the full screen.
 ****************************************************************************/
void VT_SetMhegScaling(void *context, S_RECTANGLE *scaling, int resolution_width, int resolution_height) {
    S_VT_CONVERSION_STATE *state;

    ASSERT(context != NULL);
    if (context == NULL) return;

    state = (S_VT_CONVERSION_STATE *)context;

    state->scaling_mode = SCALING_MHEG;
    state->mheg_scaling_given = TRUE;
    state->mheg_scaling_rect = *scaling;
    state->mheg_resolution_width = resolution_width;
    state->mheg_resolution_height = resolution_height;

    state->settings_changed = TRUE;
}

/*!**************************************************************************
 * @brief    Set application scaling information
 * @param    context - transformation calculator context
 * @param    window - output window (screen CS)
 * @Param    resolution_width
 * @Param    resolution_height
 * @note     When window is NULL, application scaling is turned off
 ****************************************************************************/
void VT_SetAppScaling(void *context, S_RECTANGLE *window, int resolution_width, int resolution_height) {
    S_VT_CONVERSION_STATE *state;

    ASSERT(context != NULL);
    if (context == NULL) return;

    state = (S_VT_CONVERSION_STATE *)context;

    if (window == NULL) {
        if (state->scaling_mode == SCALING_APP) {
            state->scaling_mode = SCALING_NONE;
            state->settings_changed = TRUE;
        }
    } else {
        state->scaling_mode = SCALING_APP;
        state->app_scaling_window = *window;
        state->resolution_width = resolution_width;
        state->resolution_height = resolution_height;
        state->settings_changed = TRUE;
    }
}

/*!**************************************************************************
 * @brief    Find the type of scaling specified by the MHEG application, if any
 *
 * @param    state - transformation calculator state
 * @return   MHEG_SCALING_QUARTER if video should be quarter-screen,
 *           MHEG_SCALING_NONE if video is not scaled
 *           MHEG_SCALING_OTHER for any other scaling
 ****************************************************************************/
static E_MHEG_SCALING MhegScalingType(S_VT_CONVERSION_STATE *state) {
    S_RECTANGLE *scaling = &state->mheg_scaling_rect;

    if ((scaling->width == state->mheg_resolution_width / 2) &&
        (scaling->height == state->mheg_resolution_height / 2)) {
        return MHEG_SCALING_QUARTER;
    } else if ((scaling->width == state->mheg_resolution_width) &&
               (scaling->height == state->mheg_resolution_height)) {
        if ((scaling->top == 0) && (scaling->left == 0)) {
            return MHEG_SCALING_NONE;
        } else {
            return MHEG_SCALING_OFFSET;
        }
    } else {
        return MHEG_SCALING_OTHER;
    }
}

/*!**************************************************************************
 * @fn
 * @brief
 *
 * @param
 *
 *
 *
 * @return
 *
 *
 * @warning
 * @bug
 ****************************************************************************/
static E_FORMAT_CONVERSION AfdOrUserPreferenceTransform(
    S_VT_CONVERSION_STATE *state, S_VT_MATRIX *current) {
    E_FORMAT_CONVERSION transform = FORMAT_CONVERSION_IGNORE;

    if (state->alignment == ASPECT_MODE_AUTO) {
        transform = VTC_AfdScaling(state, current);
    } else {
        transform = VTC_UserPreferenceScaling(state, current);
    }

    return transform;
}

/*!**************************************************************************
 * @brief    Recalculate video transformation
 * @param    state - current state of the system
 ****************************************************************************/
static void Recalculate(S_VT_CONVERSION_STATE *state) {
    S_VT_MATRIX transform, clip_transform;
    U8BIT temp_wss;
    S_RECTANGLE input_rectangle, output_rectangle;
    S_VT_FRACT_RECT input, output;

    CalculateMhegScaling(state);

#ifdef PRINT_STATE
    PrintState(state);
#endif

    input_rectangle.top = 0;
    input_rectangle.left = 0;
    input_rectangle.width = state->video_width;
    input_rectangle.height = state->video_height;
    VT_DBG("input_rectangle(%d, %d)", input_rectangle.height,
           input_rectangle.width);

    temp_wss = GetWss(state);

    VTC_IdentityMatrix(&transform);
    VTC_IdentityMatrix(&clip_transform);

    if (state->decoder_status == DECODER_STATUS_VIDEO) {
        GetVideoTransformation(state, &transform, &clip_transform);
    } else if (state->decoder_status == DECODER_STATUS_IFRAME) {
        GetIframeTransformation(state, &transform, &clip_transform);
    }

    VTC_RectangleToFractRectangle(&input_rectangle, &input);

    /* Apply the transformation on input/output rectangles */
    DBG(("Clip transform:\n"));
    PrintMatrix(&clip_transform);

    VTC_TransformRectangles(state, &transform, &clip_transform, &input,
                            &output);

    /* Apply "WSS" transformation, if required */
    VTC_ApplyWSS(state, &output);

    VTC_FractRectangleToRectangle(&input, &input_rectangle);
    VTC_FractRectangleToRectangle(&output, &output_rectangle);

    state->input_rectangle = input_rectangle;
    state->output_rectangle = output_rectangle;
    state->wss = temp_wss;

    state->settings_changed = FALSE;
}

/*!**************************************************************************
 * @brief    Calculate the transformation for the video using the data set in
 *           the state
 *
 * @param    state - current state of the system
 * @param    transform - The tranformation matrix
 * @param    clip_transform - Transformation matrix for clipping region
 * @return   The transformation type that was chosen
 ****************************************************************************/
static E_FORMAT_CONVERSION GetVideoTransformation(S_VT_CONVERSION_STATE *state,
                                                  S_VT_MATRIX *transform,
                                                  S_VT_MATRIX *clip_transform) {
    E_FORMAT_CONVERSION transform_type;

    transform_type = FORMAT_CONVERSION_IGNORE;

    DBG(("Initial transformation:\n"));
    PrintMatrix(transform);

    VTC_VideoCsScaling(state, transform);
    DBG(("VTC_VideoCsScaling:\n"));
    PrintMatrix(transform);

    if (state->scaling_mode == SCALING_APP) {
        /* Apply application scaling transformation */
        transform_type = AfdOrUserPreferenceTransform(state, transform);
        DBG(("AppScalingTransform:\n"));
        PrintMatrix(transform);

        VTC_AppScaling(state, transform);
        VTC_AppScaling(state, clip_transform);
        DBG(("VTC_AppScaling:\n"));
        PrintMatrix(transform);
        VTC_ToScreen(state, transform);
        VTC_ToScreen(state, clip_transform);
    } else if (state->scaling_mode == SCALING_HBBTV) {
        VTC_HbbScaleToScreen(state, transform);
        VTC_HbbScaleToScreen(state, clip_transform);
        DBG(("VTC_HbbScaleToScreen:\n"));
        PrintMatrix(transform);
    } else {
        if (state->scaling_mode == SCALING_MHEG) {
            if (MhegScalingType(state) == MHEG_SCALING_QUARTER) {
                transform_type = VTC_QuarterScreenScaling(state, transform);
            } else if (state->mheg_aspect_ratio == ASPECT_UNDEFINED) {
                if ((MhegScalingType(state) == MHEG_SCALING_NONE) ||
                    (MhegScalingType(state) == MHEG_SCALING_OFFSET)) {
                    transform_type =
                        AfdOrUserPreferenceTransform(state, transform);
                    DBG(("AfdOrUserPreferenceTransform:\n"));
                    PrintMatrix(transform);
                }
            } else {
                VTC_SceneArScaling(state, transform);
                DBG(("VTC_SceneArScaling:\n"));
                PrintMatrix(transform);
            }

            VTC_MhegScaling(state, transform);
            VTC_MhegScaling(state, clip_transform);
            DBG(("VTC_MhegScaling:\n"));
            PrintMatrix(transform);
        } else {
            transform_type = AfdOrUserPreferenceTransform(state, transform);
            DBG(("AfdOrUserPreferenceTransform:\n"));
            PrintMatrix(transform);
        }

        /* Transform from presentation coordinates back to screen coordinates */
        VTC_ToScreen(state, transform);
        VTC_ToScreen(state, clip_transform);
        DBG(("VTC_ToScreen:\n"));
        PrintMatrix(transform);
    }

    return transform_type;
}

/*!**************************************************************************
 * @brief    Calculate the transformation for the I-Frame using the data set
 *           in the state
 *
 * @param    state - current state of the system
 * @param    transform - The tranformation matrix
 * @param    clip_transform - Transformation matrix for clipping region
 * @return   The transformation type that was chosen
 ****************************************************************************/
static E_FORMAT_CONVERSION GetIframeTransformation(
    S_VT_CONVERSION_STATE *state, S_VT_MATRIX *transform,
    S_VT_MATRIX *clip_transform) {
    E_FORMAT_CONVERSION transform_type;

    transform_type = FORMAT_CONVERSION_IGNORE;

    DBG(("Initial transformation:\n"));
    PrintMatrix(transform);

    VTC_VideoCsScaling(state, transform);
    DBG(("VTC_VideoCsScaling:\n"));
    PrintMatrix(transform);

    if (state->scaling_mode == SCALING_MHEG) {
        VTC_MhegScaling(state, transform);
        DBG(("VTC_MhegScaling:\n"));
        PrintMatrix(transform);
    }

    /* Compose a transformation from the presentation coordinates to screen
       coordinates with the current transformation */
    VTC_ToScreen(state, transform);
    VTC_ToScreen(state, clip_transform);
    DBG(("VTC_ToScreen:\n"));
    PrintMatrix(transform);

    return transform_type;
}

/*!**************************************************************************
 * @brief    Calculate MHEG-5 scaling transformation given input and output
 * @param    state - conversion state
 ****************************************************************************/
static void CalculateMhegScaling(S_VT_CONVERSION_STATE *state) {
    if (!state->mheg_scaling_given) {
        InitRect(&state->mheg_scaling_rect, 0, 0, state->mheg_resolution_width,
                 state->mheg_resolution_height);
    }
}

/*!**************************************************************************
 * @brief    Check if two rectangles are different
 *
 * @param    a - first rectange
 * @param    b - second rectangle
 * @return   TRUE if rectangles are different, FALSE if they are the same
 ****************************************************************************/
static BOOLEAN RectanglesDiffer(S_RECTANGLE *a, S_RECTANGLE *b) {
    if (a->left != b->left) {
        return TRUE;
    }

    if (a->top != b->top) {
        return TRUE;
    }

    if (a->width != b->width) {
        return TRUE;
    }

    if (a->height != b->height) {
        return TRUE;
    }

    return FALSE;
}

/*!**************************************************************************
 * @brief    Initialise rectangle
 * @param    rect - rectangle to initialise
 * @param    left - left position
 * @param    top - top position
 * @param    width - rectangle width
 * @param    height - rectangle height
 ****************************************************************************/
static void InitRect(S_RECTANGLE *rect, S32BIT left, S32BIT top, S32BIT width,
                     S32BIT height) {
    rect->left = left;
    rect->top = top;
    rect->width = width;
    rect->height = height;
}

/*!**************************************************************************
 * @brief    Return WSS
 * @param    state - conversion state
 * @return   The calculated WSS value
 ****************************************************************************/
static U8BIT GetWss(S_VT_CONVERSION_STATE *state) {
    if (state->video_aspect_ratio == ASPECT_RATIO_4_3) {
        switch (state->afd) {
            case 0:
            case 1:
            case 3:
                return 0x1;
                break;

            case 2:
                return 0xd;
                break;

            case 5:
                return 0x7;
                break;

            case 6:
                return 0xd;
                break;

            case 7:

                break;
        }
    } else {
        if (state->display_aspect_ratio == ASPECT_RATIO_4_3) {
            switch (state->afd) {
                case 0:
                    switch (state->afd_preference) {
                        case AFD_PREFERENCE_AUTO:
                        case AFD_PREFERENCE_16_9_LB:
                            return 0xd;
                            break;

                        case AFD_PREFERENCE_14_9_LB:
                            return 0x8;
                            break;

                        case AFD_PREFERENCE_CCO:
                            return 0x1;
                            break;
                    }

                    break;

                case 1:
                    return 0x1;

                    break;

                case 2:

                    return 0xd;

                    break;

                case 3:

                    if (state->afd_preference == AFD_PREFERENCE_CCO) {
                        return 0x1;
                    } else {
                        return 0x8;
                    }

                    break;

                case 5:
                    return 0x7;
                    break;

                case 6:
                case 7:
                    if (state->afd_preference == AFD_PREFERENCE_CCO) {
                        return 0x1;
                    } else if (state->afd_preference ==
                               AFD_PREFERENCE_14_9_LB) {
                        return 0x8;
                    } else {
                        return 0xd;
                    }

                    break;
            }
        } else {
            return 0xe;
        }
    }

    return 1;
}

#ifdef PRINT_STATE
/*!**************************************************************************
 * @brief    Print the state
 * @param    state - conversion state
 ****************************************************************************/
static void PrintState(S_VT_CONVERSION_STATE *state) {
    DBG(("Conversion state:\n"));
    DBG(("  mheg_enabled               : %s\n",
         state->mheg_enabled ? "TRUE" : "FALSE"));
    DBG(("  afd_enabled                : %s\n",
         state->afd_enabled ? "TRUE" : "FALSE"));
    DBG(("  hbb_enabled                : %s\n",
         state->hbb_enabled ? "TRUE" : "FALSE"));
    DBG(("  afd_preference             : %s\n",
         state->afd_preference == AFD_PREFERENCE_AUTO ? "AFD_PREFERENCE_AUTO"
         : state->afd_preference == AFD_PREFERENCE_16_9_LB
             ? "AFD_PREFERENCE_16_9_LB"
         : state->afd_preference == AFD_PREFERENCE_14_9_LB
             ? "AFD_PREFERENCE_14_9_LB"
         : state->afd_preference == AFD_PREFERENCE_CCO ? "AFD_PREFERENCE_CCO"
                                                       : "UNKNOWN"));
    DBG(("  alignment                : %s\n",
         state->alignment == ASPECT_MODE_4_3      ? "ASPECT_MODE_4_3"
         : state->alignment == ASPECT_MODE_14_9   ? "ASPECT_MODE_14_9"
         : state->alignment == ASPECT_MODE_16_9   ? "ASPECT_MODE_16_9"
         : state->alignment == ASPECT_MODE_AUTO   ? "ASPECT_MODE_AUTO"
         : state->alignment == ASPECT_MODE_CUSTOM ? "ASPECT_MODE_CUSTOM"
                                                  : "UNKNOWN"));
    DBG(("  afd                        : %d\n", state->afd));
    DBG(("  mheg_aspect_ratio         : %s\n",
         state->mheg_aspect_ratio == ASPECT_RATIO_4_3    ? "ASPECT_RATIO_4_3"
         : state->mheg_aspect_ratio == ASPECT_RATIO_16_9 ? "ASPECT_RATIO_16_9"
         : state->mheg_aspect_ratio == ASPECT_UNDEFINED  ? "ASPECT_UNSPECIFIED"
                                                         : "UNKNOWN"));
    DBG(("  wam                        : %s\n",
         state->mheg_wam == ASPECT_MODE_AUTO   ? "ALIGNMENT_MODE_NONE"
         : state->mheg_wam == ASPECT_MODE_4_3  ? "ALIGNMENT_MODE_CCO"
         : state->mheg_wam == ASPECT_MODE_16_9 ? "ALIGNMENT_MODE_LB"
                                               : "UNKNOWN"));
    DBG(("  mheg_scaling_rect          : { l=%ld, t=%ld, w=%ld, h=%ld }\n",
         state->mheg_scaling_rect.left, state->mheg_scaling_rect.top,
         state->mheg_scaling_rect.width, state->mheg_scaling_rect.height));
    DBG(("  hbb_window_rect            : { l=%ld, t=%ld, w=%ld, h=%ld }\n",
         state->hbb_window_rect.left, state->hbb_window_rect.top,
         state->hbb_window_rect.width, state->hbb_window_rect.height));
    DBG(("  mheg_scaling_given         : %s\n",
         state->mheg_scaling_given ? "TRUE" : "FALSE"));
    DBG(("  mheg_resolution            : { w=%d, h=%d}\n",
         state->mheg_resolution_width, state->mheg_resolution_height));
    DBG(("  resolution            : { w=%d, h=%d}\n", state->resolution_width,
         state->resolution_height));
    DBG(("  video_width                : %d\n", state->video_width));
    DBG(("  video_height               : %d\n", state->video_height));
    DBG(("  screen_width               : %d\n", state->screen_width));
    DBG(("  screen_height              : %d\n", state->screen_height));
    DBG(("  video_aspect_ratio         : %s\n",
         state->video_aspect_ratio == ASPECT_RATIO_4_3    ? "ASPECT_RATIO_4_3"
         : state->video_aspect_ratio == ASPECT_RATIO_16_9 ? "ASPECT_RATIO_16_9"
         : state->video_aspect_ratio == ASPECT_UNDEFINED  ? "ASPECT_UNDEFINED"
                                                          : "UNKNOWN"));
    DBG(("  display_aspect_ratio       : %s\n",
         state->display_aspect_ratio == ASPECT_RATIO_4_3 ? "ASPECT_RATIO_4_3"
         : state->display_aspect_ratio == ASPECT_RATIO_16_9
             ? "ASPECT_RATIO_16_9"
         : state->display_aspect_ratio == ASPECT_UNDEFINED ? "ASPECT_UNDEFINED"
                                                           : "UNKNOWN"));
}

#endif

static void PrintMatrix(S_VT_MATRIX *matrix) {
    if (matrix) {
        DBG(("matrix: A:%ld/%ld B:%ld/%ld C:%ld/%ld D:%ld/%ld\n",
             matrix->a.numerator, matrix->a.denominator, matrix->b.numerator,
             matrix->b.denominator, matrix->c.numerator, matrix->c.denominator,
             matrix->d.numerator, matrix->d.denominator));
    }
}

static void PrintFractRect(S_VT_FRACT_RECT *fract_rect) {
    if (fract_rect) {
        DBG(
            ("fract_rect: Width:%ld/%ld Height:%ld/%ld Left:%ld/%ld "
             "Top:%ld/%ld\n",
             fract_rect->width.numerator, fract_rect->width.denominator,
             fract_rect->height.numerator, fract_rect->height.denominator,
             fract_rect->left.numerator, fract_rect->left.denominator,
             fract_rect->top.numerator, fract_rect->top.denominator));
    }
}

/*!**************************************************************************
 * @brief    Create an identity matrix
 * @param    matrix - the matrix
 ****************************************************************************/
static void VTC_IdentityMatrix(S_VT_MATRIX *matrix) {
    matrix->a = MakeFraction(1, 1);
    matrix->b = MakeFraction(0, 1);
    matrix->c = MakeFraction(1, 1);
    matrix->d = MakeFraction(0, 1);
}

/*!**************************************************************************
 * @brief    Apply video CS transformation
 * @param    current - the current transformation matrix
 * @param    state - conversion state
 ****************************************************************************/
static void VTC_VideoCsScaling(S_VT_CONVERSION_STATE *state,
                               S_VT_MATRIX *current) {
    S_VT_MATRIX transform;

    if (state->video_width == 352 && state->video_height == 288) {
        Scale_352_288_Video(state, &transform);
    } else if (state->video_width == 352 && state->video_height == 576) {
        Scale_352_576_Video(state, &transform);
    } else if (state->video_width == 480 && state->video_height == 576) {
        Scale_480_576_Video(state, &transform);
    } else if (state->video_width == 544 && state->video_height == 576) {
        Scale_544_576_Video(state, &transform);
    } else {
        Scale_Video(state, &transform);
    }

    MultiplyMatrices(&transform, current, current);
}

/*!**************************************************************************
 * @brief    Apply AFD-override scaling transformation
 * @param    state - conversion state
 * @param    current - the current transformation matrix
 * @return   Format conversion applied
 ****************************************************************************/
static E_FORMAT_CONVERSION VTC_UserPreferenceScaling(
    S_VT_CONVERSION_STATE *state, S_VT_MATRIX *current) {
    E_FORMAT_CONVERSION transformation;
    S_VT_MATRIX transform;

    VTC_IdentityMatrix(&transform);
    transformation = FORMAT_CONVERSION_IGNORE;

    if (state->alignment != ASPECT_MODE_CUSTOM) {
        /* 4:3 display */
        if (state->display_aspect_ratio == ASPECT_RATIO_4_3) {
            /* 4:3 video */
            if (state->video_aspect_ratio == ASPECT_RATIO_4_3) {
                switch (state->alignment) {
                    case ASPECT_MODE_16_9:
                        Scale_4_3_Centre(state, state->resolution_width,
                                         state->resolution_height, &transform);
                        transformation = FORMAT_CONVERSION_CENTRE_4_3;
                        break;

                    case ASPECT_MODE_14_9:
                        Scale_14_9_Centre(state, state->resolution_width,
                                          state->resolution_height, &transform);
                        transformation = FORMAT_CONVERSION_CENTRE_14_9;
                        break;

                    default:;
                }
            }
            /* 16:9 video */
            else if (state->video_aspect_ratio == ASPECT_RATIO_16_9) {
                switch (state->alignment) {
                    case ASPECT_MODE_16_9:
                    default:
                        Scale_16_9_Letterbox(state, state->resolution_width,
                                             state->resolution_height,
                                             &transform);
                        transformation = FORMAT_CONVERSION_LETTERBOX;
                        break;

                    case ASPECT_MODE_14_9:
                        Scale_14_9_Letterbox(state, state->resolution_width,
                                             state->resolution_height,
                                             &transform);
                        transformation = FORMAT_CONVERSION_LETTERBOX_14_9;
                        break;

                    case ASPECT_MODE_4_3:
                    case ASPECT_MODE_ZOOM:
                        Scale_CentreCutOut(state, state->resolution_width,
                                           state->resolution_height,
                                           &transform);
                        transformation = FORMAT_CONVERSION_PANSCAN;
                        break;
                }
            }
        } else if (state->display_aspect_ratio == ASPECT_RATIO_16_9) {
            /* 16:9 display */
            if (state->video_aspect_ratio == ASPECT_RATIO_4_3) {
                /* 4:3 video */
                switch (state->alignment) {
                    case ASPECT_MODE_16_9:
                    case ASPECT_MODE_ZOOM:
                        Scale_4_3_Zoom(state, state->resolution_width,
                                       state->resolution_height, &transform);
                        transformation = FORMAT_CONVERSION_ZOOM_4_3;
                        break;

                    case ASPECT_MODE_14_9:
                        Scale_14_9_Zoom(state, state->resolution_width,
                                        state->resolution_height, &transform);
                        transformation = FORMAT_CONVERSION_ZOOM_14_9;
                        break;

                    case ASPECT_MODE_4_3:
                    default:
                        Scale_Pillarbox(state, state->resolution_width,
                                        state->resolution_height, &transform);
                        transformation = FORMAT_CONVERSION_PILLAR_BOX;
                        break;
                }
            } else if (state->video_aspect_ratio == ASPECT_RATIO_16_9) {
                /* 16:9 video */
                switch (state->alignment) {
                    case ASPECT_MODE_4_3:
                        Scale_Pillarbox(state, state->resolution_width,
                                        state->resolution_height, &transform);
                        transformation = FORMAT_CONVERSION_PILLAR_BOX;
                        break;

                    case ASPECT_MODE_14_9:
                        Scale_16_9_Zoom(state, state->resolution_width,
                                        state->resolution_height, &transform);
                        transformation = FORMAT_CONVERSION_PANSCAN_14_9;
                        break;

                    default:;
                }
            }
        }
    }

    MultiplyMatrices(&transform, current, current);

    return transformation;
}

/*!**************************************************************************
 * @brief    Apply AFD video scaling transformation
 * @param    state - conversion state
 * @param    current - the current transformation matrix
 * @return   Format conversion applied
 ****************************************************************************/
static E_FORMAT_CONVERSION VTC_AfdScaling(S_VT_CONVERSION_STATE *state,
                                          S_VT_MATRIX *current) {
    BOOLEAN apply;
    E_FORMAT_CONVERSION transformation;
    S_VT_MATRIX transform;

    apply = TRUE;
    transformation = FindAfdTransformation(state);

    switch (transformation) {
        case FORMAT_CONVERSION_PILLAR_BOX:
            Scale_Pillarbox(state, state->resolution_width,
                            state->resolution_height, &transform);
            break;

        case FORMAT_CONVERSION_ZOOM_4_3:
            Scale_4_3_Zoom(state, state->resolution_width,
                           state->resolution_height, &transform);
            break;

        case FORMAT_CONVERSION_ZOOM_14_9:
            Scale_14_9_Zoom(state, state->resolution_width,
                            state->resolution_height, &transform);
            break;

        case FORMAT_CONVERSION_LETTERBOX:
            Scale_16_9_Letterbox(state, state->resolution_width,
                                 state->resolution_height, &transform);
            break;

        case FORMAT_CONVERSION_LETTERBOX_14_9:
            Scale_14_9_Letterbox(state, state->resolution_width,
                                 state->resolution_height, &transform);
            break;

        case FORMAT_CONVERSION_PANSCAN:
            Scale_CentreCutOut(state, state->resolution_width,
                               state->resolution_height, &transform);
            break;

        case FORMAT_CONVERSION_PANSCAN_14_9:
            Scale_16_9_Zoom(state, state->resolution_width,
                            state->resolution_height, &transform);
            break;

        case FORMAT_CONVERSION_CENTRE_14_9:
            Scale_14_9_Centre(state, state->resolution_width,
                              state->resolution_height, &transform);
            break;

        case FORMAT_CONVERSION_CENTRE_4_3:
            Scale_4_3_Centre(state, state->resolution_width,
                             state->resolution_height, &transform);
            break;

        default:
            /* Couldn't find - don't transform */
            apply = FALSE;
    }

    if (apply) {
        MultiplyMatrices(&transform, current, current);
    }

    return transformation;
}

/*!**************************************************************************
 * @brief    Apply MHEG-5 video scaling transformation
 * @param    state - conversion state
 * @param    current - the current transformation matrix
 ****************************************************************************/
static void VTC_MhegScaling(S_VT_CONVERSION_STATE *state,
                            S_VT_MATRIX *current) {
    S_VT_MATRIX transform;

    MhegScaling(state, &transform);
    MultiplyMatrices(&transform, current, current);
}

/*!**************************************************************************
 * @brief    Apply HBBTV video transformation
 * @param    state - conversion state
 * @param    current - the current transformation matrix
 ****************************************************************************/
static void VTC_HbbScaleToScreen(S_VT_CONVERSION_STATE *state,
                                 S_VT_MATRIX *current) {
    S_VT_MATRIX transform;

    HbbScaling(state, &transform);
    DBG(("HbbScaling:\n"));
    PrintMatrix(&transform);

    MultiplyMatrices(&transform, current, current);

    Scale_HbbToScreen(state, &transform);
    DBG(("Scale_HbbToScreen:\n"));
    PrintMatrix(&transform);
    MultiplyMatrices(&transform, current, current);
}

/*!**************************************************************************
 * @brief    Apply application scaling transformation
 * @param    state - conversion state
 * @param    current - the current transformation matrix
 ****************************************************************************/
static void VTC_AppScaling(S_VT_CONVERSION_STATE *state, S_VT_MATRIX *current) {
    S_VT_MATRIX transform;

    AppScaling(state, &transform);
    MultiplyMatrices(&transform, current, current);
}

/*!**************************************************************************
 * @brief    Apply MHEG-5 quarter screen video scaling transformation
 * @param    state - conversion state
 * @param    current - the current transformation matrix
 * @return   Format conversion applied
 ****************************************************************************/
static E_FORMAT_CONVERSION VTC_QuarterScreenScaling(
    S_VT_CONVERSION_STATE *state, S_VT_MATRIX *current) {
    E_FORMAT_CONVERSION transformation;
    S_VT_MATRIX transform;

    transformation = FORMAT_CONVERSION_IGNORE;

    NoScaling(state, &transform);

    if (state->mheg_aspect_ratio == ASPECT_UNDEFINED) {
        if ((state->video_aspect_ratio == ASPECT_RATIO_4_3) &&
            (state->display_aspect_ratio == ASPECT_RATIO_16_9)) {
            /* 4:3 video on 16:9 display = pillar-box */
            transformation = FORMAT_CONVERSION_PILLAR_BOX;
            Scale_Pillarbox(state, state->resolution_width,
                            state->resolution_height, &transform);
        } else if ((state->video_aspect_ratio == ASPECT_RATIO_16_9) &&
                   (state->display_aspect_ratio == ASPECT_RATIO_4_3)) {
            /* 16:9 video on 4:3 display = pan-scan (centre cut-out) */
            transformation = FORMAT_CONVERSION_PANSCAN;
            Scale_CentreCutOut(state, state->resolution_width,
                               state->resolution_height, &transform);
        }
    } else if ((state->mheg_aspect_ratio == ASPECT_RATIO_4_3) &&
               (state->video_aspect_ratio == ASPECT_RATIO_16_9)) {
        /* SAR=4:3, 16:9 video = pan-scan (centre cut-out) */
        transformation = FORMAT_CONVERSION_PANSCAN;
        Scale_CentreCutOut(state, state->resolution_width,
                           state->resolution_height, &transform);
    } else if ((state->mheg_aspect_ratio == ASPECT_RATIO_16_9) &&
               (state->video_aspect_ratio == ASPECT_RATIO_4_3) &&
               (state->display_aspect_ratio == ASPECT_RATIO_16_9)) {
        /* SAR=16:9, 4:3 video = pillar-box (only on 16:9 display) */
        transformation = FORMAT_CONVERSION_PILLAR_BOX;
        Scale_Pillarbox(state, state->resolution_width,
                        state->resolution_height, &transform);
    } else if ((state->mheg_aspect_ratio == ASPECT_RATIO_16_9) &&
               (state->video_aspect_ratio == ASPECT_RATIO_16_9) &&
               (state->display_aspect_ratio == ASPECT_RATIO_4_3)) {
        /* SAR=16:9, 16:9 video = pan-scan (when using 4:3 display) */
        transformation = FORMAT_CONVERSION_PANSCAN;
        Scale_CentreCutOut(state, state->resolution_width,
                           state->resolution_height, &transform);
    }

    MultiplyMatrices(&transform, current, current);

    return transformation;
}

/*!**************************************************************************
 * @brief    Apply scene aspect ratio transformation (if required)
 * @param    state - conversion state
 * @param    current - the current transformation matrix
 ****************************************************************************/
static void VTC_SceneArScaling(S_VT_CONVERSION_STATE *state,
                               S_VT_MATRIX *current) {
    BOOLEAN apply;
    S_VT_MATRIX transform;

    apply = FALSE;

    if ((state->mheg_aspect_ratio == ASPECT_RATIO_4_3) &&
        (state->video_aspect_ratio == ASPECT_RATIO_16_9) &&
        (state->decoder_status == DECODER_STATUS_VIDEO)) {
        /* 16:9 video on 4:3 Scene: follow Widescreen Alignment Mode */
        switch (state->mheg_wam) {
            default:
                /* Anamorphic output - do nothing */
                break;

            case ASPECT_MODE_4_3:
                Scale_CentreCutOut(state, state->resolution_width,
                                   state->resolution_height, &transform);
                apply = TRUE;
                break;

            case ASPECT_MODE_16_9:
                Scale_16_9_Letterbox(state, state->resolution_width,
                                     state->resolution_height, &transform);
                apply = TRUE;
                break;
        }
    }

    if (apply) {
        MultiplyMatrices(&transform, current, current);
    }
}

/*!**************************************************************************
 * @brief   Compose a transformation from the presentation coordinates to
 *          screen coordinateswith the given transformation.
 * @param   state - conversion state
 * @param   current - the current transformation matrix
 ****************************************************************************/
static void VTC_ToScreen(S_VT_CONVERSION_STATE *state, S_VT_MATRIX *current) {
    S_VT_MATRIX transform;

    Scale_ToScreen(state, &transform);
    MultiplyMatrices(&transform, current, current);
}

/*!**************************************************************************
 * @brief    Compose an application scaling transformation with the given
 *           transformation
 * @param    state - conversion state
 * @param    current - the current transformation matrix
 * @return   Format conversion applied
 ****************************************************************************/
static E_FORMAT_CONVERSION VTC_AppScalingTransform(S_VT_CONVERSION_STATE *state,
                                                   S_VT_MATRIX *current) {
    E_FORMAT_CONVERSION transformation;
    S_VT_MATRIX transform;

    transformation = FORMAT_CONVERSION_IGNORE;

    /* Take care of video aspact ratio */
    NoScaling(state, &transform);

    if ((state->video_aspect_ratio == ASPECT_RATIO_4_3) &&
        (state->display_aspect_ratio == ASPECT_RATIO_16_9)) {
        /* 4:3 video on 16:9 display = pillar-box */
        transformation = FORMAT_CONVERSION_PILLAR_BOX;
        Scale_Pillarbox(state, state->resolution_width,
                        state->resolution_height, &transform);
    } else if ((state->video_aspect_ratio == ASPECT_RATIO_16_9) &&
               (state->display_aspect_ratio == ASPECT_RATIO_4_3)) {
        /* 16:9 video on 4:3 display = pan-scan (centre cut-out) */
        transformation = FORMAT_CONVERSION_PANSCAN;
        Scale_CentreCutOut(state, state->resolution_width,
                           state->resolution_height, &transform);
    }

    MultiplyMatrices(&transform, current, current);

    return transformation;
}

/*!**************************************************************************
 * @brief    Apply a WSS transformation on the output
 * @param    state - conversion state
 * @param    output - output double vector
 ****************************************************************************/
static void VTC_ApplyWSS(S_VT_CONVERSION_STATE *state,
                         S_VT_FRACT_RECT *output) {
    S_VT_MATRIX transform;

    DBG(("VTC_ApplyWSS: before:\n"));
    PrintFractRect(output);

    if ((state->scaling_mode == SCALING_MHEG) &&
        (state->mheg_aspect_ratio == ASPECT_RATIO_4_3) &&
        (state->display_aspect_ratio == ASPECT_RATIO_16_9)) {
        /* "Pillar-box" the output (input is unchanged) */
        VTC_IdentityMatrix(&transform);
        Scale_Pillarbox(state, state->screen_width, state->screen_height,
                        &transform);
        PrintMatrix(&transform);
        ApplyTransformation(&transform, output, output);
        DBG(("VTC_ApplyWSS: after:\n"));
        PrintFractRect(output);
    }
}

/*!**************************************************************************
 * @brief    Convert an S_RECTANGLE to a  S_VT_FRACT_RECT
 * @param    rectangle - rectangle to be converted
 * @param    vector - resulting vector
 ****************************************************************************/
static void VTC_RectangleToFractRectangle(S_RECTANGLE *rectangle,
                                          S_VT_FRACT_RECT *fract_rect) {
    fract_rect->left = MakeFraction(rectangle->left, 1);
    fract_rect->top = MakeFraction(rectangle->top, 1);
    fract_rect->width = MakeFraction(rectangle->width, 1);
    fract_rect->height = MakeFraction(rectangle->height, 1);
}

/*!**************************************************************************
 * @brief    Convert an S_VT_FRACT_RECT to a S_RECTANGLE (with rounded-down
 *           values)
 * @param    vector - vector to be converted
 * @param    rectangle - resulting rectangle
 ****************************************************************************/
static void VTC_FractRectangleToRectangle(S_VT_FRACT_RECT *fract_rect,
                                          S_RECTANGLE *rectangle) {
    rectangle->left =
        ((fract_rect->left.numerator + fract_rect->left.denominator / 2) /
         fract_rect->left.denominator);
    rectangle->top =
        ((fract_rect->top.numerator + fract_rect->top.denominator / 2) /
         fract_rect->top.denominator);
    rectangle->width =
        ((fract_rect->width.numerator + fract_rect->width.denominator / 2) /
         fract_rect->width.denominator);
    rectangle->height =
        ((fract_rect->height.numerator + fract_rect->height.denominator / 2) /
         fract_rect->height.denominator);
}

/*!**************************************************************************
 * @brief    Apply the given transformation on a rectangle
 * @param    state - conversion state
 * @param    transform - the transformation matrix
 * @param    clip_transform - clipping transformation matrix
 * @param    input_rect - input rectangle
 * @param    output_rect - output rectangle
 ****************************************************************************/
static void VTC_TransformRectangles(S_VT_CONVERSION_STATE *state,
                                    S_VT_MATRIX *transform,
                                    S_VT_MATRIX *clip_transform,
                                    S_VT_FRACT_RECT *input_rect,
                                    S_VT_FRACT_RECT *output_rect) {
    S_VT_FRACT_RECT screen_clip;
    S_VT_FRACT_RECT scale_clip;
    S_VT_FRACT_RECT trans_clip;
    S_VT_MATRIX inverse;

    screen_clip =
        MakeRectangle(0, 0, state->screen_width, state->screen_height);

    scale_clip =
        MakeRectangle(0, 0, state->resolution_width, state->resolution_height);

    /* Multiply the vectors by the input matrix */
    ApplyTransformation(transform, input_rect, output_rect);

    /* Create a clipping region for mheg scaling */
    ApplyTransformation(clip_transform, &scale_clip, &trans_clip);

    /* Clip output */
    ClipRectangle(&screen_clip, output_rect);
    ClipRectangle(&trans_clip, output_rect);

    /*check that app has specified non-zero size output*/
    if (transform->a.numerator != 0 && transform->c.numerator != 0) {
        /* Find input rectangle for clipped result */
        InvertMatrix(transform, &inverse);
        ApplyTransformation(&inverse, output_rect, input_rect);
    }
}

/******************************************/

/*!**************************************************************************
 * @brief    Clip rectangle to given region
 * @param    clip_rect - clipping region (rectangle)
 * @param    rect - the rectangle to clip
 ****************************************************************************/
static void ClipRectangle(S_VT_FRACT_RECT *clip_rect, S_VT_FRACT_RECT *rect) {
    S_VT_FRACTION right, bottom;
    S_VT_FRACTION clip_right, clip_bottom;

    AddFractions(&rect->left, &rect->width, &right);
    AddFractions(&rect->top, &rect->height, &bottom);

    AddFractions(&clip_rect->left, &clip_rect->width, &clip_right);
    AddFractions(&clip_rect->top, &clip_rect->height, &clip_bottom);

    if ((CompareFractions(&rect->left, &clip_right) >= 0) ||
        (CompareFractions(&rect->top, &clip_bottom) >= 0) ||
        (CompareFractions(&right, &clip_rect->left) <= 0) ||
        (CompareFractions(&bottom, &clip_rect->top) <= 0)) {
        /* Invalid transformation */
        rect->left = MakeFraction(0, 1);
        rect->top = MakeFraction(0, 1);
        rect->width = MakeFraction(0, 1);
        rect->height = MakeFraction(0, 1);
    } else {
        if (CompareFractions(&rect->left, &clip_rect->left) < 0) {
            SubtractFraction(&right, &clip_rect->left, &rect->width);
            rect->left = clip_rect->left;
        }

        if (CompareFractions(&rect->top, &clip_rect->top) < 0) {
            SubtractFraction(&bottom, &clip_rect->top, &rect->height);
            rect->top = clip_rect->top;
        }

        AddFractions(&rect->left, &rect->width, &right);
        AddFractions(&rect->top, &rect->height, &bottom);

        if (CompareFractions(&right, &clip_right) > 0) {
            SubtractFraction(&clip_right, &rect->left, &rect->width);
        }

        if (CompareFractions(&bottom, &clip_bottom) > 0) {
            SubtractFraction(&clip_bottom, &rect->top, &rect->height);
        }
    }
}

static S32BIT Gcd(S32BIT a, S32BIT b) {
    S32BIT r;

    while (b != 0) {
        r = b;
        b = a % b;
        a = r;
    }

    return a;
}

static BOOLEAN ReduceFraction(S_VT_FRACTION *input, S_VT_FRACTION *output) {
    S32BIT num, den, gcd;

    num = input->numerator >= 0 ? input->numerator : -input->numerator;
    den = input->denominator >= 0 ? input->denominator : -input->denominator;

    gcd = Gcd(num, den);

    ASSERT(gcd != 0);

    output->numerator = input->numerator / gcd;
    output->denominator = input->denominator / gcd;

    return TRUE;
}

static void AddFractions(S_VT_FRACTION *in_a, S_VT_FRACTION *in_b,
                         S_VT_FRACTION *out) {
    S32BIT gcd;

    /* Check if one of the inputs is 0, in which case the more
     * complicated addition function doesn't need to be performed
     */
    ASSERT(in_a->denominator != 0);
    ASSERT(in_b->denominator != 0);

    if (in_a->numerator == 0) {
        ReduceFraction(in_b, out);
        ASSERT(out->denominator != 0);
    } else if (in_b->numerator == 0) {
        ReduceFraction(in_a, out);
        ASSERT(out->denominator != 0);
    } else {
        /* Add the two fractions */
        gcd = Gcd(in_a->denominator, in_b->denominator);
        out->numerator = ((in_a->numerator * in_b->denominator / gcd) +
                          (in_b->numerator * in_a->denominator / gcd));
        out->denominator = in_a->denominator * in_b->denominator / gcd;
        ASSERT(out->denominator != 0);
        ReduceFraction(out, out);
    }
}

static void SubtractFraction(S_VT_FRACTION *in_a, S_VT_FRACTION *in_b,
                             S_VT_FRACTION *out) {
    S_VT_FRACTION temp;

    temp = *in_b;
    temp.numerator = 0 - temp.numerator;
    AddFractions(in_a, &temp, out);
}

static int CompareFractions(S_VT_FRACTION *in_a, S_VT_FRACTION *in_b) {
    return (in_a->numerator * in_b->denominator -
            in_b->numerator * in_a->denominator);
}

static void MultiplyMatrices(S_VT_MATRIX *matrix_a, S_VT_MATRIX *matrix_b,
                             S_VT_MATRIX *output_matrix) {
    S_VT_FRACTION buffer;
    S_VT_MATRIX tmp;

    /* Multiplying:
     *
     *  (a 0 b)   (a 0 b)
     *  (0 c d) x (0 c d)
     *  (0 0 1)   (0 0 1)
     *
     * The order used allows the output matrix to be the same as one of
     * the input matrices.
     */

    ASSERT(matrix_a->a.denominator != 0);
    ASSERT(matrix_a->b.denominator != 0);
    ASSERT(matrix_a->c.denominator != 0);
    ASSERT(matrix_a->d.denominator != 0);
    ASSERT(matrix_b->a.denominator != 0);
    ASSERT(matrix_b->b.denominator != 0);
    ASSERT(matrix_b->c.denominator != 0);
    ASSERT(matrix_b->d.denominator != 0);

    buffer.numerator = matrix_a->a.numerator * matrix_b->b.numerator;
    buffer.denominator = matrix_a->a.denominator * matrix_b->b.denominator;
    AddFractions(&buffer, &matrix_a->b, &tmp.b);

    tmp.a.denominator = matrix_a->a.denominator * matrix_b->a.denominator;
    tmp.a.numerator = matrix_a->a.numerator * matrix_b->a.numerator;
    ReduceFraction(&tmp.a, &tmp.a);

    buffer.numerator = matrix_a->c.numerator * matrix_b->d.numerator;
    buffer.denominator = matrix_a->c.denominator * matrix_b->d.denominator;
    ASSERT(buffer.denominator != 0);
    ASSERT(matrix_a->d.denominator != 0);
    AddFractions(&buffer, &matrix_a->d, &tmp.d);

    tmp.c.denominator = matrix_a->c.denominator * matrix_b->c.denominator;
    tmp.c.numerator = matrix_a->c.numerator * matrix_b->c.numerator;
    ReduceFraction(&tmp.c, &tmp.c);

    ASSERT(tmp.a.denominator != 0);
    ASSERT(tmp.b.denominator != 0);
    ASSERT(tmp.c.denominator != 0);
    ASSERT(tmp.d.denominator != 0);

    *output_matrix = tmp;
}

static void InvertMatrix(S_VT_MATRIX *matrix, S_VT_MATRIX *inverse) {
    S32BIT temp;

    /* Inverse:
     *
     *  (a 0 b)    (1/a   0   -b/a)
     *  (0 c d) -> ( 0   1/c  -d/c)
     *  (0 0 1)    ( 0    0     1 )
     *
     * The order used allows the output matrix to be the same as one of
     * the input matrices.
     */

    ASSERT(matrix->a.numerator != 0);
    ASSERT(matrix->c.numerator != 0);

    inverse->b.numerator = -matrix->b.numerator * matrix->a.denominator;
    inverse->b.denominator = matrix->b.denominator * matrix->a.numerator;

    if (inverse->b.denominator < 0) {
        inverse->b.numerator *= -1;
        inverse->b.denominator *= -1;
    }

    ReduceFraction(&inverse->b, &inverse->b);

    inverse->d.numerator = -matrix->d.numerator * matrix->c.denominator;
    inverse->d.denominator = matrix->d.denominator * matrix->c.numerator;

    if (inverse->d.denominator < 0) {
        inverse->d.numerator *= -1;
        inverse->d.denominator *= -1;
    }

    ReduceFraction(&inverse->d, &inverse->d);

    temp = matrix->a.numerator;
    inverse->a.numerator = matrix->a.denominator;
    inverse->a.denominator = temp;

    if (inverse->a.denominator < 0) {
        inverse->a.numerator *= -1;
        inverse->a.denominator *= -1;
    }

    temp = matrix->c.numerator;
    inverse->c.numerator = matrix->c.denominator;
    inverse->c.denominator = temp;

    if (inverse->c.denominator < 0) {
        inverse->c.numerator *= -1;
        inverse->c.denominator *= -1;
    }

    ASSERT(inverse->a.denominator != 0);
    ASSERT(inverse->b.denominator != 0);
    ASSERT(inverse->c.denominator != 0);
    ASSERT(inverse->d.denominator != 0);
}

/*!**************************************************************************
 * @brief    Apply a transformation to a vector
 * @param    matrix - the transformation matrix
 * @param    input  - the vector to transform
 * @param    output - the transformed vector
 ****************************************************************************/
static void ApplyTransformation(S_VT_MATRIX *matrix, S_VT_FRACT_RECT *input,
                                S_VT_FRACT_RECT *output) {
    S_VT_FRACT_RECT tmp;

    tmp.width.numerator = input->width.numerator * matrix->a.numerator;
    tmp.width.denominator = input->width.denominator * matrix->a.denominator;
    tmp.height.numerator = input->height.numerator * matrix->c.numerator;
    tmp.height.denominator = input->height.denominator * matrix->c.denominator;
    ReduceFraction(&tmp.width, &tmp.width);
    ReduceFraction(&tmp.height, &tmp.height);

    tmp.left.numerator = input->left.numerator * matrix->a.numerator;
    tmp.left.denominator = input->left.denominator * matrix->a.denominator;
    tmp.top.numerator = input->top.numerator * matrix->c.numerator;
    tmp.top.denominator = input->top.denominator * matrix->c.denominator;

    AddFractions(&tmp.left, &matrix->b, &tmp.left);
    AddFractions(&tmp.top, &matrix->d, &tmp.top);

    *output = tmp;
}

/*!**************************************************************************
 * @brief    Find the AFD transformation
 * @param    state - conversion state
 * @return   The AFD transformation, or NONE if not found
 ****************************************************************************/
static E_FORMAT_CONVERSION FindAfdTransformation(S_VT_CONVERSION_STATE *state) {
    E_FORMAT_CONVERSION trans;
    size_t i;

    DBG(("Looking for AFD transformation (%d, %s, %s, %s)\n", state->afd,
         state->video_aspect_ratio == ASPECT_RATIO_4_3    ? "ASPECT_RATIO_4_3"
         : state->video_aspect_ratio == ASPECT_RATIO_16_9 ? "ASPECT_RATIO_16_9"
                                                          : "UNKNOWN",
         state->display_aspect_ratio == ASPECT_RATIO_4_3 ? "ASPECT_RATIO_4_3"
         : state->display_aspect_ratio == ASPECT_RATIO_16_9
             ? "ASPECT_RATIO_16_9"
             : "UNKNOWN",
         state->afd_preference == AFD_PREFERENCE_CCO ? "AFD_PREFERENCE_CCO"
         : state->afd_preference == AFD_PREFERENCE_14_9_LB
             ? "AFD_PREFERENCE_14_9_LB"
         : state->afd_preference == AFD_PREFERENCE_16_9_LB
             ? "AFD_PREFERENCE_16_9_LB"
         : state->afd_preference == AFD_PREFERENCE_AUTO ? "AFD_PREFERENCE_AUTO"
                                                        : "UNKNOWN"));

    for (i = 0; i < sizeof(afd_table) / sizeof(*afd_table); i++) {
        /* Use AFD_PREFERENCE_AUTO as a fall-back */
        if ((afd_table[i].afd == state->afd) &&
            (afd_table[i].video_aspect_ratio == state->video_aspect_ratio) &&
            (afd_table[i].display_aspect_ratio ==
             state->display_aspect_ratio) &&
            ((afd_table[i].afd_preference == state->afd_preference) ||
             (afd_table[i].afd_preference == AFD_PREFERENCE_AUTO))) {
            trans = afd_table[i].transformation;
            DBG(("Found transformation %s\n",
                 trans == FORMAT_CONVERSION_PILLAR_BOX
                     ? "FORMAT_CONVERSION_PILLAR_BOX"
                 : trans == FORMAT_CONVERSION_ZOOM_4_3
                     ? "FORMAT_CONVERSION_ZOOM_4_3"
                 : trans == FORMAT_CONVERSION_ZOOM_14_9
                     ? "FORMAT_CONVERSION_ZOOM_14_9"
                 : trans == FORMAT_CONVERSION_LETTERBOX
                     ? "FORMAT_CONVERSION_LETTERBOX"
                 : trans == FORMAT_CONVERSION_LETTERBOX_14_9
                     ? "FORMAT_CONVERSION_LETTERBOX_14_9"
                 : trans == FORMAT_CONVERSION_PANSCAN
                     ? "FORMAT_CONVERSION_PANSCAN"
                 : trans == FORMAT_CONVERSION_PANSCAN_14_9
                     ? "FORMAT_CONVERSION_PANSCAN_14_9"
                     : "NONE"));

            return trans;
        }
    }

    return FORMAT_CONVERSION_IGNORE;
}

static void Scale_352_288_Video(S_VT_CONVERSION_STATE *state,
                                S_VT_MATRIX *transform) {
    ASSERT(state);

    transform->a = MakeFraction(state->resolution_width, 360);
    transform->b = MakeFraction(state->resolution_width, 90);
    transform->c = MakeFraction(state->resolution_height, 288);
    transform->d = MakeFraction(0, 1);
}

static void Scale_352_576_Video(S_VT_CONVERSION_STATE *state,
                                S_VT_MATRIX *transform) {
    ASSERT(state);

    transform->a = MakeFraction(state->resolution_width, 360);
    transform->b = MakeFraction(state->resolution_width, 90);
    transform->c = MakeFraction(state->resolution_height, 576);
    transform->d = MakeFraction(0, 1);
}

static void Scale_480_576_Video(S_VT_CONVERSION_STATE *state,
                                S_VT_MATRIX *transform) {
    ASSERT(state);

    transform->a = MakeFraction(state->resolution_width, 480);
    transform->b = MakeFraction(0, 1);
    transform->c = MakeFraction(state->resolution_height, 576);
    transform->d = MakeFraction(0, 1);
}

static void Scale_544_576_Video(S_VT_CONVERSION_STATE *state,
                                S_VT_MATRIX *transform) {
    ASSERT(state);

    transform->a = MakeFraction(state->resolution_width, 540);
    transform->b = MakeFraction(0 - state->resolution_width, 270);
    transform->c = MakeFraction(state->resolution_height, 576);
    transform->d = MakeFraction(0, 1);
}

static void Scale_Video(S_VT_CONVERSION_STATE *state, S_VT_MATRIX *transform) {
    ASSERT(state);

    transform->a = MakeFraction(state->resolution_width, state->video_width);
    transform->b = MakeFraction(0, 1);
    transform->c = MakeFraction(state->resolution_height, state->video_height);
    transform->d = MakeFraction(0, 1);
}

static void Scale_ToScreen(S_VT_CONVERSION_STATE *state,
                           S_VT_MATRIX *transform) {
    ASSERT(state);

    transform->a = MakeFraction(state->screen_width, state->resolution_width);
    transform->b = MakeFraction(0, 1);
    transform->c = MakeFraction(state->screen_height, state->resolution_height);
    transform->d = MakeFraction(0, 1);
}

static void Scale_HbbToScreen(S_VT_CONVERSION_STATE *state,
                              S_VT_MATRIX *transform) {
    ASSERT(state);

    transform->a = MakeFraction(state->screen_width, HD_WIDTH);
    transform->b = MakeFraction(0, 1);
    transform->c = MakeFraction(state->screen_height, HD_HEIGHT);
    transform->d = MakeFraction(0, 1);
}

static void Scale_Pillarbox(S_VT_CONVERSION_STATE *state, S32BIT width,
                            S32BIT height, S_VT_MATRIX *transform) {
    // USE_UNWANTED_PARAM(width);//unknown macro-chenfei.dou

    ASSERT(state);

    transform->a = MakeFraction(3, 4);
    transform->b = MakeFraction(width, 8);
    transform->c = MakeFraction(1, 1);
    transform->d = MakeFraction(0, 1);
}

static void Scale_4_3_Zoom(S_VT_CONVERSION_STATE *state, S32BIT width,
                           S32BIT height, S_VT_MATRIX *transform) {
    // USE_UNWANTED_PARAM(width);//unknown macro-chenfei.dou

    ASSERT(state);

    transform->a = MakeFraction(1, 1);
    transform->b = MakeFraction(0, 1);
    transform->c = MakeFraction(4, 3);
    transform->d = MakeFraction(-height, 6);
}

static void Scale_14_9_Zoom(S_VT_CONVERSION_STATE *state, S32BIT width,
                            S32BIT height, S_VT_MATRIX *transform) {
    ASSERT(state);

    transform->a = MakeFraction(7, 8);
    transform->b = MakeFraction(width, 16);
    transform->c = MakeFraction(7, 6);
    transform->d = MakeFraction(-height, 12);
}

static void Scale_14_9_Centre(S_VT_CONVERSION_STATE *state, S32BIT width,
                              S32BIT height, S_VT_MATRIX *transform) {
    ASSERT(state);

    transform->a = MakeFraction(8, 7);
    transform->b = MakeFraction(-width, 14);
    transform->c = MakeFraction(8, 7);
    transform->d = MakeFraction(-height, 14);
}

static void Scale_4_3_Centre(S_VT_CONVERSION_STATE *state, S32BIT width,
                             S32BIT height, S_VT_MATRIX *transform) {
    ASSERT(state);

    transform->a = MakeFraction(4, 3);
    transform->b = MakeFraction(-width, 6);
    transform->c = MakeFraction(4, 3);
    transform->d = MakeFraction(-height, 6);
}

static void Scale_16_9_Letterbox(S_VT_CONVERSION_STATE *state, S32BIT width,
                                 S32BIT height, S_VT_MATRIX *transform) {
    // USE_UNWANTED_PARAM(width);//unknown macro-chenfei.dou

    ASSERT(state);

    transform->a = MakeFraction(1, 1);
    transform->b = MakeFraction(0, 1);
    transform->c = MakeFraction(3, 4);
    transform->d = MakeFraction(height, 8);
}

static void Scale_14_9_Letterbox(S_VT_CONVERSION_STATE *state, S32BIT width,
                                 S32BIT height, S_VT_MATRIX *transform) {
    ASSERT(state);

    transform->a = MakeFraction(8, 7);
    transform->b = MakeFraction(-width, 14);
    transform->c = MakeFraction(6, 7);
    transform->d = MakeFraction(height, 14);
}

static void Scale_CentreCutOut(S_VT_CONVERSION_STATE *state, S32BIT width,
                               S32BIT height, S_VT_MATRIX *transform) {
    // USE_UNWANTED_PARAM(width);//unknown macro-chenfei.dou

    ASSERT(state);

    transform->a = MakeFraction(4, 3);
    transform->b = MakeFraction(-width, 6);
    transform->c = MakeFraction(1, 1);
    transform->d = MakeFraction(0, 1);
}

static void Scale_16_9_Zoom(S_VT_CONVERSION_STATE *state, S32BIT width,
                            S32BIT height, S_VT_MATRIX *transform) {
    ASSERT(state);

    transform->a = MakeFraction(7, 6);
    transform->b = MakeFraction(-width, 12);
    transform->c = MakeFraction(7, 6);
    transform->d = MakeFraction(-height, 12);
}

static void UpdateResolution(S_VT_CONVERSION_STATE *state) {
    if (state->scaling_mode == SCALING_MHEG) {
        state->resolution_width = state->mheg_resolution_width;
        state->resolution_height = state->mheg_resolution_height;
    } else if (state->scaling_mode == SCALING_HBBTV) {
        state->resolution_width = HD_WIDTH;
        state->resolution_height = HD_HEIGHT;
    }
}

static void MhegScaling(S_VT_CONVERSION_STATE *state, S_VT_MATRIX *transform) {
    S32BIT x = state->mheg_scaling_rect.left;
    S32BIT y = state->mheg_scaling_rect.top;
    S32BIT width = state->mheg_scaling_rect.width;
    S32BIT height = state->mheg_scaling_rect.height;
    S32BIT res_width = state->mheg_resolution_width;
    S32BIT res_height = state->mheg_resolution_height;

    transform->a = MakeFraction(width, res_width);
    transform->b = MakeFraction(x, 1);
    transform->c = MakeFraction(height, res_height);
    transform->d = MakeFraction(y, 1);
}

static void HbbScaling(S_VT_CONVERSION_STATE *state, S_VT_MATRIX *transform) {
    S32BIT x = state->hbb_window_rect.left;
    S32BIT y = state->hbb_window_rect.top;
    S32BIT width = state->hbb_window_rect.width;
    S32BIT height = state->hbb_window_rect.height;
    S32BIT adjust;

    if (state->video_aspect_ratio == ASPECT_RATIO_4_3) {
        adjust = (height * 4) / 3;

        if (width > adjust) {
            x += (width - adjust) / 2;
            width = adjust;
        } else {
            adjust = (width * 3) / 4;

            if (height > adjust) {
                y += (height - adjust) / 2;
                height = adjust;
            }
        }
    } else /*ASPECT_RATIO_16_9*/
    {
        adjust = (height * 16) / 9;

        if (width > adjust) {
            x += (width - adjust) / 2;
            width = adjust;
        } else {
            adjust = (width * 9) / 16;

            if (height > adjust) {
                y += (height - adjust) / 2;
                height = adjust;
            }
        }
    }

    transform->a = MakeFraction(width, HD_WIDTH);
    transform->b = MakeFraction(x, 1);
    transform->c = MakeFraction(height, HD_HEIGHT);
    transform->d = MakeFraction(y, 1);
}

static void AppScaling(S_VT_CONVERSION_STATE *state, S_VT_MATRIX *transform) {
    S32BIT x = state->app_scaling_window.left;
    S32BIT y = state->app_scaling_window.top;
    S32BIT width = state->app_scaling_window.width;
    S32BIT height = state->app_scaling_window.height;

    ASSERT(state);

    transform->a = MakeFraction(width, state->resolution_width);
    transform->b = MakeFraction(x, 1);
    transform->c = MakeFraction(height, state->resolution_height);
    transform->d = MakeFraction(y, 1);
}

static void NoScaling(S_VT_CONVERSION_STATE *state, S_VT_MATRIX *transform) {
    ASSERT(state);

    transform->a = MakeFraction(1, 1);
    transform->b = MakeFraction(0, 1);
    transform->c = MakeFraction(1, 1);
    transform->d = MakeFraction(0, 1);
}

static S_VT_FRACTION MakeFraction(S32BIT numerator, S32BIT denominator) {
    S_VT_FRACTION fraction;
    fraction.numerator = numerator;
    fraction.denominator = denominator;
    return fraction;
}

static S_VT_FRACT_RECT MakeRectangle(S32BIT left, S32BIT top, S32BIT width,
                                     S32BIT height) {
    S_VT_FRACT_RECT rect;
    rect.left.numerator = left;
    rect.top.numerator = top;
    rect.width.numerator = width;
    rect.height.numerator = height;

    rect.left.denominator = 1;
    rect.top.denominator = 1;
    rect.width.denominator = 1;
    rect.height.denominator = 1;
    return rect;
}

void VT_DisableScalingMode(void *context){
    S_VT_CONVERSION_STATE *state;

    if (context == NULL) return;

    state = (S_VT_CONVERSION_STATE *)context;
    if (state->scaling_mode != SCALING_NONE) {
        state->scaling_mode = SCALING_NONE;
        state->settings_changed = TRUE;
    }
}

void AFDHandle(void *context, S_FRAME_DIS_INFO *frame_info,
               E_ASPECT_RATIO frame_aspectratio, U8BIT afd_value) {
    BOOLEAN changed = FALSE;
    S_VT_CONVERSION_STATE *state;
    U8BIT afd_new = afd_value & 0x07;
    //E_ASPECT_RATIO dis_aspect;

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
    //dis_aspect = getDisplayAspect(state->screen_width, state->screen_height);
    //if (state->display_aspect_ratio != dis_aspect) {
    //    state->display_aspect_ratio = dis_aspect;
    //    changed = TRUE;
    //}
    if (state->video_aspect_ratio != frame_aspectratio) {
        state->video_aspect_ratio = frame_aspectratio;
        changed = TRUE;
    }
    if (state->afd != afd_new) {
        state->afd = afd_new;
        changed = TRUE;
    }
    if (changed) state->settings_changed = TRUE;

    if (state->settings_changed) {
        UpdateResolution(state);
        Recalculate(state);
        state->settings_changed = FALSE;
    }
}

S_RECTANGLE getInrectangle(void *context) {
    S_RECTANGLE input;
    S_VT_CONVERSION_STATE *state;
    InitRect(&input, 0, 0, 0, 0);

    if (context == NULL) return input;

    state = (S_VT_CONVERSION_STATE *)context;
    input = state->input_rectangle;
    return input;
}

S_RECTANGLE getOutrectangle(void *context) {
    S_RECTANGLE output;
    S_VT_CONVERSION_STATE *state;
    InitRect(&output, 0, 0, 0, 0);

    if (context == NULL) return output;

    state = (S_VT_CONVERSION_STATE *)context;
    output = state->output_rectangle;
    return output;
}

S_RECTANGLE getScalingRect(void *context) {
    S_RECTANGLE scaling;
    S_VT_CONVERSION_STATE *state;
    InitRect(&scaling, 0, 0, 0, 0);

    if (context == NULL) return scaling;

    state = (S_VT_CONVERSION_STATE *)context;
    if (state->scaling_mode == SCALING_NONE) {
        return scaling;
    } else if (state->scaling_mode == SCALING_APP) {
        scaling = state->app_scaling_window;
    } else if (state->scaling_mode == SCALING_HBBTV) {
        scaling = state->hbb_window_rect;
    } else if (state->scaling_mode == SCALING_MHEG) {
        if (state->mheg_scaling_given) {
            scaling = state->mheg_scaling_rect;
        } else {
            InitRect(&scaling, 0, 0, state->mheg_resolution_width, state->mheg_resolution_height);
        }
    }
    return scaling;
}

int getAspect(int numerator, int denominator) {
    int aspect = ASPECT_UNDEFINED;

    if (numerator == 4 && denominator == 3) {
        aspect = ASPECT_RATIO_4_3;
    } else if (numerator == 16 && denominator == 9) {
        aspect = ASPECT_RATIO_16_9;
    }

    return aspect;
}

int getDisplayAspect(int width, int height) {
    int aspect = ASPECT_UNDEFINED;

    if (width * 3 == height * 4)
        aspect = ASPECT_RATIO_4_3;
    else if (width * 9 == height * 14)
        aspect = ASPECT_RATIO_16_9;
    return aspect;
}

BOOLEAN checkInScaling(void *context) {
    S_VT_CONVERSION_STATE *state = (S_VT_CONVERSION_STATE *)context;

    if (state && state->afd_enabled) {
        if (state->alignment == ASPECT_MODE_AUTO)
            return TRUE;
        if (state->scaling_mode == SCALING_APP
            && (state->app_scaling_window.width != 0)
            && (state->app_scaling_window.height != 0))
            return TRUE;
        if (state->scaling_mode == SCALING_HBBTV
            && (state->hbb_window_rect.width != 0)
            && (state->hbb_window_rect.height != 0))
            return TRUE;
        if (state->scaling_mode == SCALING_MHEG
            && (state->mheg_scaling_given)
            && (state->mheg_scaling_rect.width != 0)
            && (state->mheg_scaling_rect.height != 0))
            return TRUE;
    }
    return FALSE;
}

void print_vt_state(void *context, char* buf, int count) {
    S_VT_CONVERSION_STATE *state = (S_VT_CONVERSION_STATE *)context;
    snprintf(buf, count, "  enable : %d\n"
                         "  aspect : %d(0 auto, 1 4:3, 2 16:9, 3 14:9, 4 zoom, 5:cus)\n"
                         "  value  : %d\n"
                         "  type   : %d(0 none, 1 app, 2 mheg)\n"
                         "  scaling: %d %d %d %d\n"
                         "  res    : %d %d\n"
                         "  video  : %d %d\n"
                         "  screen : %d %d\n"
                         "  vaspect: %d(0 4:3, 1 16:9 255 und)\n"
                         "  v out  : %d %d %d %d\n"
                         "  dis out: %d %d %d %d\n",
                         state->afd_enabled,
                         state->alignment,
                         state->afd,
                         state->scaling_mode,
                         getScalingRect(state).left, getScalingRect(state).top,
                         getScalingRect(state).width, getScalingRect(state).height,
                         state->resolution_width, state->resolution_height,
                         state->video_width, state->video_height,
                         state->screen_width, state->screen_height,
                         state->video_aspect_ratio,
                         state->input_rectangle.left, state->input_rectangle.top,
                         state->input_rectangle.width, state->input_rectangle.height,
                         state->output_rectangle.left, state->output_rectangle.top,
                         state->output_rectangle.width, state->output_rectangle.height);
}
