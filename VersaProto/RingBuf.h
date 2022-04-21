#pragma once
//バーサライタの「振り」の検出に使用するために作成

typedef int fixedFlt;  //固定小数点型を意図 x100

class RingBuf {
   private:
    fixedFlt* buf;
    const unsigned int size;
    unsigned int wp;

   public:
    RingBuf(unsigned int sz = 256) : size(sz) {
        buf = new fixedFlt[size];
        for (int i = 0; i < size; i++) {
            buf[i] = 0;
        }
        wp = 0;
    }

    void push(float data) {
        buf[wp] = data * 100;
        if (++wp >= size) {
            wp = 0;
        }
    }

    // return tuple (min,max, avg)
    float getStatistics(int sz = -1) {
        fixedFlt max = INT_MIN, min = INT_MAX;
        if ((sz < 0) || (sz > size)) {
            sz = size;
        }
        for (size_t s = 0; s < sz; s++) {
            fixedFlt data = buf[(wp + s) % size];
            if (min > data) min = data;
            if (max < data) max = data;
        }
        float avg;
        if ((max - min) > (1 * 100)) {  // max-minの振れ幅が大きい=振られている
            avg = (max + min) / 200.;
        } else {
            avg = -100;//ありえない値にしておく
        }
        return avg;
    }

    float getPrev(int step = 2) {
        return buf[(wp - step + size) % size];
    }
};