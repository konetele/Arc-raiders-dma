#include "Overlay.h"
#include <iostream>

namespace ARC
{
    bool Overlay::Initialize()
    {
        if (!CreateOverlayWindow())
        {
            std::cout << "[!] Failed to create overlay window" << std::endl;
            return false;
        }

        if (!CreateD3D11Device())
        {
            std::cout << "[!] Failed to create D3D11 device" << std::endl;
            return false;
        }

        // Setup ImGui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        // Setup style
        ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        ImGui_ImplWin32_Init(m_hWnd);
        ImGui_ImplDX11_Init(m_pDevice, m_pDeviceContext);

        std::cout << "[+] Overlay initialized (" << m_ScreenWidth << "x" << m_ScreenHeight << ")" << std::endl;
        return true;
    }

    void Overlay::Shutdown()
    {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        CleanupD3D11();

        if (m_hWnd)
        {
            DestroyWindow(m_hWnd);
            UnregisterClass(L"ArcRaidersOverlay", GetModuleHandle(nullptr));
        }
    }

    void Overlay::SetOverlayVisible(bool isMenuOpen)
    {
        if (!m_hWnd) return;

        m_MenuOpen = isMenuOpen;
        LONG exStyle = GetWindowLong(m_hWnd, GWL_EXSTYLE);

        if (isMenuOpen)
        {
            // МЕНЮ ОТКРЫТО
            exStyle &= ~WS_EX_TRANSPARENT;
            SetWindowLong(m_hWnd, GWL_EXSTYLE, exStyle);
            
            // ИЗМЕНЕНИЕ: HWND_NOTOPMOST вместо HWND_TOPMOST
            // Окно можно перекрыть другими окнами
            SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

            std::cout << "[DEBUG] Overlay: MENU MODE (Interactive, Not Topmost)" << std::endl;
        }
        else
        {
            // МЕНЮ ЗАКРЫТО (Только ESP)
            exStyle |= WS_EX_TRANSPARENT;
            SetWindowLong(m_hWnd, GWL_EXSTYLE, exStyle);
            
            // ИЗМЕНЕНИЕ: HWND_NOTOPMOST вместо HWND_TOPMOST
            SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOACTIVATE);

            std::cout << "[DEBUG] Overlay: ESP MODE (Click-through, Not Topmost)" << std::endl;
        }
    }

    void Overlay::BeginFrame()
    {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
                m_Running = false;
        }

        if (!m_Running) return;

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }

    void Overlay::EndFrame()
    {
        if (!m_Running) return;

        ImGui::Render();

        const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, nullptr);
        m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, clearColor);

        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        m_pSwapChain->Present(1, 0);
    }

    bool Overlay::CreateOverlayWindow()
    {
        WNDCLASSEX wc = {};
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = WndProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = L"ArcRaidersOverlay";

        RegisterClassEx(&wc);

        m_ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
        m_ScreenHeight = GetSystemMetrics(SM_CYSCREEN);

        // ИЗМЕНЕНИЕ: Убрали WS_EX_TOPMOST
        // Теперь окно создается на обычном уровне, а не поверх всех
        m_hWnd = CreateWindowEx(
            WS_EX_LAYERED, // Был WS_EX_TOPMOST | WS_EX_LAYERED
            L"ArcRaidersOverlay",
            L"Arc Raiders ESP",
            WS_POPUP,
            m_MonitorX, m_MonitorY,
            m_ScreenWidth, m_ScreenHeight,
            nullptr,
            nullptr,
            wc.hInstance,
            nullptr
        );

        if (!m_hWnd) return false;

        MARGINS margins = { -1, -1, -1, -1 };
        DwmExtendFrameIntoClientArea(m_hWnd, &margins);

        ShowWindow(m_hWnd, SW_SHOW);
        UpdateWindow(m_hWnd);

        return true;
    }

    bool Overlay::CreateD3D11Device()
    {
        DXGI_SWAP_CHAIN_DESC sd = {};
        sd.BufferCount = 2;
        sd.BufferDesc.Width = m_ScreenWidth;
        sd.BufferDesc.Height = m_ScreenHeight;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = m_hWnd;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

        D3D_FEATURE_LEVEL featureLevel;
        const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };

        HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &m_pSwapChain, &m_pDevice, &featureLevel, &m_pDeviceContext);
        
        if (FAILED(hr)) return false;

        ID3D11Texture2D* pBackBuffer;
        m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        m_pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_pRenderTargetView);
        pBackBuffer->Release();

        return true;
    }

    void Overlay::CleanupD3D11()
    {
        if (m_pRenderTargetView) { m_pRenderTargetView->Release(); m_pRenderTargetView = nullptr; }
        if (m_pSwapChain) { m_pSwapChain->Release(); m_pSwapChain = nullptr; }
        if (m_pDeviceContext) { m_pDeviceContext->Release(); m_pDeviceContext = nullptr; }
        if (m_pDevice) { m_pDevice->Release(); m_pDevice = nullptr; }
    }

    LRESULT CALLBACK Overlay::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
            return true;

        switch (msg)
        {
        case WM_NCHITTEST:
        {
            if (m_MenuOpen) return HTCLIENT; 
            return HTTRANSPARENT;
        }

        case WM_SIZE:
            if (m_pDevice != nullptr && wParam != SIZE_MINIMIZED)
            {
                if (m_pRenderTargetView) { m_pRenderTargetView->Release(); m_pRenderTargetView = nullptr; }
                m_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
                ID3D11Texture2D* pBackBuffer;
                m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
                m_pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_pRenderTargetView);
                pBackBuffer->Release();
            }
            return 0;

        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU) return 0;
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }

        return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    void Overlay::UpdateWindowPosition()
    {
        if (!m_hWnd) return;
        // ИЗМЕНЕНИЕ: HWND_NOTOPMOST
        SetWindowPos(m_hWnd, HWND_NOTOPMOST, m_MonitorX, m_MonitorY, m_ScreenWidth, m_ScreenHeight, SWP_SHOWWINDOW);
        std::cout << "[*] Window updated." << std::endl;
    }

    void Overlay::RenderDisplaySettings()
    {
        if (ImGui::CollapsingHeader("Display Settings"))
        {
            bool changed = false;
            const char* resolutions[] = { "1920x1080", "2560x1440", "3440x1440 (Ultrawide)", "3840x2160 (4K)", "Custom" };
            static int currentRes = 2;

            if (ImGui::Combo("Resolution", &currentRes, resolutions, IM_ARRAYSIZE(resolutions))) {
                switch (currentRes) {
                case 0: m_ScreenWidth = 1920; m_ScreenHeight = 1080; changed = true; break;
                case 1: m_ScreenWidth = 2560; m_ScreenHeight = 1440; changed = true; break;
                case 2: m_ScreenWidth = 3440; m_ScreenHeight = 1440; changed = true; break;
                case 3: m_ScreenWidth = 3840; m_ScreenHeight = 2160; changed = true; break;
                }
            }
            if (currentRes == 4) {
                if (ImGui::InputInt("Width", &m_ScreenWidth)) changed = true;
                if (ImGui::InputInt("Height", &m_ScreenHeight)) changed = true;
            }
            ImGui::Spacing();
            if (ImGui::Button("Apply Changes") || changed) UpdateWindowPosition();
        }
    }
}