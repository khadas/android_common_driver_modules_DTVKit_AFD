/*
 *
 * Copyright (c) 2015 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * tb_module related tb detect
 *
 */

#include <linux/amlogic/major.h>
#include <linux/ctype.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/compat.h>

#include "AFD/AFDparse.h"
#include "AFD/vtm.h"
#include "afd_internal.h"

#define DRV_NAME "afd_module"
#define DEV_NAME "afd_module"
#define BUS_NAME "afd_module"
#define CLS_NAME "afd_module"

static struct device *afd_modue_dev;

static char afd_version_str[] = "AFD module: v2022.03.03a";
static unsigned int afd_debug_flag;
static unsigned int afd_debug_value_flag;

static int parse_para(const char *para, int para_num, int *result) {
    char *token = NULL;
    char *params, *params_base;
    int *out = result;
    int len = 0, count = 0;
    int res = 0;
    int ret = 0;

    if (!para) return 0;

    params = kstrdup(para, GFP_KERNEL);
    params_base = params;
    token = params;
    if (token) {
        len = strlen(token);
        do {
            token = strsep(&params, " ");
            if (!token) break;
            while (token && (isspace(*token) || !isgraph(*token)) && len) {
                token++;
                len--;
            }
            if (len == 0) break;
            ret = kstrtoint(token, 0, &res);
            if (ret < 0) break;
            len = strlen(token);
            *out++ = res;
            count++;
        } while ((count < para_num) && (len > 0));
    }

    kfree(params_base);
    return count;
}

static void update_scaling_rect(const char *para) {
    int parsed[5];
    S_RECTANGLE rect;

    if (likely(parse_para(para, 5, parsed) == 5)) {
        if (afd_debug_flag) {
            pr_err("[AFD] receive scaling rect: [%d,%d,%d,%d,%d]",
                   parsed[0], parsed[1], parsed[2],parsed[3],parsed[4]);
        }
        int path = parsed[0];
        rect.left = parsed[1];
        rect.top = parsed[2];
        rect.width = parsed[3];
        rect.height = parsed[4];
        VT_NODE_t* vt = find_vtc(path);
        if (!vt) {
            vt = create_vtc(path);
        }
        S_VT_CONVERSION_STATE* vt_context = vt ? &(vt->vtc) : NULL;
        if (vt_context) {
            if (vt_context->scaling_mode == SCALING_APP) {
                VT_SetAppScaling(vt_context, &rect);
            } else if (vt_context->scaling_mode == SCALING_HBBTV) {
                VT_SetHbbtvWindow(vt_context, &rect);
            } else if (vt_context->scaling_mode == SCALING_MHEG) {
                VT_SetMhegScaling(vt_context, &rect);
            }
        } else {
            pr_err("[AFD]: %s- no %d exist\n", __func__, path);
        }
    }
}

/*********************************************************
 * vpp afd callback APIs
 *********************************************************/
static void *afd_init_wrap(void) {
    struct afd_handle_s *handle = NULL;

    handle = vmalloc(sizeof(struct afd_handle_s));
    memset(handle, 0, sizeof(struct afd_handle_s));

    return (void *)handle;
}

