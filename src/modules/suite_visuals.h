#ifndef __SUITE_VISUALS_H__
#define __SUITE_VISUALS_H__

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "core/display.h"
#include "core/config.h"

// ============================================================================
// Suite Visual Utilities — Smooth Animation/Transition System
// Uses TFT_eSPI anti-aliased drawing: drawSmoothArc, drawSmoothCircle,
// drawSmoothRoundRect, fillSmoothRoundRect, drawWideLine
// ============================================================================

// --- Timing ---
inline unsigned long suiteMillis() { return millis(); }

// --- Easing Functions ---
inline float easeOutCubic(float t) { return 1.0f - powf(1.0f - t, 3.0f); }
inline float easeOutQuad(float t) { return t * (2.0f - t); }
inline float easeInOutQuad(float t) { return t < 0.5f ? 2.0f * t * t : 1.0f - powf(-2.0f * t + 2.0f, 2.0f) / 2.0f; }
inline float easeOutBack(float t) { const float c1 = 1.70158f; const float c3 = c1 + 1.0f; return 1.0f + c3 * powf(t - 1.0f, 3.0f) + c1 * powf(t - 1.0f, 2.0f); }
inline float easeOutElastic(float t) { if (t == 0.0f || t == 1.0f) return t; return powf(2.0f, -10.0f * t) * sinf((t * 10.0f - 0.75f) * ((2.0f * PI) / 3.0f)) + 1.0f; }

// --- Color Helpers ---
inline uint16_t suiteLerpColor(uint16_t c1, uint16_t c2, float t) {
    uint8_t r1 = (c1 >> 11) & 0x1F, g1 = (c1 >> 5) & 0x3F, b1 = c1 & 0x1F;
    uint8_t r2 = (c2 >> 11) & 0x1F, g2 = (c2 >> 5) & 0x3F, b2 = c2 & 0x1F;
    uint8_t r = r1 + (int)((r2 - r1) * t);
    uint8_t g = g1 + (int)((g2 - g1) * t);
    uint8_t b = b1 + (int)((b2 - b1) * t);
    return (r << 11) | (g << 5) | b;
}

inline uint16_t suitePulseColor(uint16_t base, uint16_t target, float phase) {
    float t = (sinf(phase * 2.0f * PI) + 1.0f) / 2.0f;
    return suiteLerpColor(base, target, t);
}

inline uint16_t suiteDimColor(uint16_t color, float dim) {
    uint8_t r = (color >> 11) & 0x1F;
    uint8_t g = (color >> 5) & 0x3F;
    uint8_t b = color & 0x1F;
    return ((uint8_t)(r * dim) << 11) | ((uint8_t)(g * dim) << 5) | (uint8_t)(b * dim);
}

// ============================================================================
// 0. SMOOTH DRAWING PRIMITIVES (TFT_eSPI anti-aliased)
// ============================================================================

// Smooth rounded rect card with anti-aliased corners
inline void suiteSmoothCard(int x, int y, int w, int h, uint16_t fillColor, uint16_t borderColor, int radius = 6) {
    tft.fillSmoothRoundRect(x, y, w, h, radius, fillColor, wilyConfig.bgColor);
    tft.drawSmoothRoundRect(x, y, radius, radius - 1, w, h, borderColor, wilyConfig.bgColor);
}

// Smooth circle button with glow
inline void suiteSmoothCircleBtn(int cx, int cy, int r, uint16_t fillColor, uint16_t glowColor = 0) {
    tft.fillSmoothCircle(cx, cy, r, fillColor, wilyConfig.bgColor);
    if (glowColor) {
        tft.drawSmoothCircle(cx, cy, r + 2, suiteDimColor(glowColor, 0.3f), wilyConfig.bgColor);
    }
}

// Smooth progress arc (anti-aliased)
inline void suiteSmoothProgressArc(int cx, int cy, int r, int pct, uint16_t color) {
    int endAngle = (int)(360.0f * pct / 100.0f);
    tft.drawSmoothArc(cx, cy, r, r - 4, 0, 360, suiteDimColor(color, 0.15f), wilyConfig.bgColor, false);
    if (endAngle > 0) {
        tft.drawSmoothArc(cx, cy, r, r - 4, 0, endAngle, color, wilyConfig.bgColor, true);
    }
}

