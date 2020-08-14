CP_PATH=$(abspath ../..)/CrossCompiler
CROSS_COMPILE=$(CP_PATH)/gcc-linaro-arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
ROOTFS=$(CP_PATH)/sysroots/cortexa15t2hf-vfp-neon-linux-gnueabi

ARCH=arm
CC=$(CROSS_COMPILE)gcc
CXX=$(CROSS_COMPILE)g++

CC = ../../CrossCompiler/gcc-linaro-arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc

SHAREDINC += -I$(abspath ..)/shared
INC += -I protocol
INC += -I$(ROOTFS)/include
INC += -I$(ROOTFS)/usr/include
INC += -I$(ROOTFS)/usr/include/omap
INC += -I$(ROOTFS)/usr/include/libdrm
INC += -I$(ROOTFS)/usr/include/gbm
LIBDIR := $(ROOTFS)/usr/lib

CFLAGS := -O1 -g -Wall -fPIC -mfloat-abi=hard -mfpu=neon -std=c99 -Wl,-rpath,$(ROOTFS)/lib -Wl,-rpath,$(ROOTFS)/usr/lib $(INC)
CXXFLAGS = -Wall -ansi -g -fPIC -mfloat-abi=hard -mfpu=neon $(INC) -I$(ROOTFS)/include/c++/4.7.3/

LDFLAGS = -lm -lpthread -L$(LIBDIR) -lrt -ldrm -lmtdev -ldrm_omap -lstdc++ -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_objdetect
TARGET = recognize

all: $(TARGET)

clean:
	rm -f *.a *.o $(TARGET) *.lo

$(TARGET): recognize.c recognize-lib.o recognize-update.o v4l2.o display-kms.o util.o vpe-common.o $(abspath ..)/shared/ctrlboard-lib.o
	$(CC) $(CFLAGS) -o $@ recognize.c recognize-lib.o recognize-update.o v4l2.o display-kms.o util.o vpe-common.o $(abspath ..)/shared/ctrlboard-lib.o $(LDFLAGS) $(SHAREDINC)

v4l2.o: v4l2.c v4l2.h
	$(CC) -c $(CFLAGS) -o $@ v4l2.c $(SHAREDINC)

display-kms.o: display-kms.c display-kms.h
	$(CC) -c $(CFLAGS) -o $@ display-kms.c $(SHAREDINC)

util.o: $(abspath ..)/shared/util.c $(abspath ..)/shared/util.h
	$(CC) -c $(CFLAGS) -o $@ $(abspath ..)/shared/util.c $(SHAREDINC)

vpe-common.o: vpe-common.c vpe-common.h
	$(CC) -c $(CFLAGS) -o $@ vpe-common.c $(SHAREDINC)

recognize-lib.o: recognize-lib.c recognize-lib.h
	$(CC) -c $(CFLAGS) -o $@ recognize-lib.c $(SHAREDINC)

recognize-update.o: recognize-update.c recognize-update.h $(abspath ..)/shared/ctrlboard-lib.o
	$(CC) -c $(CFLAGS) -o $@ recognize-update.c $(abspath ..)/shared/ctrlboard-lib.o $(SHAREDINC)

$(abspath ..)/shared/ctrlboard-lib.o: $(abspath ..)/shared/ctrlboard-lib.c $(abspath ..)/shared/ctrlboard-lib.h
	$(CC) -c $(CFLAGS) -o $@ $(abspath ..)/shared/ctrlboard-lib.c $(SHAREDINC)

# drawing.o: drawing.c drawing.h font_8x8.h
# 	$(CC) -c $(CFLAGS) -o $@ drawing.c $(SHAREDINC)

# input_cmd.o: input_cmd.cpp input_cmd.h
# 	$(CXX) -c $(CXXFLAGS) -o $@ input_cmd.cpp $(SHAREDINC)

# exam_cv.o: exam_cv.cpp exam_cv.h
# 	$(CXX) -c $(CXXFLAGS) -o $@ exam_cv.cpp $(SHAREDINC)

