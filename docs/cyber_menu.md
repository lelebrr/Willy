# Cyber Menu UI (Willy v2.1 Evolution)

The Willy Cyber Menu is an advanced, high-performance UI designed for embedded displays (like the CYD 2.8"). It integrates advanced LVGL animations and real-time system monitoring.

## Features

### 1. Advanced Animations

- **Cascade Entry**: Icons fade and slide up with an incremental delay, creating a wave effect.
- **Neon Pulsing**: Button borders subtly pulse with transparency easing.
- **Interactive Hover**:
  - **Zoom**: Smooth 1.3x magnification.
  - **3D Rotation**: 8-degree tilt for depth perception.
  - **Glow/Shadow**: Accent-colored shadow spread.

### 2. Integration & Themes

- **Dynamic Themes**: Uses `WillyConfig.priColor` and `WillyConfig.secColor` to match the system's current theme (Dark, Light, or Cyber).
- **Core Integration**: Seamlessly calls Willy's existing module callbacks (Wi-Fi, BLE, IR, etc.).

### 3. Notification Bar

- **Real-time Status**:
  - **Time**: Syncs with system time (NTP supported).
  - **Battery**: Real-time percentage with color-coded alerts (Green > 50%, Yellow > 20%, Red < 20%).
- **Interactive Symbols**: Visual indicators for WiFi and Bluetooth connectivity.

## Technical Details

- **Extra Footprint**: < 10KB.
- **Engine**: LVGL 8.3.11.
- **Optimization**: Function reuse and minimal vector path points to prevent CPU lag on ESP32.

## How to Access

Go to the **Main Menu** and select **Cyber Menu (Exp)**.
