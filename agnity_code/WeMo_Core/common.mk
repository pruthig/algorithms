#PLEASE change you board type here and check in to SVN, make build from CC and you will get image you want
#EVB, EVT, DVT

# The versioning schema for the EMS firmware is as follows: A.B.C.D.
# A: Major Release
# B: Minor Release
# C: Patch Release
# D: build revision
# build revision is generated automatically, The A.B.C parts are defined here:
VER_MAJOR=2
VER_MINOR=00
VER_PATCH=0

BUILD_TYPE=B

ifeq ($(BUILD_TYPE),A)
GEMTEK_SDK_NAME=Belkin_Plug-Ins_20120511_v0.00.11
endif
ifeq ($(BUILD_TYPE),B)
GEMTEK_SDK_NAME=Belkin_Plug-Ins_20111115_v0.00.10
endif
ifeq ($(BUILD_TYPE),S)
GEMTEK_SDK_NAME=Belkin_Plug-Ins_20111115_v0.00.10
endif

CURRENT_PRODUCT=$(shell cat ../../.current_product)

ifeq ($(CURRENT_PRODUCT),WeMo_NetCam)
#override the value with NetCam product
GEMTEK_SDK_NAME=NetCam_SDK
endif

ifeq ($(CURRENT_PRODUCT),WeMo_Baby)
GEMTEK_SDK_NAME=Belkin_Baby_Monitor_20120725_v0.00.01
endif

SDKPATH=$(PLUGIN_ROOT)/$(BRANCH_PATH)/$(GEMTEK_SDK_NAME)
SOURCEPATH=$(SDKPATH)/user/pluginApp
SOURCEPATH_WWW=$(SDKPATH)/user/www
LINUX_SOURCE_PATH = $(SDKPATH)/linux-2.6.21.x
BUSYBOX_SOURCE_PATH = $(SDKPATH)/user/busybox
PREL_PATH=$(PLUGIN_ROOT)/../$(PRODUCT)_build$(BRANCH_SUFFIX)/prel
BINROOT=$(PLUGIN_ROOT)/$(BRANCH_PATH)/Plugin/src/bin
PLATFORM=LINUX 

ifeq ($(BUILD_TYPE),S)
export ENABLE_ROBUST_MUTEX=N
else
export ENABLE_ROBUST_MUTEX=Y
endif

ifeq ($(CURRENT_PRODUCT),WeMo_NetCam)
export ENABLE_ROBUST_MUTEX=N
export NETCAM_SDK=Y
endif

export STRIP=mipsel-linux-strip
export BOARD=DVT

ifeq ($(BOARD),EVB)
	export PLUGIN_BOARD_TYPE=EVB
endif

ifeq ($(BOARD),EVT)
	export PLUGIN_BOARD_TYPE=EVT
endif

ifeq ($(BOARD),DVT)
	export PLUGIN_BOARD_TYPE=DVT
endif

ifeq ($(BOARD),PVT)
	export PLUGIN_BOARD_TYPE=PVT
endif

ifeq ($(BOARD),QAB)
	export PLUGIN_BOARD_TYPE=QAB
endif

ifeq ($(PLUGIN_BOARD_TYPE),EVB)
	export PLUGIN_BOARD=OLD
else
	export PLUGIN_BOARD=NEW
endif



#FROM MAIN MAKEFILE
export PROJECT=RALINK
export WL_DOMAIN=FCC
ifeq ($(BUILD_TYPE),A)
export COUNTRY=WW
endif
ifeq ($(BUILD_TYPE),B)
export COUNTRY=US
endif
ifeq ($(BUILD_TYPE),S)
export COUNTRY=US
endif
export MODEL=BELKIN
export VENDOR=WL
export PHY=RTL8366
export FWPREFIX=WeMo

ifeq ($(PLUGIN_BOARD_TYPE),QAB)
export QAB=Y
endif

ifeq ($(PLUGIN_BOARD_TYPE),PVT)
export ENABLE_DEBUG=N
else
export ENABLE_DEBUG=Y
endif

FW_VERSION1 = $(VER_MAJOR).$(VER_MINOR)

FW_REV:= $(shell \
           cd ../; svn info | grep "Revision:" | \
           sed "s/Revision: \(.*\)/\1/"\
          )

FW_VERSION = $(FW_VERSION1).$(FW_REV).$(PLUGIN_BOARD_TYPE)
