// バーサライタ試行 (ビジュアルポイもどきからの派生として)
// メイン

/*****
Copyright 2022 thkana
ソースコード形式かバイナリ形式か、変更するかしないかを問わず、以下の条件を満たす場合に限り、
再頒布および使用が許可されます。

1. ソースコードを再頒布する場合、上記の著作権表示、本条件一覧、および下記免責条項を含めること。
2. バイナリ形式で再頒布する場合、頒布物に付属のドキュメント等の資料に、上記の著作権表示、
本条件一覧、および下記免責条項を含めること。

本ソフトウェアは、著作権者およびコントリビューターによって「現状のまま」提供されており、
明示黙示を問わず、商業的な使用可能性、および特定の目的に対する適合性に関する暗黙の保証も含め、
またそれに限定されない、いかなる保証もありません。著作権者もコントリビューターも、事由のいかんを
問わず、 損害発生の原因いかんを問わず、かつ責任の根拠が契約であるか厳格責任であるか
（過失その他の）不法行為であるかを問わず、仮にそのような損害が発生する可能性を知らされて
いたとしても、本ソフトウェアの使用によって発生した（代替品または代用サービスの調達、使用の喪失、
データの喪失、利益の喪失、業務の中断も含め、またそれに限定されない）直接損害、間接損害、
偶発的な損害、特別損害、懲罰的損害、または結果損害について、一切責任を負わないものとします。
*****/

#if defined(ARDUINO_M5Stick_C)
#include <M5StickC.h>
#include <utility/MPU6886.h>
auto &mpu6886 = M5.Mpu6886;  // MPU6886シンボルが隠されているので仕方なくauto
Button &Btn = M5.BtnA;
const int LEDPIN = 33;  // Groveコネクタ1pin
#elif defined(ARDUINO_M5Stack_ATOM)
#include <M5Atom.h>
#include <utility/MPU6886.h>
MPU6886 &mpu6886 = M5.IMU;
Button &Btn = M5.Btn;
const int LEDPIN = 32;  // Groveコネクタ1pin
#endif

// Arduino\libraries\FastLED\src\chipsets.h 578行目付近調整で多少転送レート上げられる
#include <FastLED.h>
#include "pattern.h"
#include "timeSync.h"

CRGB leds[LEDNUM];     // FastLEDライブラリで使用するLEDの定義
int lps = 10000 / 23;  // 72LEDの場合更新周期がおよそ2.3ミリ秒なので、1秒あたり10000/23列表示

int idx;  // ストーリーの番号

float accX = 0.0f;
float accY = 0.0f;
float accZ = 0.0f;

// IMUの測定完了を調べる
bool getImuReady() {
    int driver_Addr = MPU6886_ADDRESS;
    int start_Addr = 0x3a;  // MPU6886_INT_STATUS
    Wire1.beginTransmission(driver_Addr);
    Wire1.write(start_Addr);
    Wire1.endTransmission(false);
    Wire1.requestFrom(driver_Addr, 1);
    return Wire1.read() & 1;
}

//デバッグ/モニタ用 開始するストーリーの名前を表示する
void showCurrentBoard(int index) {
    Serial.print(storyBoard[index].startTime);
    Serial.print(": ");
    Serial.println(storyBoard[index].name);
}

//最初から実行するように変数等を初期化する
void statInit() {
    idx = 0;
    storyBoard[idx].pattern.init();
    //    FastLED.clear();
    for (auto &c : leds) {
        c = CRGB::Black;
    }
    FastLED.show();
    while (updateSync() < storyBoard[0].startTime) {
    }
    showCurrentBoard(idx);
}

void setup() {
#if defined(ARDUINO_M5Stick_C)
    M5.begin();
    M5.Axp.ScreenBreath(10);  // LCDのバックライトを暗く点灯する
    M5.Lcd.println("Waiting...");
#elif defined(ARDUINO_M5Stack_ATOM)
    M5.begin(true, false, true);
    M5.dis.clear();
    M5.dis.drawpix(0, CRGB(0, 255, 0));
    M5.update();
#endif

    mpu6886.Init();
    mpu6886.SetAccelFsr(mpu6886.AFS_16G);  // IMU 感度切替

    setupSync();
    if (amITRef()) {
#if defined(ARDUINO_M5Stick_C)
        M5.Lcd.println("I am time reference.");
#elif defined(ARDUINO_M5Stack_ATOM)
        M5.dis.drawpix(0, CRGB(32, 0, 0));
#endif
    } else {
#if defined(ARDUINO_M5Stick_C)
        M5.Lcd.println("Sync to refrence.");
#elif defined(ARDUINO_M5Stack_ATOM)
        M5.dis.drawpix(0, CRGB(0, 0, 32));
#endif
    }

    Serial.println("START.");
    FastLED.addLeds<WS2812B, LEDPIN, GRB>(leds, LEDNUM);  // FastLEDライブラリの初期化 LEDの種類によって変更が必要な場合あり
    FastLED.setBrightness(32);                            //動画等を撮るときはここを10ぐらいまで下げた方が綺麗に取れる場合も
    FastLED.setMaxRefreshRate(lps);                       // lpsは1秒あたりの表示列数

    statInit();
}

void loop() {
    if (amITRef()) {  //最初に戻せるのは基準機だけ
        M5.update();
        //ボタンが押されたら最初から
        if (Btn.wasPressed()) {
#if defined(ARDUINO_M5Stick_C)
        M5.Axp.ScreenBreath(10);  // LCDのバックライトを暗く点灯する
#elif defined(ARDUINO_M5Stack_ATOM)
            M5.dis.drawpix(0, CRGB(32, 0, 0));
#endif
            resetTRef();
            statInit();
        }
    } else if (chkRewindReq()) {  //基準機からの巻き戻し要求
        statInit();
    }
    unsigned long t = updateSync();
    if (t > 5000) {
#if defined(ARDUINO_M5Stick_C)
        M5.Axp.ScreenBreath(0);  // LCDのバックライトをOffにする
#elif defined(ARDUINO_M5Stack_ATOM)
        M5.dis.drawpix(0, CRGB::Black);  // M5AtomのLED0番消灯
#endif
    }
    //時間がきたらストーリーの切り替え
    if (idx < storySize - 1) {                     //現在が最後のストーリーでないなら
        if (t >= storyBoard[idx + 1].startTime) {  //次のストーリーの時間?
            idx++;                                 //次のストーリーへ進む
            showCurrentBoard(idx);                 //ストーリー名をシリアルに表示
            storyBoard[idx].pattern.init();
        }
    }

    bool imuUpdate = false;
    if (getImuReady()) {  // IMUのデータ更新あり
        imuUpdate = true;
        mpu6886.getAccelData(&accX, &accY, &accZ);
    }

    storyBoard[idx].pattern.getLed(leds, t - storyBoard[idx].startTime, imuUpdate, accX, accY, accZ);

    FastLED.show();  //設定したデータをLEDに転送
}
