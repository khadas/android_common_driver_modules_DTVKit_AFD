#include <sys/ioctl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "afd_datatype.h"

#define AFD_DEV "/dev/aml_afd"

static int afd_dev;

static void printHelp() {
    printf("afd_test create  [path] [inst]\n");
    printf("afd_test release [path]\n");
    printf("afd_test update  aspect [0/5]\n");
    printf("afd_test update  scale  [path] [type] \
        [left] [top] [width] [height] [resolution_width] [resolution_height]\n");
    printf("afd_test list    paths\n");
    printf("afd_test overscan\n");
    printf("afd_test state   [path]\n");
}

static void create_afd_context(int path, int instance_id) {
    if (afd_dev != -1) {
        struct afd_ctl_create_t iop;
        iop.path = path;
        iop.instance_id = instance_id;
        if (ioctl(afd_dev, AFD_IOCTl_CREATE_CONTEXT, &iop) != -1) {
            printf("%s:ioctl success\n", __FUNCTION__);
        } else {
            printf("%s:ioctl failed: %s\n", __FUNCTION__, strerror(errno));
        }
    }
}

static void release_afd_context(int path) {
    if (afd_dev != -1) {
        if (ioctl(afd_dev, AFD_IOCTL_RELEASE_CONTEXT, &path) != -1) {
            printf("%s:ioctl success\n", __FUNCTION__);
        } else {
            printf("%s:ioctl failed: %s\n", __FUNCTION__, strerror(errno));
        }
    }
}

static void set_afd_aspect(int aspect) {
    if (afd_dev != -1 && (aspect == 0 || aspect == 5)) {
        if (ioctl(afd_dev, AFD_IOCTL_SET_ASPECT, &aspect) != -1) {
            printf("%s:ioctl success\n", __FUNCTION__);
        } else {
            printf("%s:ioctl failed: %s\n", __FUNCTION__, strerror(errno));
        }
    }
}

static void set_afd_scaling(struct afd_ctl_scaling_t *asc) {
    if (afd_dev != -1) {
        if (ioctl(afd_dev, AFD_IOCTL_SET_SCALE_TYPE, asc) != -1) {
            printf("%s:ioctl success\n", __FUNCTION__);
        } else {
            printf("%s:ioctl failed: %s\n", __FUNCTION__, strerror(errno));
        }
    }
}

static void print_afd_paths(void) {
    if (afd_dev != -1) {
        struct afd_recv_list_t l;
        if (ioctl(afd_dev, AFD_IOCTL_GET_PATHS, &l) != -1) {
            printf("%s:ioctl success\n", __FUNCTION__);
            if (l.size > 0) {
                printf("[");
                for (int i=0; i<l.size; i++) {
                    printf(" %d ", l.list[i]);
                }
                printf("]");
            }
        } else {
            printf("%s:ioctl failed: %s\n", __FUNCTION__, strerror(errno));
        }
    }
}

static void test_afd_overscan(void) {
    int test_hs = 16;
    int test_vs = 17;
    if (afd_dev != -1) {
        struct afd_ctl_overscan_t os;
        os.uhd_overscan.a = test_hs;
        os.uhd_overscan.b = test_vs;
        os.uhd_overscan.c = test_hs;
        os.uhd_overscan.d = test_vs;
        memcpy(&(os.fhd_overscan), &(os.uhd_overscan), sizeof(struct afd_rect_t));
        memcpy(&(os.hd_overscan), &(os.uhd_overscan), sizeof(struct afd_rect_t));
        memcpy(&(os.sd_overscan), &(os.uhd_overscan), sizeof(struct afd_rect_t));
        if (ioctl(afd_dev, AFD_IOCTL_SET_OVERSCAN, &os) != -1) {
            printf("%s:ioctl success\n", __FUNCTION__);
        } else {
            printf("%s:ioctl failed: %s\n", __FUNCTION__, strerror(errno));
        }
    }
}

