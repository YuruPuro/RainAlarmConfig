/**
 * 静電容量式レインセンサー for M5Stamp
 * 雨が降ったらＬＩＮＥに通知
 */
#include "EEPROM.h"
#include "Senser.h"
#include "MiniServer.h"
#include "LineNotify.h"

const uint32_t colorOrange= 0x0080FF00 ;
const uint32_t colorGreen = 0x00FF0000 ;
const uint32_t colorRed   = 0x0000FF00 ;
const uint32_t colorBlue  = 0x000000FF ;

int mode = 0 ;  // 動作モード 0:Rain Sencer 1:Config
MiniServer *miniServer = NULL ;
LINENOTYFY *lineNotify = NULL ;
SENSER rainSenser ;

#define RGBPIN 27
#define BTNPIN 39

// --- 設定値 ---
const char HEAD_KEY[] = "RAIN ALARM" ;
struct conf_s {
  char HEAD[12] ;          // データ識別キー:RAIN ALARM
  char WIFI_SSID[32] ;     // Wi-Fiに接続する用
  char WIFI_PWD[32] ;      // Wi-Fiに接続する用
  char LINEHOST[64];       // LINE Notify API URL
  char LINETOKEN[64];      // LINE Notify API Token
  uint32_t SENSER_SUNNY ;
  uint32_t SENSER_RAIN ;
} conf ; 

// ---------------------
// ----- COLOR LED -----
// ---------------------
void SetColorLED(uint32_t bitData) {
  int BTN ;
  for (uint32_t sendBit = 0x00800000;sendBit!=0;sendBit>>=1) {
    if ((bitData & sendBit) == 0 ) {
      digitalWrite(RGBPIN,HIGH);
      for (int L=0;L<2;L++) { BTN = digitalRead(BTNPIN); }
      digitalWrite(RGBPIN,LOW);
      for (int L=0;L<6;L++) { BTN = digitalRead(BTNPIN); }
    } else {
      digitalWrite(RGBPIN,HIGH);
      for (int L=0;L<3;L++) { BTN = digitalRead(BTNPIN); }
      digitalWrite(RGBPIN,LOW);
      for (int L=0;L<3;L++) { BTN = digitalRead(BTNPIN); }
    }
  }
}

// ----------------------
// ----- セットアップ -----
// ----------------------
void setup() {
  // --- PIN MODE INITIAL ---
  pinMode(BTNPIN,INPUT) ;
  pinMode(RGBPIN,OUTPUT) ;
  digitalWrite(RGBPIN,LOW);
  delayMicroseconds(80) ;
  digitalWrite(RGBPIN,HIGH);

  // --- CONFIG ---
  SetColorLED(colorRed) ;
  if (!EEPROM.begin(sizeof(conf))) {
    SetColorLED(colorRed) ;
    while(true);
  }

  uint8_t *confP = (uint8_t *)&conf ;
  for (int i = 0; i < sizeof(conf); i++) {
    confP[i] = byte(EEPROM.read(i)) ;
  }

  for (int i=0;HEAD_KEY[i] != 0 ;i++) {
    if (conf.HEAD[i] != HEAD_KEY[i]) {
      // --- 初期値設定 -----
      strcpy(conf.HEAD,HEAD_KEY) ;
      strcpy(conf.WIFI_SSID,"Hoge") ;
      strcpy(conf.WIFI_PWD,"Piyo") ;
      strcpy(conf.LINEHOST,"Hoge") ;
      strcpy(conf.LINETOKEN,"notify-api.line.me") ;
      conf.SENSER_SUNNY = rainSenser.getBorderSunny( ) ;
      conf.SENSER_RAIN = rainSenser.getBorderRain( ) ;
      break ;
    }
  }
  rainSenser.init( ) ;

  miniServer = new MiniServer( ) ;
  lineNotify = new LINENOTYFY( ) ;

  // --- MODE DISPLAY ---
  mode = 0 ;
  SetColorLED(colorGreen) ;
}

