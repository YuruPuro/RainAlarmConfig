#include "MiniServer.h"

const char* ssidAtom = "RainAlarm"; // SSID 初期値
const char* pwdAtom  = "RainAlarm"; // PWD 初期値

// ▲▼▲▼ HTTPD Server ▲▼▲▼
MiniServer::MiniServer( ) {
  wiFiServer = new WiFiServer(80) ;
  
  paramNum = 0 ;

  // --- PARAM ---
  strcpy(ssid,"") ;     // Wi-Fiに接続する用
  strcpy(pwd,"") ;      // Wi-Fiに接続する用
  strcpy(lineHost,"notify-api.line.me") ;  // LINE Notify API URL
  strcpy(lineToken,"");       // LINE Notify API Token
}

MiniServer::~MiniServer( ) {
  delete wiFiServer ;
}

void MiniServer::StartServerMode( ) {
//  WiFi.mode(WIFI_STA);
  WiFi.softAP(ssidAtom, pwdAtom);
  IPAddress Ip(192, 168, 10, 10);     // Server IP を固定
  IPAddress NMask(255, 255, 255, 0);  // Server SubNetMask
  WiFi.softAPConfig(Ip, Ip, NMask);   // IP , Gateway , SubNetMask
  IPAddress myIP = WiFi.softAPIP();
  wiFiServer->begin();
}

void MiniServer::EndServerMode( ) {
  wiFiServer->end();
  WiFi.disconnect(true,true) ;
  WiFi.mode(WIFI_OFF);
}

void MiniServer::handleRoot() {
  bool flagContentRead = false ;
  int ContentLen = 0 ;
  int uriTop = 0 ;
  int grequestMode = -1 ; // 0:GET 1:POST
  char uri[16] ;

  endStat = 0 ;
  WiFiClient client = wiFiServer->available();   // listen for incoming clients
  if (client) {                             // if you get a client,
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        if (c == '\n') {                    // if the byte is a newline character
          if (currentLine.indexOf("GET ") ==0) {
            grequestMode = 0 ;
            uriTop = 4 ;
            // URL
            for (int i=uriTop;i<currentLine.length();i++) {
              if (currentLine.charAt(i) == ' ' || currentLine.charAt(i) == '?') break ;
              uri[i-uriTop] = currentLine.charAt(i) ;
              uri[i-uriTop+1] = 0x00 ;
            }
          } else
          if (currentLine.indexOf("POST ") ==0) {
            grequestMode = 1 ;
            uriTop = 5 ;
            // URL
            for (int i=uriTop;i<currentLine.length();i++) {
              if (currentLine.charAt(i) == ' ' || currentLine.charAt(i) == '?') break ;
              uri[i-uriTop] = currentLine.charAt(i) ;
              uri[i-uriTop+1] = 0x00 ;
            }
          }
          if (currentLine.indexOf("Content-Length:") ==0) {
            for (int i=15;i<currentLine.length();i++) {
              if ('0' <= currentLine.charAt(i) && currentLine.charAt(i) <='9') {
                ContentLen = ContentLen * 10 + (int)(currentLine.charAt(i) - '0') ; 
              }
            }
          }

          if (currentLine.length() == 0) {
            // 空白行 : GETの終端、POSTの区切り
            if (grequestMode == 0) {
              if (strcmp(uri,"/Senser") == 0) {
                client.println("HTTP/1.1 200 OK");
                client.println("Content-type:text/plain");
                client.println();
                client.println(SenserVal) ;
              } else {
                WebPageTop(&client) ;
              }
              break ;
            }
            currentLine = "";
            flagContentRead = true ;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
          if ( flagContentRead ) {
            ContentLen -- ;
            if (ContentLen == 0) {
              // POST のパラメタ
              // ----------------------
              getParam(currentLine) ;
              for (int i=0;i<paramNum;i++) {
                if (strcmp(param[i].param,"SSID") == 0) {
                  strcpy(ssid,param[i].value) ;
                }
                if (strcmp(param[i].param,"PWD") == 0) {
                  strcpy(pwd,param[i].value) ;
                }
                if (strcmp(param[i].param,"LINEHOST") == 0) {
                  strcpy(lineHost,param[i].value) ;
                }
                if (strcmp(param[i].param,"LINETOKEN") == 0) {
                  strcpy(lineToken,param[i].value) ;
                }
                if (strcmp(param[i].param,"SUNNY") == 0) {
                  long v = 0 ;
                  for (int j=0;param[i].value[j]!=0;j++) {
                    if ('0'<= param[i].value[j] && param[i].value[j] <= '9') {
                      v = v * 10 + (long)(param[i].value[j] - '0') ;
                    }
                  }
                  sunny = v ;
                }
                if (strcmp(param[i].param,"RAIN") == 0) {
                  long v = 0 ;
                  for (int j=0;param[i].value[j]!=0;j++) {
                    if ('0'<= param[i].value[j] && param[i].value[j] <= '9') {
                      v = v * 10 + (long)(param[i].value[j] - '0') ;
                    }
                  }
                  rain = v ;
                }
                if (strcmp(param[i].param,"SETUP") == 0) {
                  endStat = 1 ;
                }
                if (strcmp(param[i].param,"LINETEST") == 0) {
                  endStat = 2 ;
                }
              }
              if (endStat == 1) {
                WebPageSET(&client) ;
              } else
              if (endStat == 2) {
                WebPageTEST(&client) ;
              } else {
                WebPageTop(&client) ;
              }
              break ;
            }
          }
        }
      }
    }
    client.stop();
  }
}

