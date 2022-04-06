ifeq ($(KERNEL_A32_SUPPORT), true)
KERNEL_ARCH := arm
else
KERNEL_ARCH := arm64
endif
SUBDIR = ./AFD
INCLUDE += /usr/include
#KDIR := $(shell pwd)/$(PRODUCT_OUT)/obj/KERNEL_OBJ/
#AFD_MODULES := $(shell pwd)/$(PRODUCT_OUT)/obj/afd_module

ifeq (true, $(TARGET_BUILD_KERNEL_4_9))
AFD_BUILD_4_9=1
else
AFD_BUILD_4_9=0
endif

CONFIGS := CONFIG_AFD_MODULE=m

CONFIGS_BUILD := -Wno-undef -Wno-pointer-sign \
		-Wno-unused-const-variable \
		-Wimplicit-function-declaration \
		-Wno-unused-function

KBUILD_CFLAGS_MODULE += $(GKI_EXT_MODULE_PREDEFINE)

afd_module-objs = $(SUBDIR)/AFDparse.o \
				  $(SUBDIR)/vtc.o \
				  $(SUBDIR)/vtm.o \
				  afd_main.o

obj-$(CONFIG_AFD_MODULE) += afd_module.o

modules:
	$(MAKE) -C $(KERNEL_SRC) M=$(M) modules ARCH=$(KERNEL_ARCH)  "EXTRA_CFLAGS+=-I$(INCLUDE) -Wno-error -I$(EXTRA_CFLAGS1) -DAFD_BUILD_4_9=$(AFD_BUILD_4_9) $(CONFIGS_BUILD) $(EXTRA_INCLUDE) $(KBUILD_CFLAGS_MODULE)" $(CONFIGS)

all:modules

modules_install:
	$(MAKE) INSTALL_MOD_STRIP=1 M=$(M) -C $(KERNEL_SRC) modules_install
	mkdir -p ${OUT_DIR}/../vendor_lib/modules
	cd ${OUT_DIR}/$(M)/; find -name "*.ko" -exec cp {} ${OUT_DIR}/../vendor_lib/modules/ \;

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(M) clean
