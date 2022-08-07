/**
 * RAIN SENSER
 */

// 移動平均用
#define  AVRNUM 10 // 移動平均のサンプル数

class SENSER {
  private:
    int    BorderRain ;   // 雨降りと判定する境界値
    int    BorderSunny ;  // 雨上がりと判定する境界値
    int    avrPos;        // 移動平均用：ポインタ
    long   avr[AVRNUM] ;  // 移動平均用計測値
    long   avrT ;         // 時定数-T
    int    stat ;         // 0:晴れ 1:雨が降ってきた 2:雨 3:雨が上がった
    bool   flagRain ;     // 雨フラグ

  public:
    SENSER( ) ;           // コンストラクタ
    void init( ) ;            // 移動平均初期化
    int getCapacitance( ) ;   // 天候ステータス
    long getAvrT( ) { return avrT ; } ;
    int  getBorderRain( ) { return BorderRain ; }
    int  getBorderSunny( ) { return BorderSunny ; }
 } ;
 
