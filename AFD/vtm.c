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


#include "vtm.h"

static VT_CONTEXTS_t mVtContexts;

void init_vt_context(void) {
    int i;
    S_VT_OVERSCANS_t defaultOverscan;

    mVtContexts.mAspectMode = ASPECT_MODE_CUSTOM;
    for (i = 0; i < MAX_PLAYER_INSTANCES; i ++) {
        mVtContexts.vs[i].path = -1;
        mVtContexts.vs[i].inst_id = -1;
        VT_Rest(&(mVtContexts.vs[i].vtc));
        mVtContexts.vs[i].handle = NULL;
    }

    defaultOverscan.uhd.hs = 0;
    defaultOverscan.uhd.vs = 0;
    defaultOverscan.uhd.re = 0;
    defaultOverscan.uhd.be = 0;
    defaultOverscan.fhd.hs = HD_OVERSCAN_H;
    defaultOverscan.fhd.vs = HD_OVERSCAN_V;
    defaultOverscan.fhd.re = HD_OVERSCAN_H;
    defaultOverscan.fhd.be = HD_OVERSCAN_V;
    defaultOverscan.hd.hs = HD_OVERSCAN_H;
    defaultOverscan.hd.vs = HD_OVERSCAN_V;
    defaultOverscan.hd.re = HD_OVERSCAN_H;
    defaultOverscan.hd.be = HD_OVERSCAN_V;
    defaultOverscan.sd.hs = SD_OVERSCAN_H;
    defaultOverscan.sd.vs = SD_OVERSCAN_V;
    defaultOverscan.sd.re = SD_OVERSCAN_H;
    defaultOverscan.sd.be = SD_OVERSCAN_V;
    VT_Set_Global_Overscan(&defaultOverscan);
}

void* create_vtc(int path) {
    int i;
    void *findVtc = find_vtc(path);

    if (findVtc) return findVtc;

    for (i = 0; i < MAX_PLAYER_INSTANCES; i ++) {
        if (mVtContexts.vs[i].path == -1) {
            mVtContexts.vs[i].path = path;
            mVtContexts.vs[i].inst_id = 0;
            mVtContexts.vs[i].handle = NULL;
            VT_Rest(&(mVtContexts.vs[i].vtc));
            VT_SetVideoAlignmentPref(&(mVtContexts.vs[i].vtc), mVtContexts.mAspectMode);
            return &(mVtContexts.vs[i]);
        }
    }
    return NULL;
}

void* find_vtc(int path) {
    int i;
    VT_NODE_t *v = NULL;

    for (i = 0; i < MAX_PLAYER_INSTANCES; i ++) {
        if (mVtContexts.vs[i].path == path) {
            v = &(mVtContexts.vs[i]);
            return v;
        }
    }
    return NULL;
}

void* find_vtc_inst(int inst_id) {
    int i;
    VT_NODE_t *v = NULL;

    for (i = 0; i < MAX_PLAYER_INSTANCES; i ++) {
        if (mVtContexts.vs[i].inst_id == inst_id) {
            v = &(mVtContexts.vs[i]);
            return v;
        }
    }
    return NULL;
}

void* find_vtc_inst_by_handle(void * handle) {
    int i;
    VT_NODE_t *v = NULL;

    for (i = 0; i < MAX_PLAYER_INSTANCES; i ++) {
        if (mVtContexts.vs[i].handle == handle) {
            v = &(mVtContexts.vs[i]);
            return v;
        }
    }
    return NULL;
}

int release_vtc(int path) {
    int i;

    for (i = 0; i < MAX_PLAYER_INSTANCES; i ++) {
        if (mVtContexts.vs[i].path == path) {
            mVtContexts.vs[i].path = -1;
            mVtContexts.vs[i].inst_id = -1;
            mVtContexts.vs[i].handle = NULL;
            VT_Rest(&(mVtContexts.vs[i].vtc));
            return path;
        }
    }
    return -1;
}

void apply_aspect(int mode) {
    int i;

    mVtContexts.mAspectMode = mode;
    for (i = 0; i < MAX_PLAYER_INSTANCES; i ++) {
        if (mVtContexts.vs[i].path != -1) {
            VT_SetVideoAlignmentPref(&(mVtContexts.vs[i].vtc), mVtContexts.mAspectMode);
        }
    }
}

unsigned int get_vtc_paths(int* array, unsigned int size) {
    int i, path;
    int index = 0;

    if (!array || size == 0)
        return 0;

    for (i = 0; i < MAX_PLAYER_INSTANCES; i ++) {
        if (index < size) {
            path = mVtContexts.vs[i].path;
            if (path != -1) {
                array[index] = path;
                index ++;
            }
        }
    }

    return index;
}

void print_afd_value(char *buf, int count, int debug_val) {
    int value, i;
    char tmp_buf[32];

    if (!buf) return;

    for (i = 0; i < MAX_PLAYER_INSTANCES; i ++) {
        if (mVtContexts.vs[i].path != -1) {
            value = mVtContexts.vs[i].vtc.afd;
            if (debug_val != 0xff)
                value = debug_val;
            memset(tmp_buf, 0, 32);
            snprintf(tmp_buf, 32, "%d-%d: %d\n", mVtContexts.vs[i].path, mVtContexts.vs[i].inst_id, value);
            strcat(buf, tmp_buf);
        }
    }
}

void print_vt_states(char *buf, int count) {
    int i;
    char tmp_buf[1024];
    if (!buf) return;

    for (i = 0; i < MAX_PLAYER_INSTANCES; i ++) {
        if (mVtContexts.vs[i].path != -1) {
            memset(tmp_buf, 0, 1024);
            snprintf(tmp_buf, 1024, "AFD: path %d, inst_id %d\n", mVtContexts.vs[i].path, mVtContexts.vs[i].inst_id);
            strcat(buf, tmp_buf);
            memset(tmp_buf, 0, 1024);
            print_vt_state(&(mVtContexts.vs[i].vtc), tmp_buf, 1024);
            strcat(buf, tmp_buf);
        }
    }
}
