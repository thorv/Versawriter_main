// バーサライタ試行 (ビジュアルポイもどきからの派生として)
// 複数機体同期機構

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

#include <Arduino.h>
#if defined(ARDUINO_M5Stick_C)
#include <M5StickC.h>
#include <utility/MPU6886.h>
Button& BtnSync = M5.BtnA;
#elif defined(ARDUINO_M5Stack_ATOM)
#include <M5Atom.h>
Button& BtnSync = M5.Btn;
#elif defined(ARDUINO_M5Stack_Core_ESP32)
#include <M5Stack.h>
Button& BtnSync = M5.BtnA;
#endif

#include "timeSync.h"

#include <esp_now.h>
#include <WiFi.h>

#ifndef WIFI_STA //VSCodeのコード解析のエラー回避
#define WIFI_STA 0
#endif

const int REFPERIOD = 5000;  //時刻同期の間隔
unsigned long t0;
unsigned long tp;
volatile unsigned long tDelta;
volatile bool tRefFlag;
esp_now_peer_info_t slave = {
    {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}  //ブロードキャスト設定
};
bool amTRef;
bool rewindReq;

static void onReceive(const uint8_t* mac_addr, const uint8_t* data, int data_len) {
    tRefFlag = true;
    unsigned long tRef;
    if (data_len >= 4) {
        tRef = *(unsigned long*)data;
        if (tRef == 0) {
            rewindReq = true;
        }
    }
    tDelta = tRef - millis();
}

void resetTRef() {
    if (amTRef) {
        t0 = millis();
        tp = 0;
        unsigned long tt = 0;
        esp_now_send(slave.peer_addr, (uint8_t*)&tt, sizeof(unsigned long));
        esp_now_send(slave.peer_addr, (uint8_t*)&tt, sizeof(unsigned long));  //念のため2度
    }
}

void setupSync() {
    WiFi.mode(static_cast<wifi_mode_t>(WIFI_STA));
    WiFi.disconnect();
    if (esp_now_init() != ESP_OK) {
        ESP.restart();
    }
    esp_now_register_recv_cb(onReceive);
    // Aボタンが押されるか同期信号を受信するまで待機
    while (1) {
        if (tRefFlag) {
            t0 = 0;
            break;
        }
        M5.update();
        if (BtnSync.wasPressed()) {  //ボタンAが押されたら基準機になる
            esp_now_add_peer(&slave);
            amTRef = true;
            resetTRef();
            tDelta = 0;
            M5.update();
            break;
        }
    }
}

unsigned long updateSync() {
    unsigned long tt = millis() + tDelta - t0;
    if (amTRef && (tt - tp) > REFPERIOD) {
        tp = tt;
        esp_now_send(slave.peer_addr, (uint8_t*)&tt, sizeof(unsigned long));
    }
    return tt;
}

bool amITRef(){
    return amTRef;
}
bool chkRewindReq(){
    bool ret=rewindReq;
    rewindReq=false;
    return ret;
}