void MiniServer::getParam(String currentLine) {
  bool flagParam = true ;
  int hexLen = 0 ;
  uint8_t hexVal = 0 ;
  
  int len = 0 ;
  char ch ;
  paramNum = 0 ;
  for (int i=0;i<PARAMU_NUM;i++) {
     param[i].param[0] = 0 ;
     param[i].value[0] = 0 ;
  }
  for (int i=0;i<currentLine.length();i++) {
    ch = currentLine.charAt(i) ;
    if (hexLen > 0) {
      if ('0' <= ch && ch <='9') {
        hexVal = hexVal * 16 + (uint8_t)(ch - '0') ;
        hexLen ++ ;
      } else
      if ('A' <= ch && ch <='F') {
        hexVal = hexVal * 16 + (uint8_t)(ch - 'A') + 10 ;
        hexLen ++ ;
      } else
      if ('a' <= ch && ch <='f') {
        hexVal = hexVal * 16 + (uint8_t)(ch - 'a') + 10 ;
        hexLen ++ ;
      }
      if (hexLen == 3) {
        if (flagParam) {
          param[paramNum].param[len ++] = hexVal ;
          param[paramNum].param[len] = 0x00 ;
        } else {
          param[paramNum].value[len ++] = hexVal ;
          param[paramNum].value[len] = 0x00 ;
        }
        hexLen = 0 ;
      }
      continue ;
    }
    if (ch == '%') {
      hexLen = 1 ;
      continue ;
    }
    if (ch == '=') {
      flagParam = false ;
      len  = 0 ;
      continue ;
    }
    if (ch == '&') {
      flagParam = true ;
      len  = 0 ;
      paramNum ++ ;
      if (paramNum >= PARAMU_NUM) {
        break ;
      }
      continue ;
    }
    if (ch == '+') {
      ch = ' ' ;
    }

    if (flagParam) {
      param[paramNum].param[len ++] = ch ;
      param[paramNum].param[len] = 0x00 ;
    } else {
      param[paramNum].value[len ++] = ch ;
      param[paramNum].value[len] = 0x00 ;
    }
  }
  if (param[0].param[0] != 0) {
    paramNum ++ ;
  }
}

