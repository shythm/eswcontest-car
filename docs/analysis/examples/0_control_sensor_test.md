# 예제코드 0번 분석

## LIGHT_BEEP

차의 전조등, 후미등, 부저, 깜박이 검사하는 코드

* CarLight_Write(아래 인자들) :  단순히 차 전조등, 후미등 껐다 켰다 하는 기능
  ALL_ON, ALL_OFF, FRONT_ON, REAR_ON 이렇게 4개 인자 전달
* Alarm_Write(아래 인자들) : 단순히 알람을 켰다가 껐다가 하는 기능
  ON, OFF 이렇게 2개 인자 전달
* Winker_Write(아래 인자들) : 깜박이 껐다 켰다 하는 기능
  왼쪽 오른쪽 두 개 깜박이가 있다
  ALL_ON, ALL_OFF, LEFT_ON, RIGHT_ON 이렇게 4개 인자 전달

## POSITION_CONTROL

포지션(엔코더) 로 차량 제어하는 예제 코드

주의사항으로는 Speed Conroller도 켜져있어야 한다.  또한 DesireSpeed_Write() 함수를 이용해서 목표 스피드도 지정해주어야 한다.

* SpeedControlOnOff_Write() : 속도로 제어하는 기능을 켤 것인지 끌 것인지
  CONTROL, UNCONTROL 2개의 인자 전달
* DesireSpeed_Wirte() : 원하는 속도 입력
* SpeedControlOnOff_Read() : 현재 속도 제어 기능이 켜져있는지 꺼져있는지를 확인. 변수에 반환인자를 저장할 수 있다.
* PositionPropportionPoint_Read() : 현재 gain 값이 얼마인지를 읽어옴
* PositionPropportionPoint_Write() : gain 인자가 전달되어 게인을 설정함
* EncoderCounter_Write() : 전달된 인자로 엔코더 값이 초기화됨
* EncoderCounter_Read() : 현재의 엔코더 값이 반환됨
* DesireEncoderCount_Write() : 전달 된 인자가 목표 엔코더 값이 되어 현재 엔코더 값부터 희망한 엔코더 값이 될 때까지 차가 앞으로 감.
* DesireEncoderCount_Read() : 최근에 입력된 희망 엔코더 값이 반환됨

## SPEED_CONTROL

속도로 차량을 제어하는 예제 코드

주의사항으로는 PositionControlOnOff_Write() 함수를 이용하여 Position Controller를 꺼주어야만 한다.

* PositionControlOnOff_Write() : 앞서 말했듯이 엔코더를 이용해 차량 제어를 할지 말지 정하는 함수. CONTROL, UNCONTROL 두 개의 인자
* SpeedControlOnOff_Read(void) : 현재 속도제어가 켜져있는지 꺼져있는지를 반환하는 함수
* SpeedControlOnOff_Write() : 속도제어를 할지 말지 결정. CONTROL, UNCONTROL 두 개의 인자 갖는다.
* SpeedPIDProportional_Read(void) : 속도이 비례하는 gain 값을 반환인자로 갖는 함수 . 기본 값은 10이다.
* SpeedPIDProportional_Wirte() : 속도에 비례하는 gain 값을 지정하는 함수. gain을 인자로 갖는다. 기본 값은 10이다.
* SpeedPIDIntegral_Read() : 속도를 적분한 값(거리)에 비례하는 gain 값을 읽어오는 함수. 
* SpeedPIDIntegral_Wirte() : 속도를 적분한 값(거리)에 비례하는 gain 값을 지정하는 함수. gain을 인자로 갖는다.기본 값은 10이다.
* SpeedPIDDifferential_Read() : 속도를 미분한 값(가속도)에 비례하는 gain 값을 읽어오는 함수
* SpeedPIDDifferential_Write() : 속도를 미분한 (가속도)에 비례하는 gain 값을 지정하는 함수. gain을 인자로 갖는다기본 값은 10이다.
* DesireSpeed_Read() : 저장했던 희망 속도를 반환한다.
* DesireSpeed_Write() : 희망 속도를 지정한다. Speed를 인자로 갖는다

## SERVO_CONTROL

* SteeringServoControl_Read() : 차체 방향을 조절하는 서보모터의 현재 값을 읽어온다. 기본 값은 1500이다
* SteeringServoControl_Write() : 차체 방향을 조절하는 서보모터의 angle 값을 지정한다. 
* CameraXServoControl_Write() : 카메라의 x축 방향을 조절하는 서보모터의 방향을 조절한다. 기본값은 1500이다. angle 변수가 인자로 들어간다.
* CameraXServoControl_Read() : 카메라 x축 방향 값을 읽어온다. 
* CameraXServoControl_Write() : 카메라 x축 방향 값을 지정한다. 기본 값은 1500이다.
* CameraYServoControl_Read() : 카메라 y축 방향 값을 읽어온다.
* CameraYServoControl_Write() : 카메라 y축 방향 값을 지정한다. 기본 값은 1500이다.

## LINE_TRACE

* LineSensor_Read() : 

## DISTANCE_SENSOR

* 

​                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      