// Animated glow ring around a circle
inline void suiteGlowRing(int cx, int cy, int r, uint16_t color, float phase, int thickness = 3) {
    tft.drawSmoothArc(cx, cy, r + thickness, r - thickness,
                       (int)(phase * 360) % 360, ((int)(phase * 360) % 360) + 60,
                       color, wilyConfig.bgColor, true);
}

// ============================================================================
// 1. SCREEN TRANSITIONS
// ============================================================================

// Slide transition: old screen slides out, new screen slides in
inline void suiteTransitionSlide(int durationMs = 200) {
    int steps = 8;
    int delayPerStep = durationMs / steps;
    for (int i = 1; i <= steps; i++) {
        float t = (float)i / steps;
        int offset = (int)(tftWidth * easeOutCubic(t));
        tft.scroll(offset - (int)(tftWidth * easeOutCubic((float)(i-1) / steps)));
        delay(delayPerStep);
    }
    tft.scroll(0);
}

// Fade transition: screen fades to black then back
inline void suiteTransitionFade(int durationMs = 150, int steps = 6) {
    int delayPerStep = durationMs / steps;
    for (int i = 0; i < steps; i++) {
        float t = (float)i / steps;
        uint16_t color = suiteLerpColor(wilyConfig.bgColor, TFT_BLACK, t);
        tft.fillRect(0, 0, tftWidth, tftHeight, color);
        delay(delayPerStep);
    }
}

// Wipe transition: vertical bar sweeps across screen
inline void suiteTransitionWipe(uint16_t color, int durationMs = 200, bool leftToRight = true) {
    int steps = 12;
    int delayPerStep = durationMs / steps;
    int barW = tftWidth / steps;
    for (int i = 0; i < steps; i++) {
        int x = leftToRight ? i * barW : tftWidth - (i + 1) * barW;
        tft.fillRect(x, 0, barW, tftHeight, color);
        delay(delayPerStep);
    }
    tft.fillRect(0, 0, tftWidth, tftHeight, wilyConfig.bgColor);
}

// Iris transition: smooth circle expands from center
inline void suiteTransitionIris(uint16_t color, int durationMs = 250, int steps = 10) {
    int cx = tftWidth / 2;
    int cy = tftHeight / 2;
    int maxR = (int)sqrtf(cx * cx + cy * cy);
    int delayPerStep = durationMs / steps;
    for (int i = 0; i < steps; i++) {
        float t = (float)(i + 1) / steps;
        int r = (int)(easeOutCubic(t) * maxR);
        tft.fillSmoothCircle(cx, cy, r, color, wilyConfig.bgColor);
        delay(delayPerStep);
    }
}

// ============================================================================
// 2. HEADER ANIMATIONS
// ============================================================================

// Animated header with slide-in title
inline void suiteAnimHeader(const char* title, uint16_t color, int durationMs = 100) {
    tft.fillRect(0, 0, tftWidth, 30, wilyConfig.bgColor);
    tft.drawWideLine(0, 29, tftWidth, 29, 1, color);
    int steps = 6;
    int delayPerStep = durationMs / steps;
    for (int i = 0; i < steps; i++) {
        float t = (float)(i + 1) / steps;
        int xOff = (int)((1.0f - easeOutCubic(t)) * tftWidth / 2);
        tft.fillRect(0, 0, tftWidth, 29, wilyConfig.bgColor);
        tft.drawWideLine(0, 29, tftWidth, 29, 1, color);
        tft.setTextDatum(TC_DATUM);
        tft.setTextColor(color, wilyConfig.bgColor);
        tft.setTextSize(FM);
        tft.drawCentreString(title, tftWidth / 2 + xOff, 8, SMOOTH_FONT);
        tft.setTextDatum(TL_DATUM);
        tft.setTextSize(FP);
        delay(delayPerStep);
    }
}

