# Arc Raiders DMA - External Cheat

![Status](https://img.shields.io/badge/Status-Active-green) ![Platform](https://img.shields.io/badge/Platform-Windows%20x64-blue) ![DMA](https://img.shields.io/badge/Hardware-DMA-orange)

An external, hardware-based (DMA) ESP and Radar for Arc Raiders. Built with C++, MemProcFS, and DirectX 11. Designed for thread safety and performance.

## ⚠️ Disclaimer
**Educational purposes only.** Use this software at your own risk. The author is not responsible for any bans or damages.

## ✨ Features

### Visuals (ESP)
* **Player ESP:** 2D Bounding Boxes, Name tags, Health bars (gradient).
* **Filtering:** Configurable max distance for Players and AI.
* **Performance:** ~2ms render time with thread-safe data caching.

### 2D Radar
* **Tactical Map:** Top-down view with local player centering.
* **Indicators:** North compass, grid overlay, off-screen arrows.
* **Customization:** Zoom (0.5x-3.0x) and Range (50m-500m) controls.

### System
* **Overlay:** transparent DX11 window.
* **Architecture:** Multi-threaded (Separate DMA Read and GUI Render threads).

## 🎮 Controls
* `INSERT`: Toggle Menu
* `END`: Panic / Exit

## 🚀 Performance
* **DMA Thread:** ~50ms (20 ticks/sec)
* **Render Thread:** ~16ms (60 FPS capped)
* **Memory:** ~3MB RAM usage