static void print_afd_state(int path) {
    if (afd_dev != -1) {
        struct afd_recv_state_t st;
        st.valid = 0;
        st.path = path;
        if (ioctl(afd_dev, AFD_IOCTL_GET_STATE, &st) != -1) {
            printf("%s:ioctl success\n", __FUNCTION__);
            if (st.valid) {
                printf("AFD: path %d, inst %d\n", st.path, st.instance_id);
                printf("  enable : %u\n", st.enable);
                printf("  value  : %u\n", st.afd_value);
                printf("  type   : %u(0 auto, 1 4:3, 2 16:9, 3 14:9, 4 zoom, 5:cus)\n",
                    st.scaling.type);
                printf("  scaling: %d %d %d %d\n",
                    st.scaling.scaling_rect.a, st.scaling.scaling_rect.b,
                    st.scaling.scaling_rect.c, st.scaling.scaling_rect.d);
                printf("  res    : %u %u\n", st.scaling.resolution_width, st.scaling.resolution_height);
                printf("  video  : %u %u\n", st.video_width, st.video_height);
                printf("  screen : %u %u\n", st.screen_width, st.screen_height);
                printf("  v ar   : %u(0 4:3, 1 16:9 255 und)\n", st.video_aspect);
                printf("  v out  : %u %u %u %u\n", st.video_out.a, st.video_out.b,
                    st.video_out.c, st.video_out.d);
                printf("  dis out: %u %u %u %u\n", st.display_out.a, st.display_out.b,
                    st.display_out.c, st.display_out.d);
            }
        } else {
            printf("%s:ioctl failed: %s\n", __FUNCTION__, strerror(errno));
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("afd_test: too few args...\n");
        printHelp();
        return 0;
    }

    afd_dev = open(AFD_DEV, O_RDWR | O_NONBLOCK);
    if (afd_dev == -1) {
        printf("afd_test: can not open afd device(%s)\n", strerror(errno));
        return -1;
    }

    char *command = argv[1];
    if (strcmp("create", command) == 0) {
        if (argc == 4) {
            int path = atoi(argv[2]);
            int instance_id = atoi(argv[3]);
            create_afd_context(path, instance_id);
        } else {
            printHelp();
        }
    } else if (strcmp("release", command) == 0){
        if (argc == 3) {
            int path = atoi(argv[2]);
            release_afd_context(path);
        } else {
            printHelp();
        }
    } else if (strcmp("update", command) == 0 && argc > 2) {
        char *type = argv[2];
        if (strcmp("aspect", type) == 0) {
            if (argc == 4) {
                int aspect = atoi(argv[3]);
                set_afd_aspect(aspect);
            } else {
                printHelp();
            }
        } else if (strcmp("scale", type) == 0) {
            if (argc == 11) {
                struct afd_ctl_scaling_t st;
                st.path = atoi(argv[3]);
                st.scaling.type = atoi(argv[4]);
                st.scaling.scaling_rect.a = atoi(argv[5]);
                st.scaling.scaling_rect.b = atoi(argv[6]);
                st.scaling.scaling_rect.c = atoi(argv[7]);
                st.scaling.scaling_rect.d = atoi(argv[8]);
                st.scaling.resolution_width = atoi(argv[9]);
                st.scaling.resolution_height = atoi(argv[10]);
                set_afd_scaling(&st);
            } else {
                printHelp();
            }
        }
    }else if (strcmp("list", command) == 0) {
        print_afd_paths();
    } else if (strcmp("overscan", command) == 0) {
        test_afd_overscan();
    } else if (strcmp("state", command) == 0) {
        if (argc == 3) {
            int path = atoi(argv[2]);
            print_afd_state(path);
        } else {
            printHelp();
        }
    } else {
        printHelp();
    }

    if (afd_dev != -1) {
        close(afd_dev);
    }
    return 0;
}
