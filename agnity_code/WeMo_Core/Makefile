include ../common.mk
include common.mk

SUBDIRS=AddlInfra DeviceControl NetworkControl Handlers UPnP RemoteAccess
SOURCE_TAR=${PLUGIN_ROOT}/trunk/embedded-WeMo_2_0
UPNPLIB=${PLUGIN_ROOT}/../$(PRODUCT)_build$(BRANCH_SUFFIX)/prel/lib
BINROOT=${PLUGIN_ROOT}/trunk/embedded-WeMo_2_0/Plugin/src/bin
CORE=$(CURDIR)/../WeMo_Core
FW_REV_H=$(CURDIR)/Include/fw_rev.h
FW_REV_TXT=$(CURDIR)/target/ver.txt

ifeq ($(BUILD_TYPE),A)
SOURCE_TAR_FILE=Source_A.tgz
endif

ifeq ($(BUILD_TYPE),B)
SOURCE_TAR_FILE=Source_B.tgz
endif

PROD:= $(subst WeMo_,,$(PRODUCT))

BUILD_TIME:= $(shell date "+%x %X")

ifneq ($(X86),y)
  x:= $(shell \
	if [ ! -d $(CROSS_PATH) ]; then \
	   echo "Extracting toolchain tarball..." > /dev/tty; \
	   cd $(BUILD_ROOT_TMP); tar xjf $(TOOLCHAIN_TAR); \
	fi)
  
  x:= $(shell \
	$(CROSS_PATH)/$(CC) --version > /dev/null; \
	if test $$? -eq 0; \
	then echo "$(CROSS_PATH):$(PATH)"; \
	else \
	echo "bogus"; \
	fi; \
  )

  ifeq ($(x),bogus)
    @echo "Error: unable to find working cross compiler"
    exit 1
  else
    export PATH = $(x)
  endif

	export SOURCE_PATH=$(PLUGIN_ROOT)/trunk/embedded-WeMo_2_0/$(GEMTEK_SDK_NAME)
	#export SOURCE_PATH=$(PLUGIN_ROOT)/trunk/embedded-WeMo_2_0/Source

	y:= $(shell \
	if [ ! -d $(SOURCE_PATH) ]; then \
	   echo "Extracting linux source tarball...$(SOURCE_TAR_FILE)" > /dev/tty; \
	   cd $(SOURCE_TAR); tar xzf $(SOURCE_TAR)/$(SOURCE_TAR_FILE); \
	fi)
endif


.PHONY: all

all:
	@if [ ! -d ../../../target ];then \
	   echo mkdir ../../../target; \
	   mkdir ../../../target; \
	fi
	echo "#ifndef __FW__REV__H__" > $(FW_REV_H)
	echo "#define __FW__REV__H__" >> $(FW_REV_H)
	echo "#define BUILD_TIME \"$(BUILD_TIME)\"" >> $(FW_REV_H)
	#echo "#define FW_REV \"$(VER_MAJOR).$(VER_MINOR).$(VER_PATCH).$(FW_REV)\"" >> $(FW_REV_H)
	echo "#define FW_REV \"$(FWPREFIX)_$(COUNTRY)_$(FW_VERSION)\"" >> $(FW_REV_H)
	echo "#define FW_REV1 \"$(FW_VERSION1).$(FW_REV)\"" >> $(FW_REV_H)
	echo "#endif" >> $(FW_REV_H)
	echo "" >> $(FW_REV_H)
	chmod 777 $(FW_REV_H)
	rm -f $(FW_REV_TXT)
	echo $(BUILD_TIME) >> $(FW_REV_TXT)
	echo $(FWPREFIX)_$(COUNTRY)_$(FW_VERSION) >> $(FW_REV_TXT)
	echo "PATH: $(PATH)"
	cp -f $(BINROOT)/../WeMo_Core/target/Makefile_Main $(MAINPATH)/Makefile

	echo "Copying new firmware upgrade interface files.........."
	cp -rf $(BINROOT)/../WeMo_Core/target/new_upgrade $(MAINPATH)/user
	cp -f $(BINROOT)/../WeMo_Core/target/gemtek_test $(MAINPATH)/user/gemtek_test/gemtek_test

#create build directory for current product
	@if [ ! -d ${PLUGIN_ROOT}/../$(PRODUCT)_build$(BRANCH_SUFFIX) ];then \
		mkdir ${PLUGIN_ROOT}/../$(PRODUCT)_build$(BRANCH_SUFFIX);\
	fi
