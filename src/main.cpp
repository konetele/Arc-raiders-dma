#include "Core/Memory.h"
#include "Game/Camera.h"
#include "Game/PlayerList.h"
#include "GUI/Overlay.h"
#include "GUI/ESP/ESP.h"
#include "GUI/Radar/Radar.h"

#include <thread>
#include <atomic>
#include <chrono>
#include <iostream>

std::atomic<bool> g_Running{true};

// DMA Thread - reads game memory
void DMAThread()
{
    std::cout << "[*] DMA Thread started" << std::endl;

    while (g_Running)
    {
        // Update game data
        ARC::Camera::Update();
        ARC::PlayerList::Update();

        // Sleep to reduce CPU usage (20 FPS update rate)
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    std::cout << "[*] DMA Thread stopped" << std::endl;
}

// Main function
int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "  Arc Raiders DMA - ESP & Radar" << std::endl;
    std::cout << "========================================" << std::endl;

    // Initialize Memory (MemProcFS)
    if (!ARC::Memory::Initialize())
    {
        std::cout << "[!] Failed to initialize DMA" << std::endl;
        std::cout << "Press Enter to exit..." << std::endl;
        std::cin.get();
        return 1;
    }

    // Initialize Overlay Window
    if (!ARC::Overlay::Initialize())
    {
        std::cout << "[!] Failed to initialize overlay" << std::endl;
        ARC::Memory::Shutdown();
        std::cout << "Press Enter to exit..." << std::endl;
        std::cin.get();
        return 1;
    }

    std::cout << "\n[+] All systems initialized" << std::endl;
    std::cout << "[+] Starting DMA thread..." << std::endl;
    std::cout << "\nControls:" << std::endl;
    std::cout << "  - INSERT: Toggle Menu" << std::endl;
    std::cout << "  - END: Exit" << std::endl;
    std::cout << "\n========================================\n" << std::endl;

    // Start DMA thread
    std::thread dmaThread(DMAThread);

// Main loop (GUI Thread)
    while (ARC::Overlay::IsRunning() && g_Running)
    {
        // Begin frame
        ARC::Overlay::BeginFrame();

        if (GetAsyncKeyState(VK_END) & 1)
            g_Running = false;

        // --- ИСПРАВЛЕННАЯ ЛОГИКА INSERT ---
        static bool menuOpen = false;  
        static bool insertPressed = false;
        
        if (GetAsyncKeyState(VK_INSERT) & 0x8000)
        {
            if (!insertPressed)
            {
                insertPressed = true;
                menuOpen = !menuOpen; // Переключаем состояние
                
                // Передаем в функцию само состояние:
                // true = Меню открыто (можно кликать)
                // false = Меню закрыто (сквозные клики)
                ARC::Overlay::SetOverlayVisible(menuOpen); 
            }
        }
        else
        {
            insertPressed = false;
        }
        // -----------------------------------

        // Always render overlay


        // Always render overlay (but it's transparent)
        {
            // Full screen overlay window (use dynamic resolution)
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2((float)ARC::Overlay::m_ScreenWidth, (float)ARC::Overlay::m_ScreenHeight));
            ImGui::Begin("Overlay", nullptr,
                        ImGuiWindowFlags_NoTitleBar |
                        ImGuiWindowFlags_NoResize |
                        ImGuiWindowFlags_NoMove |
                        ImGuiWindowFlags_NoScrollbar |
                        ImGuiWindowFlags_NoScrollWithMouse |
                        ImGuiWindowFlags_NoCollapse |
                        ImGuiWindowFlags_NoBackground |
                        ImGuiWindowFlags_NoBringToFrontOnFocus);

            auto drawList = ImGui::GetWindowDrawList();

            // Render ESP
            ARC::ESP::Render(drawList);

            ImGui::End();

            // Render Radar (separate window)
            ARC::Radar::Render();
        }

        // Render Settings Menu - only when open
        if (menuOpen)
        {
            ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_FirstUseEver);

            ImGui::Begin("Arc Raiders - Settings", &menuOpen);

            ImGui::Text("Arc Raiders ESP & Radar");
            ImGui::Separator();

            // ESP Settings
            ARC::ESP::RenderSettings();
            ImGui::Spacing();

            // Radar Settings
            ARC::Radar::RenderSettings();
            ImGui::Spacing();

            // Display Settings
            ARC::Overlay::RenderDisplaySettings();
            ImGui::Spacing();
            ImGui::Separator();

            // Stats
            if (ImGui::CollapsingHeader("Statistics"))
            {
                auto players = ARC::PlayerList::GetPlayers();
                ImGui::Text("Players in list: %d", (int)players.size());
                ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
            }

            ImGui::End();

        }

        // End frame
        ARC::Overlay::EndFrame();

        // Limit to 60 FPS
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    std::cout << "\n[*] Shutting down..." << std::endl;

    // Wait for DMA thread to finish
    g_Running = false;
    dmaThread.join();

    // Cleanup
    ARC::Overlay::Shutdown();
    ARC::Memory::Shutdown();

    std::cout << "[+] Shutdown complete" << std::endl;

    return 0;
}
