/**
 * RAIN SENSER
 */
#include <Arduino.h>
#include "Senser.h"

#define PULSE_PIN 19
#define DIGITAL_READ_PIN 22
#define ANALOG_READ_PIN 33
#define LED_RAIN  21  // 雨LED
#define LED_CHECK 25  // 計測LED

const double P = 4095.0;
const int    EP = P * 0.6322;   // 63.22%の電圧のAD読み取り値

// ----------------------
// ----- コンストラクタ ----
// ----------------------
SENSER::SENSER( ) {
  BorderRain = 620 ; // 雨降りと判定する境界値
  BorderSunny = 590; // 雨上がりと判定する境界値

  pinMode(LED_CHECK, OUTPUT);
  digitalWrite(LED_CHECK, LOW);

  pinMode(LED_RAIN, OUTPUT);
  digitalWrite(LED_RAIN, LOW);

  pinMode(PULSE_PIN, OUTPUT);
  digitalWrite(PULSE_PIN, LOW);
  pinMode(ANALOG_READ_PIN, INPUT);

  stat = 0 ;          // 0:晴れ 1:雨が降ってきた 2:雨 3:雨が上がった
  flagRain = false ;  // 雨フラグ
}


// -------------
// --- 初期化 ---
// -------------
void SENSER::init( ) {
  // 移動平均の初期設定(誤動作防止用)
  for (int i=0;i<AVRNUM;i++) {
    getCapacitance() ;
  }
}

// ----------------------
// ----- 静電容量計測 -----
// ----------------------
int SENSER::getCapacitance() {
  // ---  放電 --
  pinMode(ANALOG_READ_PIN, INPUT);

  pinMode(PULSE_PIN, OUTPUT);
  digitalWrite(PULSE_PIN, LOW);
  pinMode(DIGITAL_READ_PIN, OUTPUT);
  digitalWrite(DIGITAL_READ_PIN, LOW);
  delay(1000); 
  pinMode(DIGITAL_READ_PIN, INPUT);
  delay(20);

  // --- 充電時間T計測
  digitalWrite(LED_CHECK, HIGH);
  digitalWrite(PULSE_PIN, HIGH);
  unsigned long startTime = micros();
  while (analogRead(ANALOG_READ_PIN) < EP) ;
  digitalWrite(LED_CHECK, LOW);
  long T = micros() - startTime;
  avr[avrPos] = T ;
  avrPos = (avrPos + 1) % AVRNUM ;

  // --- 移動平均算出
  avrT = 0 ;
  for (int i=0;i<AVRNUM;i++) {
    avrT += avr[i] ;
  }
  avrT /= AVRNUM ;

  if (avrT < BorderSunny) {
    // 雨LED消灯
    digitalWrite(LED_RAIN, LOW);
    stat = 0 ; // 0:晴れ 1:雨が降ってきた 2:雨 3:雨が上がった
  }
  if (avrT >= BorderSunny) {
    // 雨LED点灯
    digitalWrite(LED_RAIN, HIGH);
    stat = 2 ; // 0:晴れ 1:雨が降ってきた 2:雨 3:雨が上がった
  }
  if (avrT > BorderRain && flagRain == false) {
    // 雨が降ってきた
    flagRain = true ;
    stat = 1 ; // 0:晴れ 1:雨が降ってきた 2:雨 3:雨が上がった
  }
  if (avrT < BorderSunny && flagRain == true) {
    // 雨が上がった
    flagRain = false ;
    stat = 3 ; // 0:晴れ 1:雨が降ってきた 2:雨 3:雨が上がった
  }

  return stat ;
}
