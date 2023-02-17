# Copyright (C) 2015 Amlogic, Inc. All rights reserved.
#
# All information contained herein is Amlogic confidential.
#
# This software is provided to you pursuant to Software License
# Agreement (SLA) with Amlogic Inc ("Amlogic"). This software may be
# used only in accordance with the terms of this agreement.
#
# Redistribution and use in source and binary forms, with or without
# modification is strictly prohibited without prior written permission
# from Amlogic.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


ifeq ($(KERNEL_A32_SUPPORT), true)
KERNEL_ARCH := arm
else
KERNEL_ARCH := arm64
endif
SUBDIR = ./AFD
#INCLUDE += /usr/include
#KDIR := $(shell pwd)/$(PRODUCT_OUT)/obj/KERNEL_OBJ/
#AFD_MODULES := $(shell pwd)/$(PRODUCT_OUT)/obj/afd_module

AFD_BUILD_4_9 := 0

ifeq (4.9, $(TARGET_BUILD_KERNEL_VERSION))
AFD_BUILD_4_9 := 1
else
ifeq (true, $(TARGET_BUILD_KERNEL_5_15))
#android t + 5.15
AFD_BUILD_4_9 := 0
else
ifneq ($(TARGET_BUILD_KERNEL_4_9),)
#android r and higher
ifeq (true, $(TARGET_BUILD_KERNEL_4_9))
AFD_BUILD_4_9 := 1
endif
else
#android p
ifneq (true, $(TARGET_BUILD_KERNEL_5_4))
AFD_BUILD_4_9 := 1
endif
endif
endif
endif

EXTRA_CFLAGS += -DAFD_BUILD_4_9=$(AFD_BUILD_4_9)

AFD_CONFIGS := CONFIG_AFD_MODULE=m

CONFIGS_BUILD := -Wno-undef -Wno-pointer-sign \
		-Wno-unused-const-variable \
		-Wimplicit-function-declaration \
		-Wno-unused-function

KBUILD_CFLAGS_MODULE += $(GKI_EXT_MODULE_PREDEFINE)

afd_module-objs = $(SUBDIR)/afd_parse.o \
				  $(SUBDIR)/vtc.o \
				  $(SUBDIR)/vtm.o \
				  afd_main.o

obj-$(CONFIG_AFD_MODULE) += afd_module.o

ifeq ($(O),)
out_dir := .
else
out_dir := $(O)
endif
include $(out_dir)/include/config/auto.conf

modules:
	$(MAKE) -C $(KERNEL_SRC) M=$(M) modules ARCH=$(KERNEL_ARCH)  "-Wno-error -I$(EXTRA_CFLAGS1) $(CONFIGS_BUILD) $(EXTRA_INCLUDE) $(KBUILD_CFLAGS_MODULE)" $(AFD_CONFIGS)

all:modules

modules_install:
	$(MAKE) INSTALL_MOD_STRIP=1 M=$(M) -C $(KERNEL_SRC) modules_install
	$(Q)mkdir -p ${out_dir}/../vendor_lib/modules
	$(Q)if [ -z "$(CONFIG_AMLOGIC_KERNEL_VERSION)" ]; then \
		cd ${out_dir}/$(M)/; find -name "*.ko" -exec cp {} ${out_dir}/../vendor_lib/modules/ \; ; \
	else \
		find $(INSTALL_MOD_PATH)/lib/modules/*/$(INSTALL_MOD_DIR) -name "*.ko" -exec cp {} ${out_dir}/../vendor_lib/modules \; ; \
	fi;

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(M) clean