// ----- StartMiniServer-----
void StartMiniServer( ) {
  // Open HTTP Server
  SetColorLED(colorRed) ;
  miniServer->StartServerMode( ) ;
  strcpy(miniServer->ssid,conf.WIFI_SSID) ;     // Wi-Fiに接続する用
  strcpy(miniServer->pwd,conf.WIFI_PWD) ;            // Wi-Fiに接続する用
  strcpy(miniServer->lineHost,conf.LINEHOST);   // LINE Notify API URL
  strcpy(miniServer->lineToken,conf.LINETOKEN); // LINE Notify API Token
  miniServer->sunny = conf.SENSER_SUNNY ;
  miniServer->rain = conf.SENSER_RAIN ;
  miniServer->SenserVal = rainSenser.getAvrT( ) ;

  SetColorLED(colorOrange) ;
}

// ----- StopMiniServer -----
void StopMiniServer( ) {
  SetColorLED(colorRed) ;
  miniServer->EndServerMode( ) ;
//  delete miniServer ;
//  miniServer = NULL ;
  SetColorLED(colorGreen) ;
}

// ----- SendLineMessage ----
void SendLineMessage(int messageNo) {
  lineNotify->setSSID(conf.WIFI_SSID) ;
  lineNotify->setPWD(conf.WIFI_PWD) ;
  lineNotify->setLineHost(conf.LINEHOST) ;
  lineNotify->setLineToken(conf.LINETOKEN) ;
  lineNotify->sendLINE(messageNo) ;
}

// -----------------
// ------ LOOP -----
// -----------------
void loop() {
  // ----- 動作モード切り替え -----
  if (digitalRead(BTNPIN) == LOW) {
    if (mode == 0) {
      // Rain Senser -> HTTP Server
      StartMiniServer( ) ;
      mode = 1 ;
    } else {
      // HTTP Server -> Rain Sencer
      StopMiniServer( ) ;
      mode = 0 ;
      delay(1000) ;
    }
    while(digitalRead(BTNPIN) == LOW) {
      delay(200) ;
    }
  }
  
  if (mode == 0) {
    // ----- Rain Sencer
    int stat = rainSenser.getCapacitance( ) ;
    if (stat == 1) {
      // 雨が降ってきた
       SendLineMessage(0) ;
    } else
    if (stat == 3) {
      // 雨が上がった
       SendLineMessage(1) ;
    }
  } else {
    // ----- HTTP Server
    rainSenser.getCapacitance( ) ;
    miniServer->SenserVal = rainSenser.getAvrT( ) ;
    miniServer->handleRoot( ) ;
    if (miniServer->endStat != 0) {
      strcpy(conf.WIFI_SSID,miniServer->ssid) ;     // Wi-Fiに接続する用
      strcpy(conf.WIFI_PWD,miniServer->pwd) ;            // Wi-Fiに接続する用
      strcpy(conf.LINEHOST,miniServer->lineHost);   // LINE Notify API URL
      strcpy(conf.LINETOKEN,miniServer->lineToken); // LINE Notify API Token
      conf.SENSER_SUNNY = miniServer->sunny ;
      conf.SENSER_RAIN = miniServer->rain ;
      // --- UPDATE ---
      uint8_t *confP = (uint8_t *)&conf ;
      for (int i = 0; i < sizeof(conf); i++) {
        EEPROM.write(i , confP[i]);
      }
      EEPROM.commit();
    
      // HTTP Server -> Rain Sencer
      StopMiniServer( ) ;
      mode = 0 ;
      delay(1000) ;

      if (miniServer->endStat == 2) {
         // 送信テスト
         SendLineMessage(2) ;
      }
      SetColorLED(colorGreen) ;
      delay(1000) ;

      mode = 0 ;
    }
  }
  delay(200) ;
}
