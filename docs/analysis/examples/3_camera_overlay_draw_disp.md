# 사전지식



```
void * memset ( void * ptr, int value, size_t num );
```

ptr이 가리키는 곳부터 num바이트를 value로 채운다. value는 unsigned char 1바이트.

for문보다 빠를 수도 있다.



**DumpState dump_state;**

스테이트의 종류: DUMP_NONE,    DUMP_CMD,    DUMP_READY,    DUMP_WRITE_TO_FILE,    DUMP_DONE



**FOURCC('a', 'b', 'c','d') : four character code**

=> char를 4개 합쳐 4바이트로 만듦 =>  abcd



**z-order**

=> 2차원 도형들이 여러 개 있는데, 그 것들의 상하 순서를 정한다.



**set global alpha**

2차원 그림에서 색상은 픽셀에 저장되고, 추가정보는 알파채널에 0혹은 1이 저장된다. 

0: 투명함, 1: 불투명함 (아래 그림이 투과해서 보이지 않음) => 마스킹 같은건가



**DRM**: Direct Rendering Manager (디스플레이 컨트롤 드라이버,GPU접근을 가능케하는 리눅스 커널의 서브 시스템)



```c
drawing.h
typedef struct _FrameBuffer {
	PixelFormat format; // 우리가 쓰는건 FORMAT_RGB888
    uint32_t stride;	// tmpFrame.width*3; 1픽셀당 R,G,B 3개의 바이트 필요
    unsigned char* buf;
}FrameBuffer;
```

```c
struct display  {
    ...
	struct plane overlay_p;
};
struct plane {
    uint32_t id;	
    uint32_t x;
    uint32_t y;
    uint32_t xres;
    uint32_t yres;
    uint32_t fb_id;
    uint32_t z_val;
    uint32_t glo_alp;
    uint32_t pre_mul_alp;
};

struct thr_data{
	...
    pthread_t threads[3]; // 0: capture_tread, 1: capture_dump_thread, 2: input_thread

}
```



```cpp
void* memcpy(void* destination, const void* source, size_t num);
```

source가 가리키는 곳부터 num바이트 만큼을 destination이 가리키는 곳에 복사.



```c
#include <sys/ioctl.h>
int ioctl(int __fd, unsigned long int __requet, ...);
```

하드웨어의 동작 상태에 따라 read(), write()함수만으로는 처리하지 못하는 데이터 발생. => 리눅스 커널에서 ioctl을 제공한다. https://guswnsla1223.tistory.com/126



```c
int drmModeObjectSetProperty (int  		fd,
                              uint32_t  object_id,
                              uint32_t  object_type,
                              uint32_t  property_id,
                              uint64_t  value ) 
    
set the current value of an object's property.

Parameters
[in]fd			The file descriptor of an open DRM device.
    object_id	The object ID of the DRM object whose properties are to be set.
    object_type	A symbol representing an object type. The following object types are 
    			supported:  DRM_MODE_OBJECT_CRTC
    property_id	The ID of the property to be set.
    value		A new value for the property.

Returns
    0 if successful, or -1 otherwise. 
```

fd: 파일 디스크립터 



```c
drawing.c
static bool get_char_pixel(char c, uint32_t x, uint32_t y) {
    // 8x8 공간에서 글자c를 그리기 위한 비트 정보 알려줌
    uint8_t bits = fontdata_8x8[8 * c + y];
    bool bit = (bits >> (7 - x)) & 1;
    return bit;
}
void drawChar (...){
    ...
	for (y = 0; y < 8; y++) {
        for (x = 0; x < 8; x++) {
            bool b = get_char_pixel(c, x, y); //(x,y): 활자의 8x8공간에서의 좌표
            drawPixel(pFrame, startx + x, starty + y, b ? color : 0x00000000);
            // (startx+x, starty+y): 평면 상의 좌표
        }	}	}
void drawString(...){
    ...
    for(i = 0; i < len; i++)	// 한 글자 폭이 8비트이므로 8*i만큼 오른쪽으로 이동해서 그린다.
        drawChar(pFrame, str[i], (startx + 8 * i), starty, size, color);
}
```





# 함수

## 	main

```c
int main(int argc, char **argv){
    //뭔진 모르겠지만 초기화
	struct display *disp;
    struct v4l2 *v4l2;
    struct thr_data tdata;
    int disp_argc = 3;
    char* disp_argv[] = {"dummy", "-s", "4:480x272", "\0"};
    int ret = 0;
    uint32_t fourcc;
    printf("-- 3_camera_overlay_draw_disp example Start --\n");

    tdata.dump_state = DUMP_NONE;
    memset(tdata.dump_img_data, 0, sizeof(tdata.dump_img_data)); //덤프할 이미지는 dump_img_data에 저장될 