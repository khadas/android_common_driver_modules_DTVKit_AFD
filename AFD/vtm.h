/*
 *
 * Copyright (c) 2015 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 *
 */
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

void print_m5_res(char *buf, int count);
void print_m5_aspect(char *buf, int count);
void print_m5_wam(char *buf, int count);
void print_afd_value(char *buf, int count, int debug_val);
void print_scaling_type(char *buf, int count);
void print_enable_value(char *buf, int count);
void print_scaling_value(char *buf, int count);
void print_aspect_mode(char *buf, int count);
void print_vt_states(char *buf, int count);