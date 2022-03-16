#ifeq ($(KERNEL_A32_SUPPORT), true)
#KERNEL_ARCH := arm
#CROSS_COMPILE := /opt/gcc-linaro-6.3.1-2017.02-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
#else
#KERNEL_ARCH := arm64
#CROSS_COMPILE := /opt/gcc-linaro-6.3.1-2017.02-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
#endif

#ifeq ($(KERNEL_A32_SUPPORT), true)
#KERNEL_ARCH ?= arm
#CROSS_COMPILE ?= arm-linux-gnueabihf-
#else
#KERNEL_ARCH ?= arm64
#CROSS_COMPILE ?= aarch64-linux-gnu-
#endif

AFD_IN=vendor/amlogic/common/external/DTVKit/AFD
AFD_OUT=$(PRODUCT_OUT)/obj/afd_module

define afd-module
$(AFD_MODULE_KO):
	rm $(AFD_OUT) -rf
	mkdir -p $(AFD_OUT)
	cp $(AFD_IN)/* $(AFD_OUT)/ -airf
	@echo "make Amlogic AFD module KERNEL_ARCH is $(KERNEL_ARCH)"
	PATH=$(KERNEL_TOOLPATHS):$$PATH $(MAKE) -C $(shell pwd)/$(PRODUCT_OUT)/obj/KERNEL_OBJ M=$(shell pwd)/$(AFD_OUT)/ $(KERNEL_ARGS) CONFIG_AFD_MODULE=m modules

	mkdir -p $(PRODUCT_OUT)/obj/lib_vendor
	rm $(PRODUCT_OUT)/obj/lib_vendor/afd_module.ko -f
	cp $(AFD_OUT)/afd_module.ko $(PRODUCT_OUT)/obj/lib_vendor/afd_module.ko -airf
	@echo "make Amlogic AFD module finished current dir is $(shell pwd)"
endef