static int afd_info_get_wrap(void *handle, struct afd_in_param *in,
                             struct afd_out_param *out) {
    int ret = AFD_RESULT_ERROR;
    unsigned char afd;
    S_FRAME_DIS_INFO frame_info;
    int aspect = 0xff;

    struct afd_handle_s *afd_h = (struct afd_handle_s *)handle;

    if (!handle || !in || !out) return ret;

    if (!in->ud_param) {
        if (afd_debug_flag)
            pr_err("[AFD] ud param is null");
        return ret;
    }

    unsigned int inst_id;
    unsigned int vpts;
    afd = getaf((unsigned char *)in->ud_param, &inst_id, &vpts);
    VT_NODE_t* vt = find_vtc_inst(inst_id);
    //for old path devices, decoder maybe has no instance info,
    //we will re-try to find path 0 vtc for main path
    if (!vt) {
        if (afd_debug_flag) pr_err("[AFD] %s: invalid inst id from decoder, try to use path 0.", __func__);
        vt = find_vtc(0);
    }
    if (afd_debug_flag) {
        pr_err("[AFD] %s: in inst(%u) with afd(%d) and vpts(%u), context: %p,(%d-%d), video ar(%d,%d), display in(%d,%d,%d,%d)",
            __func__, inst_id, (afd & 0x07), vpts, vt, (vt?vt->path:-1), (vt?vt->inst_id:-1),
            in->video_ar.numerator, in->video_ar.denominator,
            in->disp_info.x_start, in->disp_info.y_start, in->disp_info.x_end, in->disp_info.y_end);
    }
    S_VT_CONVERSION_STATE* vt_context = vt ? &(vt->vtc) : NULL;
    if (!vt_context) return ret;

    if (!vt_context->afd_enabled && !afd_debug_flag) return ret;

    // call ud param parser
    if (afd_debug_flag && afd_debug_value_flag != 0xff)
        afd = afd_debug_value_flag;
    else {
        if (afd == 0xff) afd = vt_context->afd;
    }
    // call afd handle init
    // call afd handle process to get crop and position information.
    frame_info.video_width = in->video_w;
    frame_info.video_height = in->video_h;
    frame_info.screen_width = in->disp_info.x_end - in->disp_info.x_start;
    frame_info.screen_height = in->disp_info.y_end - in->disp_info.y_start;
    aspect = getAspect(in->video_ar.numerator, in->video_ar.denominator);
    AFDHandle(vt_context, &frame_info, aspect, afd);
    S_RECTANGLE crop = getInrectangle(vt_context);
    S_RECTANGLE disp = getOutrectangle(vt_context);

    out->afd_enable = checkInScaling(vt_context);
    out->crop_info.top = crop.top;
    out->crop_info.left = crop.left;
    out->crop_info.bottom = crop.top;
    out->crop_info.right = crop.left;

    out->dst_pos.x_start = in->disp_info.x_start + disp.left;
    out->dst_pos.x_end = in->disp_info.x_start + disp.left + disp.width;
    out->dst_pos.y_start = in->disp_info.y_start + disp.top;
    out->dst_pos.y_end = in->disp_info.y_start + disp.top + disp.height;

    ret = AFD_RESULT_OK;
    afd_h->cur_video_ar.denominator = in->video_ar.denominator;
    afd_h->cur_video_ar.numerator = in->video_ar.numerator;
    afd_h->cur_screen_ar.denominator = in->screen_ar.denominator;
    afd_h->cur_screen_ar.numerator = in->screen_ar.numerator;
    afd_h->cur_screen_w = in->screen_w;
    afd_h->cur_screen_h = in->screen_h;
    afd_h->cur_video_w = in->video_w;
    afd_h->cur_video_h = in->video_h;
    afd_h->cur_disp_info.x_start = in->disp_info.x_start;
    afd_h->cur_disp_info.x_end = in->disp_info.x_end;
    afd_h->cur_disp_info.y_start = in->disp_info.y_start;
    afd_h->cur_disp_info.y_end = in->disp_info.y_end;

    if (memcmp(&out->crop_info, &afd_h->cur_crop_info,
               sizeof(struct crop_rect_s))) {
        afd_h->cur_crop_info.top = out->crop_info.top;
        afd_h->cur_crop_info.left = out->crop_info.left;
        afd_h->cur_crop_info.bottom = out->crop_info.bottom;
        afd_h->cur_crop_info.right = out->crop_info.right;
        ret |= AFD_RESULT_CROP_CHANGED;
    }
    if (memcmp(&out->dst_pos, &afd_h->cur_dst_info,
               sizeof(struct pos_rect_s))) {
        afd_h->cur_dst_info.x_start = out->dst_pos.x_start;
        afd_h->cur_dst_info.x_end = out->dst_pos.x_end;
        afd_h->cur_dst_info.y_start = out->dst_pos.y_start;
        afd_h->cur_dst_info.y_end = out->dst_pos.y_end;
        ret |= AFD_RESULT_DISP_CHANGED;
    }

    if (afd_h->afd_enable != out->afd_enable) {
        afd_h->afd_enable = out->afd_enable;
        ret |= AFD_RESULT_ENABLE_CHANGED;
    }
    return ret;
}

