#include <WiFi.h>
#include <WiFiClientSecure.h>

#include "LineNotify.h"

// -------------------------
// ----- 通信パラメタ設定 -----
// -------------------------
void LINENOTYFY::setSSID(char *ssid) {
  strcpy(SSID,ssid) ;
}

void LINENOTYFY::setPWD(char *pwd) {
  strcpy(PWD,pwd) ;
}

void LINENOTYFY::setLineHost(char *hostUrl) {
  strcpy(LINEHOST,hostUrl) ;
}

void LINENOTYFY::setLineToken(char *token) {
  strcpy(LINETOKEN,token) ;
}

// ----------------------
// ----- LINE に通知 -----
// ----------------------
// mode - 0 : 雨が降ってきた
// mode - 1 : 雨が上がった
// mode - 2 : 通知テスト
bool LINENOTYFY::sendLINE(int mode) {
  // Wi-Fi接続
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PWD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  // Line APIに接続
  WiFiClientSecure client;
  client.setInsecure( ) ;
  if (!client.connect(LINEHOST, 443)) {
    return false ;
  }
 
  // リクエストを送信
  String message = "" ;
  if (mode == 0) message = "雨が降ってきた"  ;
  else if (mode == 1) message = "雨が上がった" ;
  else if (mode == 2) message = "RainAlarm通知テスト" ;
  String query = String("message=") + message;
  String request = String("") +
               "POST /api/notify HTTP/1.1\r\n" +
               "Host: " + LINEHOST + "\r\n" +
               "Authorization: Bearer " + LINETOKEN + "\r\n" +
               "Content-Length: " + String(query.length()) +  "\r\n" + 
               "Content-Type: application/x-www-form-urlencoded\r\n\r\n" +
                query + "\r\n";
  client.print(request);

  // 受信終了まで待つ 
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
  }

  WiFi.disconnect(true,false) ;
  delay(2000) ;
  return true ;
}