// Animated header with glow effect
inline void suiteAnimHeaderGlow(const char* title, uint16_t color, int durationMs = 300) {
    tft.fillRect(0, 0, tftWidth, 30, wilyConfig.bgColor);
    tft.drawWideLine(0, 29, tftWidth, 29, 1, color);
    tft.setTextDatum(TC_DATUM);
    tft.setTextSize(FM);
    int steps = 10;
    int delayPerStep = durationMs / steps;
    for (int i = 0; i < steps; i++) {
        float t = (float)i / steps;
        uint16_t glowColor = suitePulseColor(wilyConfig.bgColor, color, t);
        tft.setTextColor(glowColor, wilyConfig.bgColor);
        tft.drawCentreString(title, tftWidth / 2, 8, SMOOTH_FONT);
        delay(delayPerStep);
    }
    tft.setTextColor(color, wilyConfig.bgColor);
    tft.drawCentreString(title, tftWidth / 2, 8, SMOOTH_FONT);
    tft.setTextDatum(TL_DATUM);
    tft.setTextSize(FP);
}

// Static header (no animation, for quick redraws)
inline void suiteDrawHeader(const char* title, uint16_t color) {
    drawMainBorder();
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(color, wilyConfig.bgColor);
    tft.setTextSize(FM);
    tft.drawCentreString(title, tftWidth / 2, 8, SMOOTH_FONT);
    tft.drawWideLine(10, 30, tftWidth - 10, 30, 1, color);
    tft.setTextDatum(TL_DATUM);
    tft.setTextSize(FP);
}

// ============================================================================
// 3. CARD ANIMATIONS
// ============================================================================

// Static card draw (no animation, for lists)
inline void suiteDrawCard(int y, int h, bool sel, uint16_t color = 0) {
    if (color == 0) color = wilyConfig.priColor;
    uint16_t bg = sel ? color : getColorVariation(color, 4, -1);
    uint16_t bd = sel ? TFT_WHITE : TFT_DARKGREY;
    tft.fillSmoothRoundRect(9, y + 1, tftWidth - 18, h - 2, 5, bg, wilyConfig.bgColor);
    tft.drawSmoothRoundRect(8, y, tftWidth - 16, h, 6, bd, wilyConfig.bgColor);
}

// Animated card selection with smooth glow border
inline void suiteAnimCardSelect(int x, int y, int w, int h, uint16_t color, int durationMs = 80) {
    int steps = 5;
    int delayPerStep = durationMs / steps;
    for (int i = 0; i < steps; i++) {
        float t = (float)(i + 1) / steps;
        uint16_t c = suiteLerpColor(wilyConfig.bgColor, color, easeOutCubic(t));
        tft.drawSmoothRoundRect(x, y, 6, 5, w, h, c, wilyConfig.bgColor);
        delay(delayPerStep);
    }
    tft.fillSmoothRoundRect(x + 1, y + 1, w - 2, h - 2, 5, color, wilyConfig.bgColor);
}

// Animated card deselection
inline void suiteAnimCardDeselect(int x, int y, int w, int h, uint16_t color, int durationMs = 60) {
    int steps = 4;
    int delayPerStep = durationMs / steps;
    for (int i = 0; i < steps; i++) {
        float t = (float)(i + 1) / steps;
        uint16_t c = suiteLerpColor(color, wilyConfig.bgColor, easeOutCubic(t));
        tft.fillSmoothRoundRect(x + 1, y + 1, w - 2, h - 2, 5, c, wilyConfig.bgColor);
        tft.drawSmoothRoundRect(x, y, 6, 5, w, h, getColorVariation(color, 4, -1), wilyConfig.bgColor);
        delay(delayPerStep);
    }
    tft.drawSmoothRoundRect(x, y, 6, 5, w, h, TFT_DARKGREY, wilyConfig.bgColor);
}

