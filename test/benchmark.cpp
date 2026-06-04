#include <iostream>
#include <chrono>

struct TFT {
    volatile int dummy = 0;
    void drawPixel(int x, int y, int color) {
        dummy += x + y + color; // prevent optimization
    }
    void drawFastHLine(int x, int y, int w, int color) {
        for(int i=0; i<w; ++i) dummy += (x+i) + y + color;
    }
    void drawFastVLine(int x, int y, int h, int color) {
        for(int i=0; i<h; ++i) dummy += x + (y+i) + color;
    }
};

TFT tft;
int priColor = 1234;

void drawOld(int x, int y) {
    int startX = x + 32;
    int startY = y + 12;
    int endX = x + 52;
    int endY = y + 32;
    int step = 2;
    int turns = 0;

    while (startX <= endX && startY <= endY && turns < 3) {
        for (int i = startX; i <= endX; i++) { tft.drawPixel(i, startY, priColor); }
        startY += step;
        for (int i = startY; i <= endY; i++) { tft.drawPixel(endX, i, priColor); }
        endX -= step;
        for (int i = endX; i >= startX; i--) { tft.drawPixel(i, endY, priColor); }
        endY -= step;
        for (int i = endY; i >= startY; i--) { tft.drawPixel(startX, i, priColor); }
        startX += step;
        turns++;
    }
}

void drawNew(int x, int y) {
    int startX = x + 32;
    int startY = y + 12;
    int endX = x + 52;
    int endY = y + 32;
    int step = 2;
    int turns = 0;

    while (startX <= endX && startY <= endY && turns < 3) {
        tft.drawFastHLine(startX, startY, endX - startX + 1, priColor);
        startY += step;
        tft.drawFastVLine(endX, startY, endY - startY + 1, priColor);
        endX -= step;
        tft.drawFastHLine(startX, endY, endX - startX + 1, priColor);
        endY -= step;
        tft.drawFastVLine(startX, startY, endY - startY + 1, priColor);
        startX += step;
        turns++;
    }
}

int main() {
    const int ITERS = 1000000;

    auto t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERS; i++) drawOld(0, 0);
    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout << "Old method: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count()
              << " ms\n";

    auto t3 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERS; i++) drawNew(0, 0);
    auto t4 = std::chrono::high_resolution_clock::now();
    std::cout << "New method: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(t4-t3).count()
              << " ms\n";

    return 0;
}
