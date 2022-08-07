/**
 * LINR Notify-API
 */
class LINENOTYFY {
  private:
    char SSID[32] ;    // Wi-Fiに接続する用
    char PWD[32] ;     // Wi-Fiに接続する用
    char LINEHOST[64];  // LINE Notify API URL
    char LINETOKEN[64]; // LINE Notify API Token
  
  public:
    void setSSID(char *ssid) ;
    void setPWD(char *pwd) ;
    void setLineHost(char *hostUrl) ;
    void setLineToken(char *token) ;
    bool sendLINE(int mode) ;
} ;