// Card press feedback (quick smooth flash)
inline void suiteAnimCardPress(int x, int y, int w, int h, uint16_t color) {
    tft.fillSmoothRoundRect(x + 1, y + 1, w - 2, h - 2, 5, TFT_WHITE, wilyConfig.bgColor);
    delay(30);
    tft.fillSmoothRoundRect(x + 1, y + 1, w - 2, h - 2, 5, color, wilyConfig.bgColor);
}

// ============================================================================
// 4. PROGRESS BAR ANIMATIONS
// ============================================================================

// Animated progress bar with smooth gradient fill
inline void suiteAnimProgressBar(int y, int pct, const char* label, uint16_t accentColor = TFT_GREEN) {
    int bw = tftWidth - 40;
    int bx = 20;
    int bh = 12;
    int by = y;
    // Background
    tft.fillSmoothRoundRect(bx, by, bw, bh, 4, suiteDimColor(accentColor, 0.1f), wilyConfig.bgColor);
    tft.drawSmoothRoundRect(bx, by, 4, 3, bw, bh, TFT_DARKGREY, wilyConfig.bgColor);
    // Animated fill
    int fw = (bw - 4) * pct / 100;
    if (fw > 0) {
        tft.fillSmoothRoundRect(bx + 2, by + 2, fw, bh - 4, 3, accentColor, wilyConfig.bgColor);
        // Glow at leading edge
        uint16_t glowC = suitePulseColor(accentColor, TFT_WHITE, millis() / 200.0f);
        tft.fillSmoothCircle(bx + 2 + fw, by + bh / 2, 3, glowC, wilyConfig.bgColor);
    }
    // Label
    if (label) {
        tft.setTextSize(FP);
        tft.setTextColor(TFT_WHITE, wilyConfig.bgColor);
        tft.drawCentreString(String(label) + " " + String(pct) + "%", tftWidth / 2, by + bh + 4, 1);
    }
}

// ============================================================================
// 5. SPINNER ANIMATIONS
// ============================================================================

// Animated spinner with rotating dots
inline void suiteAnimSpinnerDots(int cx, int cy, int radius, uint16_t color, int frame) {
    const int numDots = 8;
    for (int i = 0; i < numDots; i++) {
        float angle = ((float)(frame + i) / numDots) * 2.0f * PI;
        int dx = cx + (int)(cosf(angle) * radius);
        int dy = cy + (int)(sinf(angle) * radius);
        float brightness = (float)((numDots - i + frame % numDots) % numDots) / numDots;
        uint16_t c = suiteLerpColor(wilyConfig.bgColor, color, brightness);
        tft.fillSmoothCircle(dx, dy, 2 + (int)(brightness * 2), c, wilyConfig.bgColor);
    }
}

// Animated spinner with wave bars
inline void suiteAnimSpinnerWave(int x, int y, int frame, uint16_t color) {
    const int numBars = 5;
    const int barW = 4;
    const int maxH = 20;
    for (int i = 0; i < numBars; i++) {
        float phase = (float)(frame + i) / numBars;
        int h = (int)(maxH * (0.3f + 0.7f * fabsf(sinf(phase * PI))));
        uint16_t c = suiteLerpColor(color, wilyConfig.bgColor, 1.0f - (float)h / maxH);
        tft.fillRoundRect(x + i * (barW + 2), y + maxH - h, barW, h, 2, c);
    }
}

// Animated spinner with rotating smooth arc
inline void suiteAnimSpinnerArc(int cx, int cy, int r, int frame, uint16_t color) {
    int startAngle = (frame * 30) % 360;
    tft.drawSmoothArc(cx, cy, r + 2, r - 2, startAngle, startAngle + 90, color, wilyConfig.bgColor, true);
    tft.drawSmoothArc(cx, cy, r + 2, r - 2, startAngle + 180, startAngle + 270,
                       getColorVariation(color, 4, -1), wilyConfig.bgColor, true);
}

// Pulsing dot spinner
inline void suiteAnimSpinnerPulse(int cx, int cy, int frame, uint16_t color) {
    float t = (sinf(frame * 0.3f) + 1.0f) / 2.0f;
    int r = 4 + (int)(t * 8);
    tft.fillSmoothCircle(cx, cy, r, suiteLerpColor(wilyConfig.bgColor, color, t), wilyConfig.bgColor);
}

