# 🛠️ Setup & Build Guide

## Prerequisites
* Visual Studio 2022 (C++ Desktop Development)
* DMA Hardware (PCILeech compatible card)
* **MemProcFS** (v5.0+)
* **ImGui** (Docking branch recommended)

## 1. Project Structure
Ensure your directory looks exactly like this before building:

```text
[Root Directory]
├── src/
│   ├── Core/           # Memory & Types
│   ├── Game/           # Game Logic (Camera, Players)
│   ├── GUI/            # Overlay, ESP, Radar
│   └── Dependencies/   # [ACTION REQUIRED]
│       ├── MemProcFS/
│       │   ├── lib/
│       │   │   └── vmm.lib       <-- From MemProcFS release (x64)
│       │   ├── vmmdll.h          <-- From MemProcFS includes
│       │   ├── leechcore.h
│       │   └── ... (all .h files)
│       └── ImGui/
│           ├── backends/
│           │   ├── imgui_impl_dx11.h/cpp
│           │   └── imgui_impl_win32.h/cpp
│           ├── imgui.h
│           └── ... (all core ImGui files)