#create soft link for prel pointing to current product's prel directory
	-rm -rf ${PLUGIN_ROOT}/../prel
	ln -s ${PLUGIN_ROOT}/../$(PRODUCT)_build$(BRANCH_SUFFIX)/prel ${PLUGIN_ROOT}/../prel
	bash $(CORE)/target/build-ossl.sh $(PRODUCT)_build$(BRANCH_SUFFIX)
	bash $(CORE)/target/build-libupnp.sh $(GEMTEK_SDK_NAME) $(PRODUCT)_build$(BRANCH_SUFFIX)
	bash $(CORE)/target/build-libcurl.sh $(PRODUCT)_build$(BRANCH_SUFFIX)
	bash $(CORE)/target/build-pjnath.sh $(PRODUCT)_build$(BRANCH_SUFFIX)

	list='$(SUBDIRS)'; for subdir in $$list; do \
	  echo "Making in $$subdir"; \
	  (cd $$subdir && $(MAKE)) || exit 1; \
	done;

	sudo cp $(BINROOT)/*.so $(SOURCEPATH)
#	cp $(UPNPLIB)/*.* $(SOURCEPATH)
	sudo cp -rf $(UPNPLIB)/libupnp.so.3.0.5 $(SOURCEPATH)
	sudo cp -rf $(UPNPLIB)/libthreadutil.so.2.2.3 $(SOURCEPATH)
	sudo cp -rf $(UPNPLIB)/libixml.so.2.0.4 $(SOURCEPATH)
	sudo cp -rf $(UPNPLIB)/libcurl.so.4.2.0 $(SOURCEPATH)
	cp -f $(UPNPLIB)/libssl.so.1.0.0 $(SOURCEPATH)
	cp -f $(UPNPLIB)/libcrypto.so.1.0.0 $(SOURCEPATH)
	cp -f $(UPNPLIB)/libpj-pc-linux-gnu.a $(SOURCEPATH)
	cp -f $(UPNPLIB)/libpjlib-util-pc-linux-gnu.a $(SOURCEPATH)
	cp -f $(UPNPLIB)/libpjnath-pc-linux-gnu.a $(SOURCEPATH)
#	cp -f $(BINROOT)/wemoApp $(SOURCEPATH)
	rm -rf $(SOURCEPATH)/web
	cp -rf $(BINROOT)/../WeMo_Core/UPnP/XMLs/web $(SOURCEPATH)
	cp -f $(BINROOT)/../WeMo_Core/target/Makefile_16M32M $(SOURCEPATH)/Makefile
	cp -f $(BINROOT)/../WeMo_Core/target/icon.jpg $(SOURCEPATH)
	cp -f $(BINROOT)/../WeMo_Core/target/sensor.jpg $(SOURCEPATH)

ifeq ($(WIRED_ETH),Y)
	cp -f $(BINROOT)/../WeMo_Core/target/Belkin_startup_wired.sh $(SOURCEPATH)/Belkin_startup.sh
else
	cp -f $(BINROOT)/../WeMo_Core/target/Belkin_startup.sh $(SOURCEPATH)/Belkin_startup.sh
endif

	cp -f $(BINROOT)/../WeMo_Core/target/Clean_Belkin_settings.sh $(SOURCEPATH)
	cp -f $(BINROOT)/../WeMo_Core/target/Makefile_Main $(MAINPATH)/Makefile
	cp -f $(BINROOT)/../WeMo_Core/target/ver.txt $(SOURCEPATH)
	cp -f $(BINROOT)/../WeMo_Core/target/Makefile_user $(MAINPATH)/user/Makefile
#	cp -f $(BINROOT)/../WeMo_Core/target/ralink_gpio.h $(MAINPATH)/linux-2.6.21.x/drivers/char/ralink_gpio.h
	cp -f $(BINROOT)/../WeMo_Core/target/ralink_gpio.c $(MAINPATH)/linux-2.6.21.x/drivers/char/ralink_gpio.c
	cp -f $(BINROOT)/../WeMo_Core/target/configRa0 $(MAINPATH)/user/configRa0/configRa0
	cp -f $(BINROOT)/../WeMo_Core/target/strace $(SOURCEPATH)
	cp -f $(BINROOT)/../WeMo_Core/target/list_threads.sh $(SOURCEPATH)
#ifeq ($(BUILD_TYPE),B)
#	cp -f $(BINROOT)/../WeMo_Core/target/workingmutexattrubuntu $(SOURCEPATH)
#endif
	cp -f $(BINROOT)/../WeMo_Core/target/eeprom_backup $(MAINPATH)/user/eeprom_backup/
	cp -f $(BINROOT)/../WeMo_Core/target/sysinit $(MAINPATH)/user/sysinit/
	cp -f $(BINROOT)/../WeMo_Core/target/libnvram.so $(MAINPATH)/user/nvram/nvram/libnvram.so
#	rm -rf $(MAINPATH)/user/udhcpc
#	rm -rf $(MAINPATH)/user/udhcpd
	cp -rf $(BINROOT)/../WeMo_Core/target/udhcpc $(MAINPATH)/user/
#	cp -rf $(BINROOT)/../WeMo_Core/target/udhcpd $(MAINPATH)/user/
ifeq ($(BUILD_TYPE),A)
	cp -f $(BINROOT)/../WeMo_Core/target/cmm_info_A.c $(MAINPATH)/linux-2.6.21.x/drivers/net/wireless/rt2860v2/common/cmm_info.c
	cp -f $(BINROOT)/../WeMo_Core/target/rt5350.c $(MAINPATH)/linux-2.6.21.x/drivers/net/wireless/rt2860v2/chips/
	cp -f $(BINROOT)/../WeMo_Core/target/easytest $(MAINPATH)/user/easytest/
	cp -f $(BINROOT)/../WeMo_Core/target/apcli_ctrl.c $(MAINPATH)/linux-2.6.21.x/drivers/net/wireless/rt2860v2/ap/
	cp -f $(BINROOT)/../WeMo_Core/target/rtmp_init_inf.c $(MAINPATH)/linux-2.6.21.x/drivers/net/wireless/rt2860v2/common/rtmp_init_inf.c
	cp -f $(BINROOT)/../WeMo_Core/target/nvserv $(MAINPATH)/user/nvram/nvram/nvserv
	cp -f $(BINROOT)/../WeMo_Core/target/do_mounts.c $(MAINPATH)/linux-2.6.21.x/init/do_mounts.c
	cp -f $(BINROOT)/../WeMo_Core/target/softdog.c $(MAINPATH)/linux-2.6.21.x/drivers/char/watchdog/softdog.c
	cp -f $(BINROOT)/../WeMo_Core/target/port_nvram_A $(SOURCEPATH)/port_nvram
	cp -f $(BINROOT)/../WeMo_Core/target/rtmp_def_A.h $(MAINPATH)/linux-2.6.21.x/drivers/net/wireless/rt2860v2/include/rtmp_def.h
endif
ifeq ($(BUILD_TYPE),B)
	cp -f $(BINROOT)/../WeMo_Core/target/cmm_info_B.c $(MAINPATH)/linux-2.6.21.x/drivers/net/wireless/rt2860v2/common/cmm_info.c
	cp -f $(BINROOT)/../WeMo_Core/target/port_nvram_B $(SOURCEPATH)/port_nvram
	cp -f $(BINROOT)/../WeMo_Core/target/rtmp_def_B.h $(MAINPATH)/linux-2.6.21.x/drivers/net/wireless/rt2860v2/include/rtmp_def.h
endif
#	cp -f $(BINROOT)/../WeMo_Core/target/apcli_ctrl.c $(MAINPATH)/linux-2.6.21.x/drivers/net/wireless/rt2860v2/ap/
#	cp -f $(BINROOT)/../WeMo_Core/target/ap_cfg.c $(MAINPATH)/linux-2.6.21.x/drivers/net/wireless/rt2860v2/ap/
#	cp -f $(BINROOT)/../WeMo_Core/target/ap_apcli.c $(MAINPATH)/linux-2.6.21.x/drivers/net/wireless/rt2860v2/ap/
	cp -f $(BINROOT)/../WeMo_Core/target/xgethostbyname.c $(MAINPATH)/user/busybox/libbb/

	echo "Copy WeMo Signed Public Key"
	cp -f $(BINROOT)/../WeMo_Core/target/WeMoPubKey.asc $(MAINPATH)/bin/gpg/gnupg/WeMoPubKey.asc

	echo "Making Kernel"
	$(MAKE) -C $(SDKPATH) FW_VER=${FW_VERSION} PROD=$(PROD)
	cp -f ../../../$(GEMTEK_SDK_NAME)/images/*.bin* ../../../target
	@echo ""
	@echo "Plugin components build successfully for the target"

clean:
	list='$(SUBDIRS)'; for subdir in $$list; do \
	  echo "Making clean in $$subdir"; \
	  (cd $$subdir && $(MAKE) clean); \
	done;
	rm -f $(BINROOT)/*.*
	rm -f ../../../target/*.*
	$(MAKE) clean -C $(SDKPATH) FW_VER=${FW_VERSION} PROD=$(PROD)

veryclean: clean
#	bash target/build-clean.sh
	rm -rf $(BUILD_ROOT)/$(PRODUCT)_build$(BRANCH_SUFFIX)/
	rm -rf $(BUILD_ROOT)/prel
	rm -rf $(SDKPATH)

SDK_DIR:
	if [ ! -d $(PLUGIN_ROOT) ]; then \
	$(MAKE) -C $(SDKPATH); FW_VER=${FW_VERSION} PROD=$(PROD)\
  fi