static void afd_uninit_wrap(void *handle) {
    vfree(handle);
    return;
}

const struct VPP_AFD_Func_Ptr gAFD_Func = {
    afd_init_wrap,
    afd_info_get_wrap,
    afd_uninit_wrap,
};

/*********************************************************
 * /sys/class/afd_module APIs
 *********************************************************/
static ssize_t debug_show(struct class *class, struct class_attribute *attr,
                          char *buf) {
    ssize_t size = 0;

    size = sprintf(buf, "%u\n", afd_debug_flag);

    return size;
}

static ssize_t debug_store(struct class *class, struct class_attribute *attr,
                           const char *buf, size_t size) {
    unsigned int val;
    ssize_t ret;

    val = 0;
    ret = kstrtoint(buf, 0, &val);
    if (ret != 0) return -EINVAL;

    afd_debug_flag = val;

    return size;
}

static ssize_t m5_res_show(struct class *class, struct class_attribute *attr,
                         char *buf) {
    ssize_t size;
    char tmp_buf[1024];

    print_m5_res(tmp_buf, 1024);
    size = sprintf(buf, "%s\n", tmp_buf);

    return size;
}

static ssize_t m5_res_store(struct class *cla, struct class_attribute *attr,
                             const char *buf, size_t count) {
    int parsed[3];

    if (likely(parse_para(buf, 3, parsed) == 3)) {
        int path = parsed[0];
        VT_NODE_t *vt = find_vtc(path);
        if (!vt) {
            vt = create_vtc(path);
        }
        S_VT_CONVERSION_STATE* vt_context = vt ? &(vt->vtc) : NULL;
        if (vt_context) {
            VT_SetMhegScalingResolution(vt_context, parsed[1], parsed[2]);
        } else {
            pr_err("[AFD]: %s- no %d exist\n", __func__, path);
        }
    }
    return strnlen(buf, count);
}

static ssize_t m5_aspect_show(struct class *class, struct class_attribute *attr,
                          char *buf) {
    ssize_t size;
    char tmp_buf[1024];

    print_m5_aspect(tmp_buf, 1024);
    size = sprintf(buf, "%s\n", tmp_buf);

    return size;
}

static ssize_t m5_aspect_store(struct class *class, struct class_attribute *attr,
                           const char *buf, size_t size) {
    int parsed[2];

    if (likely(parse_para(buf, 2, parsed) == 2)) {
        int path = parsed[0];
        VT_NODE_t *vt = find_vtc(path);
        if (!vt) {
            vt = create_vtc(path);
        }
        S_VT_CONVERSION_STATE* vt_context = vt ? &(vt->vtc) : NULL;
        if (vt_context) {
            VT_SetMhegAspectRatio(vt_context, parsed[1]);
        } else {
            pr_err("[AFD]: %s- no %d exist\n", __func__, path);
        }
    }

    return size;
}

static ssize_t m5_wam_show(struct class *class, struct class_attribute *attr,
                          char *buf) {
    ssize_t size;
    char tmp_buf[1024];

    print_m5_wam(tmp_buf, 1024);
    size = sprintf(buf, "%s\n", tmp_buf);

    return size;
}

