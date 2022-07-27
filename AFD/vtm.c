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

static VT_CONTEXTS_t mVtcs;

void init_vt_context(void) {
    int i;

    mVtcs.mAspectMode = ASPECT_MODE_CUSTOM;
    for (i = 0; i < MAX_PLAYER_INSTS; i ++) {
        mVtcs.vtns[i].path = -1;
        mVtcs.vtns[i].inst_id = -1;
        VT_Rest(&(mVtcs.vtns[i].vtc));
    }
}

void* create_vtc(int path) {
    int i;

    for (i = 0; i < MAX_PLAYER_INSTS; i ++) {
        if (mVtcs.vtns[i].path == -1) {
            mVtcs.vtns[i].path = path;
            mVtcs.vtns[i].inst_id = 0;
            VT_Rest(&(mVtcs.vtns[i].vtc));
            mVtcs.vtns[i].vtc.alignment = mVtcs.mAspectMode;
            return &(mVtcs.vtns[i]);
        }
    }
    return NULL;
}

void* find_vtc(int path) {
    int i;
    VT_NODE_t *v = NULL;

    for (i = 0; i < MAX_PLAYER_INSTS; i ++) {
        if (mVtcs.vtns[i].path == path) {
            v = &(mVtcs.vtns[i]);
            return v;
        }
    }
    return NULL;
}

void* find_vtc_inst(int inst_id) {
    int i;
    VT_NODE_t *v = NULL;

    for (i = 0; i < MAX_PLAYER_INSTS; i ++) {
        if (mVtcs.vtns[i].inst_id == inst_id) {
            v = &(mVtcs.vtns[i]);
            return v;
        }
    }
    return NULL;
}

int release_vtc(int path) {
    int i;

    for (i = 0; i < MAX_PLAYER_INSTS; i ++) {
        if (mVtcs.vtns[i].path == path) {
            mVtcs.vtns[i].path = -1;
            mVtcs.vtns[i].inst_id = -1;
            VT_Rest(&(mVtcs.vtns[i].vtc));
            return path;
        }
    }
    return -1;
}

void apply_aspect(int mode) {
    int i;

    mVtcs.mAspectMode = mode;
    for (i = 0; i < MAX_PLAYER_INSTS; i ++) {
        if (mVtcs.vtns[i].path != -1) {
            mVtcs.vtns[i].vtc.alignment = mVtcs.mAspectMode;
        }
    }
}

void print_afd_value(char *buf, int count, int debug_val) {
    int value, i;
    char tmp_buf[32];

    if (!buf) return;

    for (i = 0; i < MAX_PLAYER_INSTS; i ++) {
        if (mVtcs.vtns[i].path != -1) {
            value = mVtcs.vtns[i].vtc.afd;
            if (debug_val != 0xff)
                value = debug_val;
            memset(tmp_buf, 0, 32);
            snprintf(tmp_buf, 32, "%d-%d: %d\n", mVtcs.vtns[i].path, mVtcs.vtns[i].inst_id, value);
            strcat(buf, tmp_buf);
        }
    }
}

void print_vt_states(char *buf, int count) {
    int i;
    char tmp_buf[1024];
    if (!buf) return;

    for (i = 0; i < MAX_PLAYER_INSTS; i ++) {
        if (mVtcs.vtns[i].path != -1) {
            memset(tmp_buf, 0, 1024);
            snprintf(tmp_buf, 1024, "AFD: path %d, inst_id %d\n", mVtcs.vtns[i].path, mVtcs.vtns[i].inst_id);
            strcat(buf, tmp_buf);
            memset(tmp_buf, 0, 1024);
            print_vt_state(&(mVtcs.vtns[i].vtc), tmp_buf, 1024);
            strcat(buf, tmp_buf);
        }
    }
}