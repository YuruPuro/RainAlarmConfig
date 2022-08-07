/**
 * MiniServer
 */
#ifndef _Mini_Server_H_
#define _Mini_Server_H_

#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>

#define PARAMU_NUM 12
#define DEBUG true

typedef struct param_s {
      char param[16] ;
      char value[80] ;
} param_t ;

class MiniServer
{
  private:
    WiFiServer  *wiFiServer ;
    void getParam(String currentLine) ;
    void WebPageTop(WiFiClient *client) ;
    void WebPageSET(WiFiClient *client) ;
    void WebPageTEST(WiFiClient *client) ;

    int paramNum ;
    param_t param[PARAMU_NUM] ;   

  public:
    MiniServer( ) ;   // コンストラクタ
    ~MiniServer( ) ;  // デストラクタ
    void StartServerMode( ) ;
    void EndServerMode( ) ;
    void handleRoot( );

    char ssid[32] ;     // Wi-Fiに接続する用
    char pwd[32] ;      // Wi-Fiに接続する用
    char lineHost[64];  // LINE Notify API URL
    char lineToken[64]; // LINE Notify API Token
    long sunny ;
    long rain ;
    long SenserVal ;
    int  endStat ;      // 終了モード  0:継続 1:設定変更 2:テスト送信
} ;
#endif