// ============================================================================
// 6. WELCOME SCREEN ANIMATIONS
// ============================================================================

// Procedural icon drawing (no bitmaps needed)
inline void suiteDrawIcon(int cx, int cy, int size, uint16_t color, int iconType) {
    int s = size;
    switch (iconType) {
        case 0: // BLE: Bluetooth B rune
            tft.drawWideLine(cx, cy - s, cx, cy + s, 2, color);
            tft.drawSmoothArc(cx, cy, s, s - 2, 180, 360, color, wilyConfig.bgColor, true);
            tft.drawSmoothArc(cx, cy, s / 2, s / 2 - 2, 0, 180, color, wilyConfig.bgColor, true);
            break;
        case 1: // IR: Signal waves
            for (int i = 1; i <= 3; i++) {
                tft.drawSmoothArc(cx, cy, s * i / 3, s * i / 3 - 2, 225, 315, color, wilyConfig.bgColor, true);
            }
            tft.fillSmoothCircle(cx, cy, 3, color, wilyConfig.bgColor);
            break;
        case 2: // RF: Antenna
            tft.drawWideLine(cx, cy + s, cx, cy - s, 2, color);
            tft.drawWideLine(cx - s / 2, cy - s, cx + s / 2, cy - s, 2, color);
            tft.drawSmoothArc(cx, cy, s / 2, s / 2 - 2, 270, 360, color, wilyConfig.bgColor, true);
            tft.drawSmoothArc(cx, cy, s * 2 / 3, s * 2 / 3 - 2, 270, 360, color, wilyConfig.bgColor, true);
            break;
        case 3: // NFC: Radio waves
            for (int i = 0; i < 3; i++) {
                tft.drawSmoothArc(cx, cy, s - 4 + i * 6, s - 6 + i * 6, 315, 45, color, wilyConfig.bgColor, true);
            }
            break;
        case 4: // BadUSB: USB plug
            tft.drawRoundRect(cx - s / 3, cy - s, s * 2 / 3, s * 1.5f, 3, color);
            tft.fillSmoothCircle(cx, cy - s / 3, 2, color, wilyConfig.bgColor);
            tft.drawWideLine(cx, cy + s / 2, cx, cy + s, 2, color);
            break;
        case 5: // WiFi: Signal arcs
            for (int i = 1; i <= 3; i++) {
                tft.drawSmoothArc(cx, cy, s * i / 3, s * i / 3 - 2, 225, 315, color, wilyConfig.bgColor, true);
            }
            tft.fillSmoothCircle(cx, cy, 3, color, wilyConfig.bgColor);
            break;
        case 6: // SubGHz: Frequency wave
            for (int x = -s; x <= s; x += 2) {
                int py = cy + (int)(sinf((float)x / s * PI * 2) * s / 3);
                tft.fillSmoothCircle(cx + x, py, 1, color, wilyConfig.bgColor);
            }
            break;
        case 7: // RFID: Card with chip
            tft.drawRoundRect(cx - s, cy - s * 2 / 3, s * 2, s * 4 / 3, 3, color);
            tft.fillSmoothRoundRect(cx - s / 3, cy - s / 3, s * 2 / 3, s * 2 / 3, 2, color, wilyConfig.bgColor);
            tft.drawWideLine(cx - s, cy + 2, cx + s, cy + 2, 1, suiteDimColor(color, 0.5f));
            break;
        case 8: // GPS: Compass
            tft.drawSmoothCircle(cx, cy, s, color, wilyConfig.bgColor);
            tft.drawWideLine(cx, cy - s + 4, cx, cy + s - 4, 1, suiteDimColor(color, 0.3f));
            tft.drawWideLine(cx - s + 4, cy, cx + s - 4, cy, 1, suiteDimColor(color, 0.3f));
            tft.drawSmoothArc(cx, cy, s - 2, s - 4, 0, 90, TFT_RED, wilyConfig.bgColor, true);
            tft.drawSmoothArc(cx, cy, s - 2, s - 4, 180, 270, color, wilyConfig.bgColor, true);
            break;
        default:
            tft.fillSmoothCircle(cx, cy, s, color, wilyConfig.bgColor);
            break;
    }
}

