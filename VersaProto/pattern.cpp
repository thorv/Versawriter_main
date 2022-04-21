// バーサライタ試行 (ビジュアルポイもどきからの派生として)
// 点灯パターン生成部

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

#include "pattern.h"
#include "RingBuf.h"

//ソースコードの見やすさのため、色を定数に定義しておく
#define C_BLK \
    { 0, 0, 0 }  //黒
#define C_WHT \
    { 255, 255, 255 }  //白
#define C_RED \
    { 255, 0, 0 }  //赤
#define C_GRN \
    { 0, 255, 0 }  //緑
#define C_BLU \
    { 0, 0, 255 }  //青
#define C_YLW \
    { 255, 255, 0 }  //黄
#define C_MGT \
    { 255, 0, 255 }  //紫
#define C_CYN \
    { 0, 255, 255 }  //水色

#define C_RDD \
    { 64, 0, 0 }  //明るさを抑えた赤
#define C_BLD \
    { 0, 0, 64 }  //明るさを抑えた青

#define C_ALLBLK                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               \
    {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          \
        C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK, C_BLK \
    }

#define C_ALLRED                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               \
    {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          \
        C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED \
    }

#define C_ALLBLU                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               \
    {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          \
        C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU, C_BLU \
    }

//実験段階で電力を抑えるための明るさを抑えたパターン
#define C_ALLRDD                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               \
    {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          \
        C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD, C_RDD \
    }

#define C_ALLBLD                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               \
    {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          \
        C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD, C_BLD \
    }

/////////////////
//点灯の仕方の定義
/////////////////

//一つのパターンの定義 一列の光り方
// 配列の二次元目の各要素が1/lps時間毎のパターン

const char pchar_abc
#include "char_abc.h"
    ;

const char pkizuna
#include "kizuna.h"
    ;

const char pnum
#include "num.h"
    ;

const char pda2
#include "da2.h"
    ;

const char pfr2
#include "fr2.h"
    ;

const char plove_a
#include "love_a.h"
    ;

/////////////////
//点灯の仕方の定義
/////////////////

//////////////
//単純なパターン点灯の繰り返し

#define PTN(x) SimplePattern x(p##x, sizeof(p##x) / sizeof(p##x[0]))

class SimplePattern : public Pattern {
   private:
    const char (*ptn)[LEDNUM][3];
    size_t size;
    int lineNum;

   public:
    SimplePattern(const char p[][LEDNUM][3], int sz) : ptn(p), size(sz){};
    void init() { lineNum = 0; };
    void getLed(CRGB leds[], int ms = 0, bool imuUpdate = 0, float x = 0, float y = 0, float z = 0) {
        for (int i = 0; i < LEDNUM; i++) {
            leds[i] =
                CRGB(ptn[lineNum][i][0], ptn[lineNum][i][1], ptn[lineNum][i][2]);
        }
        lineNum = (lineNum + 1) % size;
    };
};

//点灯パターンの定義
PTN(char_abc);
PTN(kizuna);
PTN(num);
PTN(fr2);
PTN(da2);

////////////////////////
// 絶対時間や点灯周期ベースの時間で点灯を制御する

class Wave : public Pattern {
   private:
    int lineNum;

   public:
    void init() { lineNum = 0; };
    void getLed(CRGB leds[], int ms = 0, bool imuUpdate = 0, float x = 0, float y = 0, float z = 0) {
        for (int i = 0; i < LEDNUM; i++) {
            if (i == lineNum % LEDNUM) {
                const char CY[3] = C_YLW;
                leds[i] = CRGB(CY[0], CY[1], CY[2]);
            } else if (i == LEDNUM - (lineNum % LEDNUM) - 1) {
                const char CM[3] = C_MGT;
                leds[i] = CRGB(CM[0], CM[1], CM[2]);
            } else {
                leds[i] = CRGB::Black;
            }
        }
        lineNum = (lineNum + 1) % LEDNUM;
    }
} wave;

class Gradation : public Pattern {
   private:
    int lineNum;

   public:
    void init() { lineNum = 0; };
    void getLed(CRGB leds[], int ms = 0, bool imuUpdate = 0, float x = 0, float y = 0, float z = 0) {
        for (int i = 0; i < LEDNUM; i++) {
            leds[i] = CHSV((uint8_t)(255 * (ms % 4096) / 4096), 255, 255);
        }
    }
} gradation;