static ssize_t m5_wam_store(struct class *class, struct class_attribute *attr,
                           const char *buf, size_t size) {
    int parsed[2];

    if (likely(parse_para(buf, 2, parsed) == 2)) {
        int path = parsed[0];
        VT_NODE_t *vt = find_vtc(path);
        if (!vt) {
            vt = create_vtc(path);
        }
        S_VT_CONVERSION_STATE* vt_context = vt ? &(vt->vtc) : NULL;
        if (vt_context) {
            if (parsed[1] <= ASPECT_MODE_CUSTOM)
                VT_SetMhegVideoAlignment(vt_context, parsed[1]);
        } else {
            pr_err("[AFD]: %s- no %d exist\n", __func__, path);
        }
    }

    return size;
}


static ssize_t value_show(struct class *class, struct class_attribute *attr,
                          char *buf) {
    ssize_t size;
    char tmp_buf[1024];

    print_afd_value(tmp_buf, 1024, (afd_debug_flag ? afd_debug_value_flag : 0xff));
    size = sprintf(buf, "%s%s\n", (afd_debug_flag ? "Debug on:\n" : ""), tmp_buf);

    return size;
}

static ssize_t value_store(struct class *class, struct class_attribute *attr,
                           const char *buf, size_t size) {
    unsigned int val;
    ssize_t ret;

    val = 0;
    ret = kstrtoint(buf, 0, &val);
    if (ret != 0) return -EINVAL;

    //only support set in debug mode
    if (afd_debug_flag)
        afd_debug_value_flag = val;

    return size;
}

static ssize_t type_show(struct class *class, struct class_attribute *attr,
                         char *buf) {
    ssize_t size = 0;
    int scaling_type = SCALING_NONE;

    //Only return the scaling type of main path for app using
    //If want to debug other paths, please read "state"
    VT_NODE_t *vt = find_vtc(0);
    S_VT_CONVERSION_STATE* vt_context = vt ? &(vt->vtc) : NULL;
    if (vt_context)
        scaling_type = vt_context->scaling_mode;

    size = sprintf(buf, "%d\n", scaling_type);

    return size;
}

static ssize_t type_store(struct class *class, struct class_attribute *attr,
                          const char *buf, size_t size) {
    unsigned int val;
    int parsed[2];

    if (likely(parse_para(buf, 2, parsed) == 2)) {
        int path = parsed[0];
        VT_NODE_t *vt = find_vtc(path);
        if (!vt) {
            vt = create_vtc(path);
        }
        S_VT_CONVERSION_STATE* vt_context = vt ? &(vt->vtc) : NULL;
        if (vt_context) {
            val = parsed[1];
            if (val <= SCALING_MHEG)
                VT_SetScalingMode(vt_context, val);
        } else {
            pr_err("[AFD]: %s- no %d exist\n", __func__, path);
        }
    }

    return size;
}

static ssize_t enable_show(struct class *class, struct class_attribute *attr,
                           char *buf) {
    ssize_t size;
    char tmp_buf[1024];

    print_enable_value(tmp_buf, 1024);
    size = sprintf(buf, "%s\n", tmp_buf);

    return size;
}

static ssize_t enable_store(struct class *class, struct class_attribute *attr,
                            const char *buf, size_t size) {
    int parsed[3];
    S_VT_CONVERSION_STATE* vt_context;

    if (likely(parse_para(buf, 3, parsed) == 3)) {
        int path = parsed[0];
        int inst_id = parsed[1];
        int enable = parsed[2];
        VT_NODE_t *vt = find_vtc(path);
        if (enable) {
            if (!vt) {
                vt = create_vtc(path);
            }
            if (vt)
                vt->inst_id = inst_id;
            vt_context = vt ? &(vt->vtc) : NULL;
            if (vt_context) VT_Enter(vt_context);
            if (afd_debug_flag) {
                pr_err("[AFD] enter afd");
            }
        } else {
            if (vt) {
                S_VT_CONVERSION_STATE* vt_context = vt ? &(vt->vtc) : NULL;
                if (vt_context) VT_Leave(vt_context);
                if (release_vtc(path) != path) {
                    pr_err("[AFD]: %s- no %d exist\n", __func__, path);
                }
                if (afd_debug_flag) {
                    pr_err("[AFD] leave afd");
                }
            }
        }
    }

    return size;
}