// Animated category card with procedural icon
inline void suiteAnimCategoryCard(int y, int h, uint16_t color, int iconType, const char* label, int delayMs = 0) {
    if (delayMs > 0) delay(delayMs);
    int cardX = 8;
    int cardW = tftWidth - 16;
    int iconX = 26;
    int iconY = y + h / 2;
    // Slide in from left
    for (int step = 0; step < 4; step++) {
        float t = (float)(step + 1) / 4.0f;
        int xOff = (int)((1.0f - easeOutCubic(t)) * tftWidth);
        tft.drawSmoothRoundRect(cardX + xOff, y, cardW, h, 6, color, wilyConfig.bgColor);
        suiteDrawIcon(iconX + xOff, iconY, 7, TFT_WHITE, iconType);
        tft.setTextSize(FP);
        tft.setTextColor(TFT_WHITE, wilyConfig.bgColor);
        tft.drawString(label, 42 + xOff, y + 3);
        delay(15);
        if (step < 3) {
            tft.fillRect(cardX + xOff, y, cardW, h, wilyConfig.bgColor);
        }
    }
}

// Full animated welcome screen with procedural icons
inline void suiteAnimWelcome(const char* title, const char* categories[], uint16_t colors[],
                             const int iconTypes[], int count, int startY = 118) {
    tft.fillScreen(wilyConfig.bgColor);
    tft.drawSmoothRoundRect(4, 4, 8, 7, tftWidth - 8, tftHeight - 8, wilyConfig.priColor, wilyConfig.bgColor);

    // Animated title block
    tft.drawSmoothRoundRect(20, 30, 8, 7, tftWidth - 40, 60, wilyConfig.priColor, wilyConfig.bgColor);
    tft.fillSmoothRoundRect(21, 31, tftWidth - 42, 58, 7, getColorVariation(wilyConfig.bgColor, 2, -1), wilyConfig.bgColor);
    tft.setTextSize(4);
    tft.setTextColor(wilyConfig.priColor, getColorVariation(wilyConfig.bgColor, 2, -1));
    tft.setTextDatum(TC_DATUM);
    tft.drawCentreString("WILLY", tftWidth / 2, 36, SMOOTH_FONT);
    tft.setTextSize(2);
    tft.setTextColor(TFT_CYAN, getColorVariation(wilyConfig.bgColor, 2, -1));
    tft.drawCentreString(title, tftWidth / 2, 72, SMOOTH_FONT);
    tft.setTextSize(FP);
    tft.setTextColor(TFT_DARKGREY, wilyConfig.bgColor);
    tft.drawCentreString("55+ Funcoes", tftWidth / 2, 100);
    tft.setTextDatum(TL_DATUM);

    // Animated category cards with icons
    for (int i = 0; i < count; i++) {
        int y = startY + i * 18;
        suiteAnimCategoryCard(y, 16, colors[i], iconTypes[i], categories[i], 30);
    }

    // Animated footer
    tft.fillRect(0, tftHeight - 28, tftWidth, 28, wilyConfig.bgColor);
    tft.drawWideLine(0, tftHeight - 28, tftWidth, tftHeight - 28, 1, TFT_DARKGREY);
    tft.setTextSize(FP);
    tft.setTextColor(TFT_GREEN, wilyConfig.bgColor);
    tft.drawString("SEL: Entrar", 10, tftHeight - 20);
    tft.setTextColor(TFT_YELLOW, wilyConfig.bgColor);
    tft.setTextDatum(TR_DATUM);
    tft.drawString("ESC: Sair", tftWidth - 10, tftHeight - 20);
    tft.setTextDatum(TL_DATUM);
}

// ============================================================================
// 7. MENU LIST ANIMATIONS
// ============================================================================

