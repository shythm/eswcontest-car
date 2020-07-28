# 예제코드 0번 분석

## LIGHT_BEEP

차의 전조등, 후미등, 부저, 깜박이 검사하는 코드

* CarLight_Write() :  단순히 차 전조등, 후미등 껐다 켰다 하는 기능
  ALL_ON, ALL_OFF, FRONT_ON, REAR_ON 이렇게 4개 인자 전달
* Alarm_Write() : 단순히 알람을 켰다가 껐다가 하는 기능
  ON, OFF 이렇게 2개 인자 전달
* Winker_Write() : 깜박이 껐다 켰다 하는 기능
  왼쪽 오른쪽 두 개 깜박이가 있다
  ALL_ON, ALL_OFF, LEFT_ON, RIGHT_ON 이렇게 4개 인자 전달

## POSITION_CONTROL

* SpeedControlOnOff_Write() : 속도로 제어하는 기능을 켤 것인지 끌 것인지
  CONTROL, UNCONTROL 2개의 인자 전달
* DesireSpeed_Wirte() : 원하는 속도 입력