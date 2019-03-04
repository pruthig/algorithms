include ../common.mk
include Makefile.core

ROOTBINDIR = $(BINROOT)

INCS += -I../../WeMo_Insight/HalInsight/src

LIBS += -L$(PREL_PATH)/lib

OBJS = $(patsubst %.cpp,%.o,$(wildcard *.cpp)) $(patsubst %.c,%.o,$(wildcard *.c))
all : htf 

htf : makedir $(OBJS)

	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS)  -o $(OUTPUT) $(OBJS)
	$(CC) -shared -o ${ROOTBINDIR}/libWemoDB.so $(OBJS)
	
makedir :
#	-mkdir -p ${OUTDIR}
install:
	cp -f *.h $(PREL_PATH)/include/plug-in/
	cp -f ${ROOTBINDIR}/libWemoDB.so $(PREL_PATH)/lib/
clean :
	rm -f ${OBJS}
	rm -f ./WemoDB.o
	rm -f ${ROOTBINDIR}/libWemoDB.so
	rm -f $(PREL_PATH)/lib/libWemoDB.so

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
