cmake_minimum_required(VERSION 3.5)

# Set build type
SET(CMAKE_BUILD_TYPE Release)

# Set optimzation level
SET(CMAKE_C_FLAGS_RELEASE "-O3")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Set compilor path and root filesystem path
SET(CP_PATH ${CMAKE_SOURCE_DIR}/CrossCompiler)
SET(ROOTFS ${CP_PATH}/sysroots/cortexa15t2hf-vfp-neon-linux-gnueabi)

# Set cross compilor path
SET(CROSS_COMPILE ${CP_PATH}/gcc-linaro-arm-linux-gnueabihf/bin/arm-linux-gnueabihf-)

# Set C cross compilor and C++ cross compilor path
SET(CMAKE_C_COMPILER ${CROSS_COMPILE}gcc)
SET(CMAKE_CXX_COMPILER ${CROSS_COMPILE}g++)

# Build executable file
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY build)

# Set C++ version 11
SET(CMAKE_CXX_STANDARD 11)

# Set C version 99
SET(CMAKE_C_STANDARD 99)

# Include libraries
INCLUDE_DIRECTORIES(protocol)
INCLUDE_DIRECTORIES(${ROOTFS}/include)
INCLUDE_DIRECTORIES(${ROOTFS}/usr/include)
INCLUDE_DIRECTORIES(${ROOTFS}/usr/include/omap)
INCLUDE_DIRECTORIES(${ROOTFS}/usr/include/libdrm)
INCLUDE_DIRECTORIES(${ROOTFS}/usr/include/gbm)

# Link dynamic library
LINK_DIRECTORIES(${ROOTFS}/usr/lib)
LINK_DIRECTORIES(${ROOTFS}/usr/src)
LINK_DIRECTORIES(${ROOTFS}/lib)

# Define variable
SET(recog recognize)
SET(clean clean-car)
SET(prcss process)
SET(share shared)

# Define source directories
FILE(GLOB src_recog src/${recog}/*.c* src/${share}/*.c*)
FILE(GLOB src_clean src/${clean}/*.c* src/${share}/*.c*)
FILE(GLOB src_prcss src/${prcss}/*.c* src/${share}/*.c*)

# Include header files
INCLUDE_DIRECTORIES(src/${recog})
INCLUDE_DIRECTORIES(src/${clean})
INCLUDE_DIRECTORIES(src/${prcss})
INCLUDE_DIRECTORIES(src/${share})

# Build recognize
ADD_EXECUTABLE(${recog}
    ${src_recog}
)
TARGET_LINK_LIBRARIES(${recog} pthread m jpeg rt drm mtdev drm_omap opencv_core opencv_highgui opencv_imgproc opencv_objdetect)

# Build clean-car
ADD_EXECUTABLE(${clean}
    ${src_clean}
)
# TARGET_LINK_LIBRARIES(${clean})

# Build process
ADD_EXECUTABLE(${prcss}
    ${src_prcss}
)
# TARGET_LINK_LIBRARIES(${prcss})
