include ../common.mk
include Makefile.core

ROOTBINDIR = $(BINROOT)

INCS += -I$(PREL_PATH)/include/upnp -I../../WeMo_CrockPot/hal
LIBS += -L$(PREL_PATH)/lib

OBJS = $(patsubst %.cpp,%.o,$(wildcard *.cpp)) $(patsubst %.c,%.o,$(wildcard *.c))
all : htf 

htf : makedir $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS)  -o $(OUTPUT) $(OBJS)
	$(CC) -shared -o ${ROOTBINDIR}/libDeviceControl.so $(OBJS)
	
makedir :
#	-mkdir -p ${OUTDIR}
install:
	cp -f *.h $(PREL_PATH)/include/plug-in/
	cp -f ${ROOTBINDIR}/libDeviceControl.so $(PREL_PATH)/lib/
clean :
	rm -f ${OBJS}
	rm -f ./DeviceControl.o
	rm -f ${ROOTBINDIR}/libDeviceControl.so
	rm -f $(PREL_PATH)/lib/libDeviceControl.so

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
