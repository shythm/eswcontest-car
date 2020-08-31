cmake_minimum_required(VERSION 3.10)

# Set compilor path and root filesystem path
# ${CMAKE_SOURCE_DIR} : Current directory
SET(CP_PATH ${CMAKE_SOURCE_DIR}/../../CrossCompiler)
SET(ROOTFS ${CP_PATH}/sysroots/cortexa15t2hf-vfp-neon-linux-gnueabi)

# Set cross compilor path
SET(CROSS_COMPILE ${CP_PATH}/gcc-linaro-arm-linux-gnueabihf/bin/arm-linux-gnueabihf-)

# Set C cross compilor and C++ cross compilor path
SET(CMAKE_C_COMPILER ${CROSS_COMPILE}gcc)
SET(CMAKE_CXX_COMPILER ${CROSS_COMPILE}g++)

# Include libraries
INCLUDE_DIRECTORIES(protocol)
INCLUDE_DIRECTORIES(${ROOTFS}/include)
INCLUDE_DIRECTORIES(${ROOTFS}/usr/include)
INCLUDE_DIRECTORIES(${ROOTFS}/usr/include/omap)
INCLUDE_DIRECTORIES(${ROOTFS}/usr/include/libdrm)
INCLUDE_DIRECTORIES(${ROOTFS}/usr/include/gbm)

# Dynamic library path
LINK_DIRECTORIES(${ROOTFS}/usr/lib)
LINK_DIRECTORIES(${ROOTFS}/usr/src)
LINK_DIRECTORIES(${ROOTFS}/lib)

# Include custom libraries
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/../shared)

# Build executable file
SET(object recognize)
ADD_EXECUTABLE(${object} recognize.c recognize-lib.c display-kms.c lane-detection.cpp recognize-update.c v4l2.c vpe-common.c ../shared/ctrlboard-lib.c ../shared/util.c)
TARGET_LINK_LIBRARIES(${object} pthread m jpeg rt drm mtdev drm_omap opencv_core opencv_highgui opencv_imgproc opencv_objdetect)
SET_PROPERTY(TARGET ${object} PROPERTY C_STANDARD 99)
