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

주요한 명령들만 간략하게 쓴다.

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
    memset(tdata.dump_img_data, 0, sizeof(tdata.dump_img_data)); //덤프할 이미지는 dump_img_data에 저장될 것이다.
    disp = disp_open(disp_argc, disp_argv);
    
    set_z_order(disp, disp->overlay_p.id);// 2차원 도형들은 (x,y)좌표로 나타냈다. 이들의 순서를 z축의 값을 통해 나타낸다.
    set_global_alpha(disp, disp->overlay_p.id); // 전역 투명도(?)
    set_pre_multiplied_alpha(disp, disp->overlay_p.id);
    alloc_overlay_plane(disp, OVERLAY_DISP_FORCC, 0, 0, OVERLAY_DISP_W, OVERLAY_DISP_H); //오버레이 평면버퍼 할당
	fourcc = getfourccformat(CAPTURE_IMG_FORMAT);
	v4l2 = v4l2_open(fourcc, CAPTURE_IMG_W, CAPTURE_IMG_H);
    
    //초기화
    tdata.disp = disp;
    tdata.v4l2 = v4l2;
    tdata.bfull_screen = true;
    tdata.bstream_start = false;
    tdata.dump_screen = false;
    
     if(-1 == (tdata.msgq_id = msgget((key_t)DUMP_MSGQ_KEY, IPC_CREAT | 0666)))
     pexam_data = &tdata;
    
    //스레드 3개 만든다.
    // thread[0]: capture_thread
    // thread[1]: capture_dump_thread
    // thread[2]: input_thread
    ret = pthread_create(&tdata.threads[0], NULL, capture_thread, &tdata);
    pthread_detach(tdata.threads[0]);

    ret = pthread_create(&tdata.threads[1], NULL, capture_dump_thread, &tdata);
    pthread_detach(tdata.threads[1]);

    ret = pthread_create(&tdata.threads[2], NULL, input_thread, &tdata);
    pthread_detach(tdata.threads[2]);
    
    // signal_handler에서 Ctrl+C 체크
    if(signal(SIGINT, signal_handler) == SIG_ERR) {
        MSG("could not register signal handler");
        closelog();
        exit(EXIT_FAILURE);
    }
    
        pause();
}
```

## **capture_thread**

```c
void * capture_thread(void *arg)
{
    //초기화
    struct thr_data *data = (struct thr_data *)arg;
    struct display *disp = data->disp;
    struct v4l2 *v4l2 = data->v4l2;
    struct buffer **buffers, *capt;
    int ret, i;
	// 버퍼할당
    buffers = disp_get_vid_buffers(disp, NBUF, getfourccformat(CAPTURE_IMG_FORMAT), CAPTURE_IMG_W, CAPTURE_IMG_H);

    ret = v4l2_reqbufs(v4l2, buffers, NBUF);

    for (i = 0; i < NBUF; i++) {	//6개 버퍼들 큐에 넣기
        v4l2_qbuf(v4l2, buffers[i]); // 드라이버
    }
    ret = v4l2_streamon(v4l2); // 스트림 시작
    data->bstream_start = true;//스트림 시작 플래그

    while(1) {	//무한 루프 (signal_handler에 의해 함수 종료될 수 있음)
        capt = v4l2_dqbuf(v4l2); //스레드
        if(data->bfull_screen) // ?? 스케일?
            capt->noScale = false;
        else
            capt->noScale = true;
        ret = disp_post_vid_buffer(disp, capt, 0, 0, CAPTURE_IMG_W, CAPTURE_IMG_H);

        if(data->dump_state == DUMP_READY) {// 덤프 레디이면
            DumpMsg dumpmsg;
            unsigned char* pbuf[4];

            if(get_framebuf(capt, pbuf) == 0)
                memcpy(data->dump_img_data, pbuf[0], CAPTURE_IMG_SIZE);
  
            dumpmsg.type = DUMP_MSGQ_MSG_TYPE;// 메세지큐를 이용하기 위한 준비
            dumpmsg.state_msg = DUMP_WRITE_TO_FILE;//메세지
            data->dump_state = DUMP_WRITE_TO_FILE;// 덤프관련 스테이트
            msgsnd(data->msgq_id, &dumpmsg, sizeof(DumpMsg)-sizeof(long), 0);
            // 메세지는 capture_dump_thread가 받음
        }
        v4l2_qbuf(v4l2, capt); //영상 받아옴
    }
    v4l2_streamoff(v4l2);
    return NULL;
}
```



## **capture_dump_thread**

```c
void * capture_dump_thread(void *arg)
{
    // 초기화
    struct thr_data *data = (struct thr_data *)arg;
    FILE *fp;
    char filename[50];
    struct timeval timestamp;
    struct tm *today;
    DumpMsg dumpmsg;

    while(1) {
        if(msgrcv(data->msgq_id, &dumpmsg, sizeof(DumpMsg)-sizeof(long), DUMP_MSGQ_MSG_TYPE, 0) >= 0) {
            switch(dumpmsg.state_msg) {
                case DUMP_CMD : //input_thread에서 덤프 명령 받으면
                    gettimeofday(&timestamp, NULL);
                    today = localtime(&timestamp.tv_sec);//현재 시간으로
                    memset(filename, 0, sizeof(filename));//파일이름 만들고
                    data->dump_state = DUMP_READY;//덤프레디capture_thread로
                    break;

                case DUMP_WRITE_TO_FILE : //capture_thread에서 작업 후 넘어옴
                    fp = fopen(filename, "w+");
                    if(data->dump_screen) {
                        makescreendata(data);
                        fwrite(data->dump_screen_data, SCREEN_DUMP_SIZE, 1, fp);
                    }

                    fclose(fp);
                    data->dump_state = DUMP_DONE; //덤프 완료
                    break;

                default :
                    MSG("dump msg wrong (%d)", dumpmsg.state_msg);
                    break;
            }
        }
    }
    return NULL;
}
```



## **input_thread**

```c
void * input_thread(void *arg)
{
    struct thr_data *data = (struct thr_data *)arg;
    struct display *disp = data->disp;

    char cmd_input[256];
    char cmd_ready = true;

    while(!data->bstream_start) { // capture_thread에서 시작할 때까지 대기 
        usleep(100*1000);
    }

    while(1)
    {
        if(cmd_ready == true) { //입력받을 준비가 되었다면
            /*standby to input command */
            cmd_ready = StandbyInput(cmd_input);     //define in cmd.cpp
        } else { // 이미 입력이 있다면
            if(0 == strncmp(cmd_input,"dump",4)) { // 입력이 dump면
                DumpMsg dumpmsg;
                dumpmsg.type = DUMP_MSGQ_MSG_TYPE;
                dumpmsg.state_msg = DUMP_CMD; //덤프 명령: capture_dump_thread가 받음
                data->dump_state = DUMP_CMD;

                msgsnd(data->msgq_id, &dumpmsg, sizeof(DumpMsg)-sizeof(long), 0);
                while(data->dump_state != DUMP_DONE) //덤프 끝날 때까지 대기
                    usleep(5*1000);

                data->dump_state = DUMP_NONE; //덤프할 거 없음
            } else if(0 == strncmp(cmd_input,"sdump",5)) {// 입력이 sdump
                DumpMsg dumpmsg;
                data->dump_screen = true;
                dumpmsg.type = DUMP_MSGQ_MSG_TYPE;
                dumpmsg.state_msg = DUMP_CMD;//덤프 명령: capture_dump_thread가 받음
                data->dump_state = DUMP_CMD;

                msgsnd(data->msgq_id, &dumpmsg, sizeof(DumpMsg)-sizeof(long), 0);
                while(data->dump_state != DUMP_DONE) //덤프 끝날 때까지 대기
                    usleep(5*1000);
                
                data->dump_state = DUMP_NONE;
                data->dump_screen = false;
                printf("sdump done!\n");
                
            } else if(0 == strncmp(cmd_input,"draw",4)) { // draw명령
                FrameBuffer tmpFrame;
                unsigned char* pbuf[4];
                char drawcmd[12];
                char drawtxt[24];
                int type = 0;
                uint32_t x1, y1, x2, y2, w, h, color;
                memset(&tmpFrame, 0, sizeof(FrameBuffer));

                if(0 == strncmp(cmd_input,"drawP",5)) {
                    sscanf(cmd_input, "%5s:%d,%d:0x%08x", drawcmd, &x1, &y1,&color);
                    type = 0; //draw pointer
                } else if(0 == strncmp(cmd_input,"drawL",5)) {
                    sscanf(cmd_input, "%5s:%d,%d:%d,%d:0x%08x", drawcmd, &x1, &y1,&x2,&y2,&color);
                    type = 1; //draw line
                } else if(0 == strncmp(cmd_input,"drawR",5)) {
                    sscanf(cmd_input, "%5s:%d,%d:%d,%d:0x%08x", drawcmd, &x1, &y1,&w,&h,&color);
                    type = 2; //draw rect
                } else if(0 == strncmp(cmd_input,"drawT",5)) {
                    sscanf(cmd_input, "%5s:%d,%d:0x%08x:%[^\n]", drawcmd, &x1, &y1,&color, drawtxt);
                    type = 3; //draw string
                } else if(0 == strncmp(cmd_input,"drawC",5)) {
                    type = 4; // clear
                } else {
                    cmd_ready = true;
                    continue;
                }

                if(get_framebuf(disp->overlay_p_bo, pbuf) == 0) {
                    tmpFrame.buf = pbuf[0]; // 영상 버퍼에 draw한다
                    tmpFrame.format = draw_get_pixel_foramt(disp->overlay_p_bo->fourcc); //FORMAT_RGB888; //alloc_overlay_plane() -- FOURCC('R','G','2','4');
                    tmpFrame.stride = disp->overlay_p_bo->pitches[0];//tmpFrame.width*3;

                    switch(type) {
                        case 0: // draw pointer
                            drawPixel(&tmpFrame, x1, y1, color);
                            break;
                        case 1: // draw line
                            drawLine(&tmpFrame, x1, y1, x2, y2, color);
                            break;
                        case 2: // draw rect
                            drawRect(&tmpFrame, x1, y1, w, h, color);
                            break;
                        case 3: // draw string
                            drawString(&tmpFrame, drawtxt, x1, y1, 0, color);
                            break;
                        case 4 : // draw clear
                            memset(tmpFrame.buf, 0, disp-> overlay_p.yres *tmpFrame.stride);
                            break;
                        default :
                            break;
                    }
                }
                update_overlay_disp(disp); //그려주고 update 꼭 해주어야 함.

            }
            cmd_ready = true;
        }
    }
    return NULL;
}
```



