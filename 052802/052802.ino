#include <SoftwareSerial.h>
#include <Servo.h>

// 전역 변수
bool bStart = false;
int nVol, cdsVol, btnVol;
bool bRobotStanding = true;

// 핀 정의
#define LEDRED    9
#define LEDYEL    10
#define LEDGRN    11
#define VOLSEN    A0
#define CDSSEN    A1
#define BTN1      8
#define TRIG_PIN  4  
#define ECHO_PIN  5   

SoftwareSerial mySerial(2, 3); // RX, TX
Servo myservo;

// 거리 측정 함수
long measureDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms 타임아웃
  if (duration == 0) return -1; // 실패 시 -1 반환
  long distance = duration * 0.034 / 2;
  return distance;
}

void setup() {
  pinMode(LEDRED, OUTPUT);
  pinMode(LEDYEL, OUTPUT);
  pinMode(LEDGRN, OUTPUT);
  pinMode(BTN1, INPUT);
  pinMode(VOLSEN, INPUT);
  pinMode(CDSSEN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  Serial.begin(115200);
  mySerial.begin(115200);
  myservo.attach(9);
  Serial.println("------ 프로그램 시작 ------");
}

void robotCon(int nMotion) {
  unsigned char exeCmd[15] = {0xff, 0xff, 0x4c, 0x53,
                              0x00, 0x00, 0x00, 0x00,
                              0x30, 0x0c,
                              0x03,
                              0x01, 0x00, 100,
                              0x00};
  exeCmd[11] = nMotion;
  exeCmd[14] = 0x00;
  for (int i = 6; i < 14; i++) exeCmd[14] += exeCmd[i];
  mySerial.write(exeCmd, 15);
  delay(50);
}

void readSensor() {
  nVol = analogRead(VOLSEN);
  cdsVol = analogRead(CDSSEN);
  btnVol = digitalRead(BTN1);

  Serial.print("btnVol = ");
  Serial.print(btnVol);
  Serial.print("\t cdsVol = ");
  Serial.print(cdsVol);
  Serial.print("\t nVol = ");
  Serial.println(nVol);
}

bool toggleCheck() {
  return btnVol;
}

void ledRobotStat() {
  digitalWrite(LEDGRN, bRobotStanding);
}

void loop() {
  readSensor();
  long distance = measureDistanceCM();

  if (distance == -1) {
    Serial.println("초음파 센서 응답 없음!");
  } else {
    Serial.print("Distance = ");
    Serial.print(distance);
    Serial.println(" cm");
  }

  if (toggleCheck()) bStart = !bStart;
  if (bStart) {
    robotCon(19);
    delay(7000);
    bStart = false;
  }

  if (cdsVol < 500 || distance > 150) {
    if (bRobotStanding) {
      robotCon(115); // 앉기
      delay(7000);
      bRobotStanding = false;
      myservo.write(50);
    }
  } else if (distance > 100 && distance <= 150) {
    if (!bRobotStanding) {
      robotCon(116); // 서기
      delay(7000);
      bRobotStanding = true;
      myservo.write(150);
    }
  } else if (distance > 50 && distance <= 100) {
    robotCon(19); // 인사
    delay(7000);
  } else if (distance <= 50) {
    robotCon(22); // 전투 태세
    delay(7000);
  }

  delay(500);
}
