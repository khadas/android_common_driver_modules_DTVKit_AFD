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

#
# This file is the build configuration for a full Android
# build for Meson reference board.
#


#!!This file is for build afd source with kernel source in android p.Please refer to device/amlogic/${PRODUCT}/Kernel.mk
#!!For the higher android versions (R,S ..), please Don't INCLUDE OR INHERIT this file, please refer to device/amlogic/${PRODUCT}/build.config.meson.*

#BOARD_VENDOR_KERNEL_MODULES += $(PRODUCT_OUT)/obj/lib_vendor/afd_module.ko

AFD_SRC_DIR := $(call my-dir)
AFD_OUT := $(PRODUCT_OUT)/obj/afd_module

define afd-module
	rm $(AFD_OUT) -rf
	mkdir -p $(AFD_OUT)
	cp $(AFD_SRC_DIR)/* $(AFD_OUT)/ -airf
	$(MAKE) -C $(KERNEL_OUT) M=$(shell pwd)/$(AFD_OUT) CROSS_COMPILE=$(PREFIX_CROSS_COMPILE) ARCH=$(KERNEL_ARCH) CONFIG_AFD_MODULE=m modules
	mkdir -p $(PRODUCT_OUT)/obj/lib_vendor
	rm $(PRODUCT_OUT)/obj/lib_vendor/afd_module.ko -f
	cp $(AFD_OUT)/afd_module.ko $(PRODUCT_OUT)/obj/lib_vendor/afd_module.ko -airf
	$(PREFIX_CROSS_COMPILE)strip $(PRODUCT_OUT)/obj/lib_vendor/afd_module.ko --strip-debug
endef