// ----- 設定画面 -----
void MiniServer::WebPageTop(WiFiClient *client) {
  client->println("HTTP/1.1 200 OK");
  client->println("Content-type:text/html");
  client->println();
  client->println("<!DOCTYPE html>") ;
  client->println("<html><head><meta charset='UTF-8'>") ;
  client->println("<script >") ;
  client->println("<!--") ;
  client->println("var order1 = 0 ;") ;
  client->println("function StartSenser() {") ;
  client->println("  clearInterval(order1) ;") ;
  client->println("  order1 = setInterval('dispSenserVal()',3000) ;") ;
  client->println("} ;") ;
  client->println("function StopSenser() {") ;
  client->println("  clearInterval(order1) ;") ;
  client->println("} ;") ;
  client->println("function dispSenserVal( ) {") ;
  client->println("  var xhr = new XMLHttpRequest();") ;
  client->println("  xhr.open('GET','/Senser',true);") ;
  client->println("  xhr.onreadystatechange = function() {") ;
  client->println("    if (xhr.readyState  === 4 && xhr.status == 200) {") ;
  client->println("      var sValv = xhr.responseText ;") ;
  client->println("      if (sValv == -1) {") ;
  client->println("        clearInterval(order1) ;") ;
  client->println("        return ;") ;
  client->println("      }") ;
  client->println("      document.getElementById('NOW').value = sValv;") ;
  client->println("    }") ;
  client->println("  } ;") ;
  client->println("  xhr.send();") ;
  client->println("};") ;
  client->println("-->") ;
  client->println("</script>") ;
  client->println("</head>") ;

  client->println("<body onload='StartSenser()'><div arign='center'><font size=+4>Rain Alarm SETUP</font></div>") ;
  client->println("<form action='SELECT' accept-charset='UTF-8' method='POST'><table><tbody>") ;
  client->print("<tr><td align='right'>SSID</td><td><input name='SSID' size='16' maxlength='31' type='text' value='") ;
  client->print(ssid) ;
  client->println("'></td></tr>") ;
  client->print("<td align='right'>PASSWORD</td><td><input name='PWD' size='16' maxlength='31' type='text' value='") ;
  client->print(pwd) ;
  client->println("'></td></tr>") ;
  client->println("<tr><td colspan='4'></td></tr>") ;
  client->print("<tr><td>LINE HOST</td><td colspan='3'><input name='LINEHOST' size='50' maxlength='63' type='text' value='") ;
  client->print(lineHost) ;
  client->println("'></td></tr>") ;
  client->print("<tr><td>LINE TOKEN</td><td colspan='3'><input  name='LINETOKEN' size='50' maxlength='63' type='text' value='") ;
  client->print(lineToken) ;
  client->println("'></td></tr>") ;

  client->print("<tr><td align='right'>SUNNY</td><td><input name='SUNNY' size='6' maxlength='6' type='text' value='") ;
  client->print(sunny) ;
  client->println("'></td></tr>") ;
  client->print("<tr><td align='right'>RAIN</td><td><input name='RAIN' size='6' maxlength='6' type='text' value='") ;
  client->print(rain) ;
  client->println("'></td></tr>") ;

  client->print("<tr><td align='right'>NOW</td><td><input readonly id='NOW' size='6' type='text' value='") ;
  client->print(SenserVal) ;
  client->println("'></td></tr>") ;

  client->println("<tr><td></td><td colspan='3'><input type='submit' name='LINETEST' value='LINE Notify TEST' onclick='StopSenser();' style='width:180px'></td></tr>") ;
 client->println("<tr><td colspan='4'></td></tr>") ;
 client->println("<tr><td colspan='4'></td></tr>") ;
  client->println("<tr><td></td><td colspan='3'><input type='submit' name='SETUP' value='設定' onclick='StopSenser();' style='width:180px'></td></tr>") ;
  client->println("</tbody></table></form></body></html>") ;
}

// ----- 設定完了画面 -----
void MiniServer::WebPageSET(WiFiClient *client) {
  client->println("HTTP/1.1 200 OK");
  client->println("Content-type:text/html");
  client->println();
  client->println("<!DOCTYPE html>") ;
  client->println("<html><head><meta charset='UTF-8'>") ;
  client->println("<body><div arign='center'><font size=+4>Rain Alarm CONFIG</font></div>") ;
  client->println("<table><tbody>") ;
  client->print("<tr><td align='right'>SSID:</td><td>") ;
  client->print(ssid) ;
  client->println("</td></tr>") ;
  client->print("<td align='right'>PASSWORD:</td><td>") ;
  client->print(pwd) ;
  client->println("</td></tr>") ;
  client->println("<tr><td colspan='4'></td></tr>") ;
  client->print("<tr><td>LINE HOST:</td><td colspan='3'>") ;
  client->print(lineHost) ;
  client->println("</td></tr>") ;
  client->print("<tr><td>LINE TOKEN:</td><td colspan='3'>") ;
  client->print(lineToken) ;
  client->println("</td></tr>") ;

  client->print("<tr><td align='right'>SUNNY:</td><td>") ;
  client->print(sunny) ;
  client->println("</td></tr>") ;
  client->print("<tr><td align='right'>RAIN:</td><td>") ;
  client->print(rain) ;
  client->println("</td></tr>") ;
  client->println("</tbody></table></body></html>") ;
}

// ----- テスト送信画面 -----
void MiniServer::WebPageTEST(WiFiClient *client) {
  client->println("HTTP/1.1 200 OK");
  client->println("Content-type:text/html");
  client->println();
  client->println("<!DOCTYPE html>") ;
  client->println("<html><head><meta charset='UTF-8'>") ;
  client->println("<body><div arign='center'><font size=+4>TEST MESSEAGE SEND to LINE NOTIFY.</font></div>") ;
  client->println("</body></html>") ;
}
