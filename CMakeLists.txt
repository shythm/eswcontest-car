cmake_minimum_required(VERSION 3.5)

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

# Include custom libraries
INCLUDE_DIRECTORIES(src/shared)

# Link dynamic library
LINK_DIRECTORIES(${ROOTFS}/usr/lib)
LINK_DIRECTORIES(${ROOTFS}/usr/src)
LINK_DIRECTORIES(${ROOTFS}/lib)

# Define variable
SET(recog recognize)
SET(clean clean-car)
SET(cntrl control)
SET(board ctrlboard)
SET(share shared)

# Define source directories
AUX_SOURCE_DIRECTORY(src/${recog} src_recog)
AUX_SOURCE_DIRECTORY(src/${clean} src_clean)
AUX_SOURCE_DIRECTORY(src/${cntrl} src_cntrl)
AUX_SOURCE_DIRECTORY(src/${board} src_board)
AUX_SOURCE_DIRECTORY(src/${share} src_share)


# Build recognize
ADD_EXECUTABLE(${recog}
    ${dir_recog}
    ${dir_share}
)
TARGET_LINK_LIBRARIES(${recog} pthread m jpeg rt drm mtdev drm_omap opencv_core opencv_highgui opencv_imgproc opencv_objdetect)

# Build control board
ADD_EXECUTABLE(${board}
    ${dir_board}
    ${dir_share}
)
TARGET_LINK_LIBRARIES(${board} m)

# Build clean-car
ADD_EXECUTABLE(${clean}
    ${dir_clean}
    ${dir_share}
)
# TARGET_LINK_LIBRARIES(${clean})

# Build control
ADD_EXECUTABLE(${cntrl}
    ${dir_cntrl}
    ${dir_share}
)
# TARGET_LINK_LIBRARIES(${cntrl})