static ssize_t scaling_show(struct class *cla, struct class_attribute *attr,
                            char *buf) {
    ssize_t size;
    char tmp_buf[1024];

    print_scaling_value(tmp_buf, 1024);
    size = sprintf(buf, "%s\n", tmp_buf);

    return size;
}

static ssize_t scaling_store(struct class *cla, struct class_attribute *attr,
                             const char *buf, size_t count) {
    update_scaling_rect(buf);
    return strnlen(buf, count);
}

static ssize_t aspect_mode_show(struct class *cla, struct class_attribute *attr,
                            char *buf) {
    ssize_t size;
    char tmp_buf[1024];

    print_aspect_mode(tmp_buf, 1024);
    size = sprintf(buf, "%s\n", tmp_buf);

    return size;
}

static ssize_t aspect_mode_store(struct class *cla, struct class_attribute *attr,
                             const char *buf, size_t size) {
    unsigned int val;
    int parsed[2];

    if (likely(parse_para(buf, 2, parsed) == 2)) {
        int path = parsed[0];
        VT_NODE_t *vt = find_vtc(path);
        if (!vt) {
            vt = create_vtc(path);
        }
        S_VT_CONVERSION_STATE* vt_context = vt ? &(vt->vtc) : NULL;
        if (vt_context) {
            val = parsed[1];
            if (val <= ASPECT_MODE_CUSTOM)
                VT_SetVideoAlignmentPref(vt_context, val);
        }
    }

    return size;
}

static ssize_t state_show(struct class *cla, struct class_attribute *attr,
                            char *buf) {
    ssize_t size;
    char* tmp_buf = kzalloc(10240, GFP_KERNEL);

    if (tmp_buf) {
        print_vt_states(tmp_buf, 10240);
        size = sprintf(buf, "%s\n", tmp_buf);
        kfree(tmp_buf);
    }

    return size;
}

static ssize_t state_store(struct class *cla, struct class_attribute *attr,
                             const char *buf, size_t size) {
    return size;
}

#if AFD_BUILD_4_9
static struct class_attribute afd_module_class_attrs[] = {
    __ATTR(debug, 0664, debug_show, debug_store),
    __ATTR(m5_res, 0664, m5_res_show, m5_res_store),
    __ATTR(m5_aspect, 0664, m5_aspect_show, m5_aspect_store),
    __ATTR(m5_wam, 0664, m5_wam_show, m5_wam_store),
    __ATTR(value, 0664, value_show, value_store),
    __ATTR(type, 0664, type_show, type_store),
    __ATTR(enable, 0664, enable_show, enable_store),
    __ATTR(scaling, 0664, scaling_show, scaling_store),
    __ATTR(aspect_mode, 0664, aspect_mode_show, aspect_mode_store),
    __ATTR(state, 0664, state_show, state_store),
    __ATTR_NULL
};

static struct class afd_module_class = {
    .name = CLS_NAME,
    .class_attrs = afd_module_class_attrs,
};
#else
static CLASS_ATTR_RW(debug);
static CLASS_ATTR_RW(m5_res);
static CLASS_ATTR_RW(m5_aspect);
static CLASS_ATTR_RW(m5_wam);
static CLASS_ATTR_RW(value);
static CLASS_ATTR_RW(type);
static CLASS_ATTR_RW(enable);
static CLASS_ATTR_RW(scaling);
static CLASS_ATTR_RW(aspect_mode);
static CLASS_ATTR_RW(state);

