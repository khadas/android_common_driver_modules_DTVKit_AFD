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

static VT_NODE_t head;
static E_VIDEO_ASPECT_MODE mAspectMode = ASPECT_MODE_CUSTOM;

void init_vt_context(void) {
    head.path = -1;
    head.inst_id = -1;
    VT_Rest(&(head.vtc));
    INIT_LIST_HEAD(&(head.node));
}

void* create_vtc(int path) {
    VT_NODE_t* vt = kzalloc(sizeof(VT_NODE_t), GFP_KERNEL);
    if (vt) {
        vt->path = path;
        vt->inst_id = 0;
        VT_Rest(&(vt->vtc));
        vt->vtc.alignment = mAspectMode;
        //INIT_LIST_HEAD(&(vt->node));
        list_add_tail(&(vt->node), &(head.node));
        return vt;
    }
    return NULL;
}

void* find_vtc(int path) {
    VT_NODE_t *v = NULL;
    struct list_head *s = &(head.node);

    if (list_empty(s)) return v;

    list_for_each_entry(v, s, node) {
        if (v && v->path == path) {
            return v;
            break;
        }
    }
    return NULL;
}

void* find_vtc_inst(int inst_id) {
    VT_NODE_t *v = NULL;
    struct list_head *s = &(head.node);

    if (list_empty(s)) return v;

    list_for_each_entry(v, s, node) {
        if (v && v->inst_id == inst_id) {
            return v;
            break;
        }
    }
    return NULL;
}

int release_vtc(int path) {
    VT_NODE_t *t = find_vtc(path);
    if (t) {
        list_del(&(t->node));
        kfree(t);
        return path;
    }
    return -1;
}

void* next_vtc(void* vt_context) {
    VT_NODE_t *v = NULL;
    struct list_head *n = head.node.next;
    if (vt_context) {
        v = (VT_NODE_t*)(vt_context);
        n = v->node.next;
    }
    if (n != &(head.node)) {
        return list_entry(n, typeof(*v), node);
    }
    return NULL;
}

void apply_aspect(int mode) {
    VT_NODE_t *v = NULL;
    struct list_head *s = &(head.node);

    if (mode >= ASPECT_MODE_AUTO && mode <= ASPECT_MODE_CUSTOM) {
        mAspectMode = mode;
        list_for_each_entry(v, s, node) {
            if (v) {
                VT_SetVideoAlignmentPref(&(v->vtc), mode);
            }
        }
    }
}

void print_m5_res(char *buf, int count) {
    char tmp_buf[40];
    VT_NODE_t *v = NULL;
    struct list_head *s = &(head.node);

    if (!buf) return;
    if (list_empty(s)) return;

    list_for_each_entry(v, s, node) {
        if (v) {
            memset(tmp_buf, 0, 40);
            snprintf(tmp_buf, 40, "%d-%d: %d %d\n", v->path, v->inst_id,
                v->vtc.mheg_resolution_width,
                v->vtc.mheg_resolution_height);
            strcat(buf, tmp_buf);
        }
    }
}

void print_m5_aspect(char *buf, int count) {
    char tmp_buf[32];
    VT_NODE_t *v = NULL;
    struct list_head *s = &(head.node);

    if (!buf) return;
    if (list_empty(s)) return;

    list_for_each_entry(v, s, node) {
        if (v) {
            memset(tmp_buf, 0, 32);
            snprintf(tmp_buf, 32, "%d-%d: %d\n", v->path, v->inst_id,
                v->vtc.mheg_aspect_ratio);
            strcat(buf, tmp_buf);
        }
    }
}

void print_m5_wam(char *buf, int count) {
    char tmp_buf[32];
    VT_NODE_t *v = NULL;
    struct list_head *s = &(head.node);

    if (!buf) return;
    if (list_empty(s)) return;

    list_for_each_entry(v, s, node) {
        if (v) {
            memset(tmp_buf, 0, 32);
            snprintf(tmp_buf, 32, "%d-%d: %d\n", v->path, v->inst_id,
                v->vtc.mheg_wam);
            strcat(buf, tmp_buf);
        }
    }
}

void print_afd_value(char *buf, int count, int debug_val) {
    int value;
    char tmp_buf[32];
    VT_NODE_t *v = NULL;
    struct list_head *s = &(head.node);

    if (!buf) return;
    if (list_empty(s)) return;

    list_for_each_entry(v, s, node) {
        if (v) {
            memset(tmp_buf, 0, 32);
            value = v->vtc.afd;
            if (debug_val != 0xff)
                value = debug_val;
            snprintf(tmp_buf, 32, "%d-%d: %d\n", v->path, v->inst_id, value);
            strcat(buf, tmp_buf);
        }
    }
}

void print_scaling_type(char *buf, int count) {
    char tmp_buf[32];
    VT_NODE_t *v = NULL;
    struct list_head *s = &(head.node);

    if (!buf) return;
    if (list_empty(s)) return;

    list_for_each_entry(v, s, node) {
        if (v) {
            memset(tmp_buf, 0, 32);
            snprintf(tmp_buf, 32, "%d-%d: %d\n", v->path, v->inst_id,
                v->vtc.scaling_mode);
            strcat(buf, tmp_buf);
        }
    }
}

void print_enable_value(char *buf, int count) {
    char tmp_buf[32];
    VT_NODE_t *v = NULL;
    struct list_head *s = &(head.node);

    if (!buf) return;
    if (list_empty(s)) return;

    list_for_each_entry(v, s, node) {
        if (v) {
            memset(tmp_buf, 0, 32);
            snprintf(tmp_buf, 32, "%d-%d: %d\n", v->path, v->inst_id,
                v->vtc.afd_enabled);
            strcat(buf, tmp_buf);
        }
    }
}

void print_scaling_value(char *buf, int count) {
    char tmp_buf[40];
    VT_NODE_t *v = NULL;
    S_RECTANGLE scaling;
    struct list_head *s = &(head.node);

    if (!buf) return;
    if (list_empty(s)) return;

    list_for_each_entry(v, s, node) {
        if (v) {
            memset(tmp_buf, 0, 40);
            scaling = getScalingRect(&(v->vtc));
            snprintf(tmp_buf, 40, "%d-%d: %d %d %d %d\n", v->path, v->inst_id,
                scaling.left, scaling.top, scaling.width, scaling.height);
            strcat(buf, tmp_buf);
        }
    }
}

void print_aspect_mode(char *buf, int count) {
    char tmp_buf[32];
    VT_NODE_t *v = NULL;
    struct list_head *s = &(head.node);

    if (!buf) return;
    if (list_empty(s)) return;

    list_for_each_entry(v, s, node) {
        if (v) {
            memset(tmp_buf, 0, 32);
            snprintf(tmp_buf, 32, "%d-%d: %d\n", v->path, v->inst_id,
                v->vtc.alignment);
            strcat(buf, tmp_buf);
        }
    }
}

void print_vt_states(char *buf, int count) {
    char tmp_buf[1024];
    VT_NODE_t *v = NULL;
    struct list_head *s = &(head.node);

    if (!buf) return;
    if (list_empty(s)) return;

    list_for_each_entry(v, s, node) {
        if (v) {
            memset(tmp_buf, 0, 1024);
            snprintf(tmp_buf, 1024, "AFD: path %d, inst_id %d\n", v->path, v->inst_id);
            strcat(buf, tmp_buf);
            memset(tmp_buf, 0, 1024);
            print_vt_state(&(v->vtc), tmp_buf, 1024);
            strcat(buf, tmp_buf);
        }
    }
}