// Static list item draw (no animation, for quick redraws)
inline void suiteDrawListItem(int y, int h, const char* text, uint16_t color, bool selected = false) {
    tft.fillSmoothRoundRect(8, y, tftWidth - 16, h, 6,
                             selected ? color : getColorVariation(wilyConfig.priColor, 4, -1), wilyConfig.bgColor);
    tft.setTextSize(FP);
    tft.setTextColor(selected ? TFT_WHITE : color, wilyConfig.bgColor);
    tft.drawString(text, 14, y + 4);
}

// Animated list item entrance (stagger from top)
inline void suiteAnimListItem(int y, int h, const char* text, uint16_t color, int index, int total) {
    int delayMs = 20 + index * 15;
    delay(delayMs);
    for (int step = 0; step < 3; step++) {
        float t = (float)(step + 1) / 3.0f;
        int xOff = (int)((1.0f - easeOutCubic(t)) * 20);
        tft.fillRect(6, y, tftWidth - 12, h, wilyConfig.bgColor);
        tft.drawSmoothRoundRect(8 + xOff, y, 6, 5, tftWidth - 16, h, TFT_DARKGREY, wilyConfig.bgColor);
        tft.setTextSize(FP);
        tft.setTextColor(color, wilyConfig.bgColor);
        tft.drawString(text, 14 + xOff, y + 4);
        delay(10);
    }
}

// ============================================================================
// 8. CONFIRMATION DIALOG ANIMATIONS
// ============================================================================

// Animated confirmation dialog
inline bool suiteAnimConfirm(const char* msg, uint16_t titleColor = TFT_YELLOW) {
    tft.fillScreen(wilyConfig.bgColor);
    // Slide-in smooth border
    for (int i = 0; i < 6; i++) {
        float t = (float)(i + 1) / 6.0f;
        int r = (int)(easeOutCubic(t) * 8);
        tft.drawSmoothRoundRect(4, 4, r, r - 1, tftWidth - 8, tftHeight - 8, titleColor, wilyConfig.bgColor);
        delay(15);
    }
    // Header
    tft.fillSmoothRoundRect(10, 8, tftWidth - 20, 28, 6, titleColor, wilyConfig.bgColor);
    tft.setTextDatum(TC_DATUM); tft.setTextSize(FM);
    tft.setTextColor(TFT_BLACK, titleColor);
    tft.drawCentreString("CONFIRMACAO", tftWidth / 2, 12, SMOOTH_FONT);
    tft.setTextDatum(TL_DATUM);
    tft.setTextSize(FP);
    // Message
    tft.setTextColor(TFT_WHITE, wilyConfig.bgColor);
    tft.drawCentreString(msg, tftWidth / 2, 55, 1);
    tft.drawWideLine(20, 75, tftWidth - 20, 75, 1, TFT_DARKGREY);
    // Two cards
    int sel = 0, last = -1;
    while (true) {
        if (sel != last) {
            int oy = 85;
            // SIM card
            bool simSel = (sel == 0);
            tft.fillSmoothRoundRect(20, oy, tftWidth / 2 - 25, 22, 6,
                                     simSel ? TFT_GREEN : getColorVariation(wilyConfig.priColor, 4, -1), wilyConfig.bgColor);
            tft.drawSmoothRoundRect(20, oy, 6, 5, tftWidth / 2 - 25, 22,
                                     simSel ? TFT_GREEN : TFT_DARKGREY, wilyConfig.bgColor);
            tft.setTextSize(FP);
            tft.setTextColor(simSel ? TFT_BLACK : TFT_GREEN,
                             simSel ? TFT_GREEN : getColorVariation(wilyConfig.priColor, 4, -1));
            tft.drawCentreString("SIM", 20 + (tftWidth / 2 - 25) / 2, oy + 5, 1);
            // NAO card
            bool naoSel = (sel == 1);
            tft.fillSmoothRoundRect(tftWidth / 2 + 5, oy, tftWidth / 2 - 25, 22, 6,
                                     naoSel ? TFT_RED : getColorVariation(wilyConfig.priColor, 4, -1), wilyConfig.bgColor);
            tft.drawSmoothRoundRect(tftWidth / 2 + 5, oy, 6, 5, tftWidth / 2 - 25, 22,
                                     naoSel ? TFT_RED : TFT_DARKGREY, wilyConfig.bgColor);
            tft.setTextColor(naoSel ? TFT_BLACK : TFT_RED,
                             naoSel ? TFT_RED : getColorVariation(wilyConfig.priColor, 4, -1));
            tft.drawCentreString("NAO", tftWidth / 2 + 5 + (tftWidth / 2 - 25) / 2, oy + 5, 1);
            last = sel;
        }
        if (check(SelPress)) return sel == 0;
        if (check(EscPress)) return false;
        if (check(UpPress) || check(LeftPress)) sel = 0;
        if (check(DownPress) || check(RightPress)) sel = 1;
        delay(50);
    }
}

