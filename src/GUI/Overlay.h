#pragma once
#include <d3d11.h>
#include <dwmapi.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

#pragma comment(lib, "dwmapi.lib")

namespace ARC
{
    class Overlay
    {
    public:
        static bool Initialize();
        static void Shutdown();
        static void BeginFrame();
        static void EndFrame();
        static bool IsRunning() { return m_Running; }
        static HWND GetWindowHandle() { return m_hWnd; }
        static void UpdateWindowPosition();
        static void RenderDisplaySettings();
        
        // Обновленная сигнатура функции
        static void SetOverlayVisible(bool isMenuOpen);

        // Display settings (public for main.cpp access)
        static inline int m_ScreenWidth = 3440;
        static inline int m_ScreenHeight = 1440;
        static inline int m_MonitorX = 0;
        static inline int m_MonitorY = 0;
        static inline int m_CurrentMonitor = 0;

        // Новая переменная состояния меню
        static inline bool m_MenuOpen = false;

    private:
        static bool CreateOverlayWindow();
        static bool CreateD3D11Device();
        static void CleanupD3D11();

        static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    private:
        static inline HWND m_hWnd = nullptr;
        static inline ID3D11Device* m_pDevice = nullptr;
        static inline ID3D11DeviceContext* m_pDeviceContext = nullptr;
        static inline IDXGISwapChain* m_pSwapChain = nullptr;
        static inline ID3D11RenderTargetView* m_pRenderTargetView = nullptr;
        static inline bool m_Running = true;
    };
}

// Forward declare
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);