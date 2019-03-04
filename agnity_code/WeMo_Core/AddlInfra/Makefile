include ../common.mk
include Makefile.core

ROOTBINDIR = $(BINROOT)

INCS += -I$(PREL_PATH)/include -I$(BUSYBOX_SOURCE_PATH)/libbb/

LIB += -lnvram

ifeq ($(NetCam_SDK),Y)
LIB += -lbelkin_api
else
LIB += -lgemtek_api
LIBROOT=$(SDKPATH)/user/nvram/nvram -L$(SDKPATH)/user/gemtek_api/gemtek_api
endif

LIBS += -L${LIBROOT} -L$(PREL_PATH)/lib
CFLAGS += -pthread
LDFLAGS += -pthread
OBJS = $(patsubst %.cpp,%.o,$(wildcard *.cpp)) $(patsubst %.c,%.o,$(wildcard *.c))
all : htf 

htf : makedir $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS)  -o $(OUTPUT) $(OBJS)
	$(CC) -shared -o ${ROOTBINDIR}/libAddlInfra.so $(OBJS)
	
makedir :
#	-mkdir -p ${OUTDIR}
install:
	cp -f *.h $(PREL_PATH)/include/plug-in/
	cp -f ${ROOTBINDIR}/libAddlInfra.so $(PREL_PATH)/lib/
clean :
	rm -f ./AddlInfra.o
	rm -f ${ROOTBINDIR}/libAddlInfra.so
	rm -f $(PREL_PATH)/lib/libAddlInfra.so
	rm -f ${OBJS}

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