//////////////////////////////
//衝撃(加速度)検出による点灯パターン

class Hit1 : public Pattern {
    int cnt;

   public:
    void init() { cnt = LEDNUM; }
    void getLed(CRGB leds[], int ms, bool imuUpdate, float x, float y, float z) {
        if (x * x + y * y + z * z > 4) {
            cnt = 0;
        }
        if (cnt < LEDNUM) {
            for (int i = 0; i < LEDNUM; i++) {
                leds[i] = CRGB(CRGB::Black);
                if (i == cnt) {
                    leds[i] = CRGB(255 * (LEDNUM - cnt - 1) / (LEDNUM - 1), 0,
                                   255 * cnt / (LEDNUM - 1));
                }
            }
            cnt++;
        } else {
            for (int i = 0; i < LEDNUM; i++) {
                leds[i] = CRGB::Black;
            }
        }
    }
} hit1;

class Hit2 : public Pattern {
    int lum;
    int hue;

   public:
    void init() {}
    void getLed(CRGB leds[], int ms, bool imuUpdate, float x, float y, float z) {
        if (x * x + y * y + z * z > 4) {
            lum = 255;
            hue = ms % 256;
        }
        for (int i = 0; i < LEDNUM; i++) {
            leds[i] = CHSV(hue, 255, lum);
        }
        lum *= 0.97;
    }
} hit2;

//////////////////////////////
// バーサライタ点灯

#define VWT(x) Versawrite v##x(p##x, sizeof(p##x) / sizeof(p##x[0]))
class Versawrite : public Pattern {
   private:
    const char (*ptn)[LEDNUM][3];
    size_t size;
    int lineNum;
    int period[2];
    int cnt;
    RingBuf rbuf;
    bool sync;
    int delta = 1;

   public:
    Versawrite(const char p[][LEDNUM][3], int sz) : ptn(p), size(sz){

                                                            };
    void init() {
        lineNum = size;
    }

    void getLed(CRGB leds[], int ms, bool imuUpdate, float accX, float accY, float accZ) {
        float acc = accX;
        bool zx = false;
        if (imuUpdate) {
            rbuf.push(acc);
            float avgVal = rbuf.getStatistics();
            if ((rbuf.getPrev() / 100. - avgVal) * (acc - avgVal) < 0 && (acc - avgVal) > 0) {  //ゼロクロス
                zx = true;
                period[1] = period[0];
                period[0] = cnt;
                cnt = 0;
                if (period[0] * 0.8 < period[1] && period[0] > period[1] * 0.8) {
                    sync = true;
                } else {
                    sync = false;
                }
            }
        }
        //対応するLED点灯
        if (0 <= lineNum && lineNum < size) {
            for (int i = 0; i < LEDNUM; i++) {
                leds[i] =
                    CRGB(ptn[lineNum][i][0], ptn[lineNum][i][1], ptn[lineNum][i][2]);
            }
        } else {
            for (int i = 0; i < LEDNUM; i++) {
                leds[i] = CRGB::Black;
            }
        }
        cnt++;
        lineNum += delta;
        if (period[0] - cnt == size / 2) {
            lineNum = 0;
            delta = 1;
        }
        if (sync && period[0] / 2 - cnt == size / 2) {
            lineNum = size - 1;
            delta = -1;
        }
    }
};
VWT(num);
VWT(kizuna);
VWT(da2);
VWT(fr2);
VWT(love_a);

//////////////////
//終了 (全消灯)

class End : public Pattern {
   public:
    void init() {}
    void getLed(CRGB leds[], int ms = 0, bool imuUpdate = 0, float x = 0, float y = 0, float z = 0) {
        for (int i = 0; i < LEDNUM; i++) {
            leds[i] = CRGB::Black;
        }
    }
} end;

//ストーリーボード
//時間とパターンの組み合わせ「ストーリー」を再生順に、時間指定で並べる
//各要素は
//  {ストーリー名称文字列(内容任意), ストーリー開始時刻, 点灯パターン名 }
const story storyBoard[] = {
    {"wave", 1000, wave},
    {"fruit", 30000, vfr2},
    {"number", 60000, num},
    {"Hit", 90000, hit1},
    {"END", 0x7fffffff, end}};

const size_t storySize = sizeof(storyBoard) / sizeof(storyBoard[0]);
