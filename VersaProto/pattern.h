// thkana作
// 「ビジュアルポイ」的な表現ツールのためのプログラム (pattern.h)
// 使用、配布、改造等について制限を設けません。自由に使用して頂いて結構です。
// ただし、動作等については無保証です。
// 2022/3/29版

#pragma once
#include <cstddef>
#include <FastLED.h>

const int LEDNUM = 72;  // LED 一列の個数

// パターン処理の「型紙」
class Pattern {
   public:
    virtual void init() = 0;
    virtual void getLed(CRGB leds[], int ms = 0, bool imuUpdate = 0, float x = 0, float y = 0, float z = 0) = 0;
    virtual ~Pattern(){};
};

//パターン開始時間(全体スタート基準)と点灯パターンを合わせて一つの「ストーリー」とする//
struct story {
    const char* name;
    const unsigned long startTime;
    Pattern& pattern;
};

extern const story storyBoard[];
extern const size_t storySize;