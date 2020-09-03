cmake_minimum_required(VERSION 3.10)

# Set compilor path and root filesystem path
# ${CMAKE_SOURCE_DIR} : Current directory
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build)

SET(CP_PATH ${CMAKE_SOURCE_DIR}/CrossCompiler)
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
INCLUDE_DIRECTORIES(src/shared)

SET(recog recognize)
SET(share ctrlboard)

# Build executable file
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY build)
ADD_EXECUTABLE(${recog}
    src/recognize/recognize.c
    src/recognize/recognize-lib.c
    src/recognize/display-kms.c
    src/recognize/recognize-update.c
    src/recognize/v4l2.c
    src/recognize/vpe-common.c
    src/shared/ctrlboard-lib.c
    src/shared/util.c)
TARGET_LINK_LIBRARIES(${recog} pthread m jpeg rt drm mtdev drm_omap opencv_core opencv_highgui opencv_imgproc opencv_objdetect)
SET_PROPERTY(TARGET ${recog} PROPERTY C_STANDARD 99)

ADD_EXECUTABLE(${share}
    src/shared/ctrlboard.c
    src/shared/ctrlboard-lib.c
    src/shared/util.c
)
TARGET_LINK_LIBRARIES(${share} m)
SET_PROPERTY(TARGET ${share} PROPERTY C_STANDARD 99)
