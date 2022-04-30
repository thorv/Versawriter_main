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
        unsigned int flag = 0;
        int idxMin = sz, idxMax = sz;
        for (size_t s = 0; s < sz; s++) {
            fixedFlt data = buf[((wp - s - 1) + size) % size];
            if (min > data) {
                flag |= 1;
                flag &= ~4;
                min = data;
                idxMin = s;
            }
            if (max < data) {
                flag |= 2;
                flag &= ~8;
                max = data;
                idxMax = s;
            }
            if (s - idxMin > 50) flag |= 4;
            if (s - idxMax > 50) flag |= 8;
            if ((flag & 0x1c) == 0xc) {
                flag |= 0x10;
                if (idxMin < idxMax) {
                    idxMin = sz;
                    min = INT_MAX;
                    flag &= ~4;
                } else {
                    idxMax = sz;
                    max = INT_MIN;
                    flag &= ~8;
                }
            }
            if ((flag & 0x1f) == 0x1f){
                break;
            }
        }
        float avg;
        if ((max - min) > (1 * 50)) {  // max-minの振れ幅が大きい=振られている
            avg = (max + min) / 200.;
        } else {
            avg = -100;  //ありえない値にしておく
        }
        return avg;
    }

    float getPrev(int step = 2) {
        return buf[(wp - step + size) % size];
    }
};