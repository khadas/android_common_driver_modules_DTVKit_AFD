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


#include <linux/list.h>
#include <linux/slab.h>
#include "vtc.h"

typedef struct vt_context {
    int path;
    int inst_id;
    S_VT_CONVERSION_STATE vtc;
    struct list_head node;
}VT_NODE_t;

void init_vt_context(void);
void* create_vtc(int path);
int release_vtc(int path);
void* find_vtc(int path);
void* find_vtc_inst(int inst_id);
void* next_vtc(void* vt_context);
void apply_aspect(int mode);

void print_m5_res(char *buf, int count);
void print_m5_aspect(char *buf, int count);
void print_m5_wam(char *buf, int count);
void print_afd_value(char *buf, int count, int debug_val);
void print_scaling_type(char *buf, int count);
void print_enable_value(char *buf, int count);
void print_scaling_value(char *buf, int count);
void print_aspect_mode(char *buf, int count);
void print_vt_states(char *buf, int count);