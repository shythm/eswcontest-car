# Embedded System Contest Study

## C / C++ / Linux Functions

- `ioctl(fd,cmd,args)` : IO control. Sends command to Linux device driver. (with arguments)
- `memset` : Fill memory with specified value.
- `set_XYZ(vpe,...)` functions in `vpe-common` : Initialize that function of `vpe` object and send command to hardware driver with `ioctl`.
- `memcpy(dst,src,size)`
- `void *mmap(void *addr, size_t length, int pr ot, int flags, int fd, off_t offset);`
  - The prot argument describes the desired memory protection of the
    mapping (and must not conflict with the open mode of the file).  It
    is either `PROT_NONE` or the bitwise `OR` of one or more of the following
    flags:
    - `PROT_EXEC`:  Pages may be executed.
    - `PROT_READ`:  Pages may be read.
    - `PROT_WRITE`: Pages may be written.
    - `PROT_NONE`:  Pages may not be accessed.
- `describeFormat`: Initialize `image_params` struct.

  ```c
  // Below is an example of initialization.
  image->fourcc = V4L2_PIX_FMT_NV21;
  image->size = image->height * image->width * 1.5;
  image->coplanar = 1;
  image->colorspace = V4L2_COLORSPACE_SMPTE170M;
  ```
- `int pthread_detach(pthread_t thread)`: Detach a thread.
  - The `pthread_detach()` function marks the thread identified by thread
       as detached.  When a detached thread terminates, its resources are
       automatically released back to the system without the need for
       another thread to join with the terminated thread.   Attempting to detach an already detached thread results in
       unspecified behavior.
- `open_xyz`: Open file, and set file descriptor, which is actually a device driver. Then, initialize device driver with ioctl method.

## Terminology

- `fourcc`: Four Character Code. An identifier for a video codec, compression format, color or pixel format used in media files.
- `VIP`: Video Input Port = Camera
- `VPE`: Video Processing Engine = GPU. Support Scalar, Color Space Conversion and Deinterlace
  - `Scaler`: Resizing
  - `Deinterlace`: Conversion from interlaced scanning to progressive scanning.
- `v4l2`: Video for Linux 2. 
- `KMS`: Kernel Mode Setting.
- `DRM`: Direct Rendering Manager
- `OMAP`: Open Multimedia Applications Platform. A series of image/video processors.
- `GEM`: Generalized Embedded Megamodule
- `bo`: Buffer Object
- `omap_bo`: A GEM buffer object allocated from the DRM device
  ```c
  struct omap_bo {
    struct omap_device *dev;
    void      *map;   /* userspace mmap'ing (if there is one) */
    uint32_t  size;
    uint32_t  handle;
    uint32_t  name;   /* flink global handle (DRI2 name) */
    uint64_t  offset; /* offset to mmap() */
    int       fd;     /* dmabuf handle */
    atomic_t	refcnt;
  };
  ```
- `playback-rate` : Video play speed. 0.5 for slowmotion, 2 for hyperlapse.
- `CRTC`: Scanout engine of disply controller, pointing to a scanout buffer (framebuffer).
  
  - The purpose of a CRTC is to read the pixel data currently in the scanout buffer and generate from it the video mode timing signal with the help of a PLL circuit. The number of CRTCs available determines how many independent output devices can the hardware handle at the same time, so in order to use multi-head configurations at least one CRTC per display device is required.

## Linux System
- To install `i386` packages, run:
  - `dpkg --add-architecture i386`
  - `apt-get update`