static struct attribute *afd_module_class_attrs[] = {&class_attr_debug.attr,
                                                     &class_attr_m5_res.attr,
                                                     &class_attr_m5_aspect.attr,
                                                     &class_attr_m5_wam.attr,
                                                     &class_attr_value.attr,
                                                     &class_attr_type.attr,
                                                     &class_attr_enable.attr,
                                                     &class_attr_scaling.attr,
                                                     &class_attr_aspect_mode.attr,
                                                     &class_attr_state.attr,
                                                     NULL};

ATTRIBUTE_GROUPS(afd_module_class);

static struct class afd_module_class = {
    .name = CLS_NAME,
    .class_groups = afd_module_class_groups,
};
#endif

/*********************************************************
 * /dev/afd_module APIs
 *********************************************************/
static int afd_module_open(struct inode *inode, struct file *file) { return 0; }

static int afd_module_release(struct inode *inode, struct file *file) {
    return 0;
}

static long afd_module_ioctl(struct file *file, unsigned int cmd, ulong arg) {
    long ret = 0;
    struct afd_ctl *user_argp = (void __user *)arg;
    struct afd_ctl argp;

    if (IS_ERR_OR_NULL(user_argp)) return -EINVAL;
    memset(&argp, 0, sizeof(struct afd_ctl));

    switch (cmd) {
        case AFD_IOCTL_CMD_SET: {
            ret = copy_from_user(argp.name, user_argp->name,
                                 sizeof(argp.name) - 1);
            argp.name[sizeof(argp.name) - 1] = '\0';
            ret |=
                copy_from_user(argp.val, user_argp->val, sizeof(argp.val) - 1);
            argp.val[sizeof(argp.val) - 1] = '\0';
            if (ret)
                ret = -EINVAL;
            else
                ret = 0;
        } break;
        case AFD_IOCTL_CMD_GET:
            /* just a test */
            break;
        default:
            return -EINVAL;
    }
    return ret;
}

#ifdef CONFIG_COMPAT
static long afd_module_compat_ioctl(struct file *file, unsigned int cmd,
                                    ulong arg) {
    long ret = 0;

    ret = afd_module_ioctl(file, cmd, (ulong)compat_ptr(arg));
    return ret;
}
#endif
static const struct file_operations afd_module_fops = {
    .owner = THIS_MODULE,
    .open = afd_module_open,
    .release = afd_module_release,
    .unlocked_ioctl = afd_module_ioctl,
#ifdef CONFIG_COMPAT
    .compat_ioctl = afd_module_compat_ioctl,
#endif
    .poll = NULL,
};

static int afd_drv_init(void) {
    int error;

    error = class_register(&afd_module_class);
    if (error) {
        pr_err("%s: class_register failed\n", __func__);
        return error;
    }

    /* create afd device */
    error = register_chrdev(AFD_MAJOR, "afd_module", &afd_module_fops);
    if (error < 0) {
        pr_err("Can't register major for afd device\n");
        return error;
    }
    afd_modue_dev = device_create(&afd_module_class, NULL, MKDEV(AFD_MAJOR, 0),
                                  NULL, DEV_NAME);
    init_vt_context();
    afd_debug_value_flag = 0xff;
    return error;
}

static void afd_drv_exit(void) {
    class_unregister(&afd_module_class);
    unregister_chrdev(AFD_MAJOR, DEV_NAME);
}

static int __init amlogic_afd_module_init(void) {
    afd_drv_init();
    return register_afd_function((struct VPP_AFD_Func_Ptr *)&gAFD_Func,
                                 afd_version_str);
}

static void __exit amlogic_afd_module_exit(void) {
    unregister_afd_function((struct VPP_AFD_Func_Ptr *)&gAFD_Func);
    afd_drv_exit();
}
module_init(amlogic_afd_module_init);
module_exit(amlogic_afd_module_exit);

MODULE_DESCRIPTION("Amlogic AFD module Driver");
MODULE_AUTHOR("Amlogic SH Display team");
MODULE_LICENSE("GPL");