// ============================================================================
// 9. FOOTER ANIMATIONS
// ============================================================================

// Static footer (no animation, for quick redraws)
inline void suiteDrawFooter(const char* left = nullptr, const char* right = nullptr) {
    tft.fillRect(0, tftHeight - 28, tftWidth, 28, wilyConfig.bgColor);
    tft.drawWideLine(0, tftHeight - 28, tftWidth, tftHeight - 28, 1, TFT_DARKGREY);
    tft.setTextSize(FP);
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(TFT_GREEN, wilyConfig.bgColor);
    if (left) tft.drawString(left, 10, tftHeight - 20);
    if (right) { tft.setTextDatum(TR_DATUM); tft.setTextColor(TFT_YELLOW, wilyConfig.bgColor); tft.drawString(right, tftWidth - 10, tftHeight - 20); }
    tft.setTextDatum(TL_DATUM);
}

// Animated footer with typing effect
inline void suiteAnimFooter(const char* left, const char* right, int durationMs = 100) {
    tft.fillRect(0, tftHeight - 28, tftWidth, 28, wilyConfig.bgColor);
    tft.drawWideLine(0, tftHeight - 28, tftWidth, tftHeight - 28, 1, TFT_DARKGREY);
    tft.setTextSize(FP);
    tft.setTextColor(TFT_GREEN, wilyConfig.bgColor);
    if (left) {
        int len = strlen(left);
        for (int i = 0; i <= len; i++) {
            tft.fillRect(10, tftHeight - 20, tftWidth / 2 - 20, 12, wilyConfig.bgColor);
            tft.drawString(String(left).substring(0, i), 10, tftHeight - 20);
            delay(durationMs / max(len, 1));
        }
    }
    tft.setTextColor(TFT_YELLOW, wilyConfig.bgColor);
    tft.setTextDatum(TR_DATUM);
    if (right) {
        int len = strlen(right);
        for (int i = 0; i <= len; i++) {
            tft.fillRect(tftWidth / 2, tftHeight - 20, tftWidth / 2 - 10, 12, wilyConfig.bgColor);
            tft.drawString(String(right).substring(0, i), tftWidth - 10, tftHeight - 20);
            delay(durationMs / max(len, 1));
        }
    }
    tft.setTextDatum(TL_DATUM);
}

// ============================================================================
// 10. UTILITY: WAIT FOR INPUT WITH SPINNER
// ============================================================================

// Show spinner while waiting, check for ESC to cancel
inline bool suiteWaitWithSpinner(const char* msg, uint16_t color = TFT_CYAN, int maxFrames = 60) {
    suiteDrawHeader(msg, color);
    int cx = tftWidth / 2;
    int cy = tftHeight / 2;
    for (int frame = 0; frame < maxFrames; frame++) {
        suiteAnimSpinnerArc(cx, cy, 20, frame, color);
        tft.setTextSize(FP);
        tft.setTextColor(TFT_WHITE, wilyConfig.bgColor);
        tft.drawCentreString("Pressione ESC para cancelar...", cx, cy + 30, 1);
        if (check(EscPress)) return true;
        delay(50);
    }
    return false;
}

#endif // __SUITE_VISUALS_H__
