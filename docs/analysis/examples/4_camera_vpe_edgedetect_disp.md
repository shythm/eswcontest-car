# 4_camera_vpe_edgedetect_disp

- 카메라로 영상을 읽어 간선을 추출하는 프로그램

## 용어 정리

### YUV 포맷

- https://en.wikipedia.org/wiki/YUV
- https://blog.dasomoli.org/265/
- https://seoduckchan.tistory.com/entry/yuv-color

 YUV 포맷은 컬러 이미지 파이프라인의 일부로 사용되는 색 인코딩 시스템으로, 빛의 삼원색을 표현하는 RGB와 달리 빛의 밝기를 나타내는 휘도(Y)와 색상 신호 2개(U, V)로 표현하는 방식이다. YUV를 사용하는 주된 이유는 RGB를 사용하는 방식에 비해 압축률을 크게 높일 수 있기 때문이다.

 YUV 포맷은 크게 Packed 포맷과 Planar 포맷으로 나뉠 수 있다.



1. Packed 포맷
   - Y, U (Cb), V (Cr) 성분이 함께 섞여 macropixel을 이루는 방식이다.
   - 주로 사용되는 종류에는 YUYV, UYVY 등이 있는데, Y성분 2개와 그 두 개 Y성분에 대한 U, V 성분을 합쳐 두개의 픽셀을 나타낸다.
   - YUYV 방식은 V422, YUNV, YUY2 로도 불리며, 32비트 안에 Y0, U0, Y1, V0 순서로 한 성분이 각각 8비트씩 저장되어 2개의 픽셀을 나타내게 된다.
   - UYVY 방식은 V422, YUNV, YUY2 로도 불리며, 32비트 안에 Y0, U0, Y1, V0 순서로 한 성분이 각각 8비트씩 저장되어 2개의 픽셀을 나타내게 된다.
   - YUYV 방식과 UYVY 방식 모두 8비트 기준으로 2개의 픽셀을 표현하려면 32비트가 필요하며, 이미지의 해상도가 W * H라면 Y는 W * H, U와 V는 각각 (W * H) / 2 만큼 필요하다.
1. Planar 포맷 
   - Y, U (Cb), V (Cr) 성분이 서로 다른 영역에 분리되어 저장되는 방식이다.
   - 주로 사용되는 종류에는 NV12, NV21 등이 있는데,  4 픽셀을 표현하기 위해 4개의 Y와 1개씩의 U, V가 필요하다.
   - 8비트를 기준으로 4픽셀을 표현하기 위해서는 48비트가 필요하며, W * H 해상도인 이미지를 표현하려면 Y는 W * H, U와 V는 각각 (W * H) / 4 만큼이 필요하다.



 main.c에서 영상을 캡쳐하는 데 UYVY 포맷을, VPE로 영상을 내보내는 데 NV12 포맷을 사용하고 있다.



### Dump

- https://m.blog.naver.com/PostView.nhn?blogId=on21life&logNo=221510758310&proxyReferer=https:%2F%2Fwww.google.com%2F

 Dump는 기억 장치의 내용을 기록한 데이터로, 프로그램 디버그 또는 시스템 테스트의 목적을 위해 기록되는 파일이다. 이 예제에서는 간선을 추출한 결과 이미지를 저장하는 것을 Dump로 명명한다.



### V4L2 (Video4Linux 2)

- https://en.wikipedia.org/wiki/Video4Linux

 V4L2 (Video4Linux 2)는 리눅스 운영체제에서 카메라를 통한 실시간 영상 촬영을 위한 드라이버와 API의 집합이다. V4L2는 리눅스 커널에서 기본적으로 지원하므로, 리눅스 시스템에서 비디오 촬영을 간단하게 구현할 수 있다.



### VPE (Video Port Extensions)

- http://software-dl.ti.com/processor-sdk-linux/esd/docs/latest/linux/Foundational_Components/Kernel/Kernel_Drivers/VPE.html
- https://processors.wiki.ti.com/index.php/PDK/PDK_VPS_VPE_Driver_User_Guide
- https://www.ti.com/lit/ds/symlink/tda2p-acd.pdf?ts=1596210116924&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FTDA2P-ACD

 VPE (Video Port Extensions)는 video decoder를 

