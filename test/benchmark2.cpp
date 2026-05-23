#include <iostream>
#include <chrono>

struct TFT {
    volatile long long spi_bytes = 0;
    volatile long long spi_transactions = 0;

    void spi_write(int bytes) {
        spi_bytes += bytes;
        spi_transactions++;
    }

    void drawPixel(int x, int y, int color) {
        // Overhead of setting window + writing 1 pixel (2 bytes)
        spi_write(4 + 4 + 2);
    }

    void drawFastHLine(int x, int y, int w, int color) {
        // Overhead of setting window + writing w pixels (w * 2 bytes)
        spi_write(4 + 4 + w * 2);
    }

    void drawFastVLine(int x, int y, int h, int color) {
        // Overhead of setting window + writing h pixels (h * 2 bytes)
        spi_write(4 + 4 + h * 2);
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
    tft.spi_transactions = 0;
    tft.spi_bytes = 0;
    drawOld(0, 0);
    std::cout << "Old method - SPI transactions: " << tft.spi_transactions << ", SPI bytes: " << tft.spi_bytes << "\n";

    tft.spi_transactions = 0;
    tft.spi_bytes = 0;
    drawNew(0, 0);
    std::cout << "New method - SPI transactions: " << tft.spi_transactions << ", SPI bytes: " << tft.spi_bytes << "\n";

    return 0;
}
