#pragma once
#include "../../Game/PlayerList.h"
#include <imgui.h>

namespace ARC
{
    class Radar
    {
    public:
        static void Render();
        static void RenderSettings();

        // Settings
        static inline bool m_Enabled = true;
        static inline float m_Scale = 150.0f; // Meters shown on radar
        static inline float m_Size = 300.0f;  // Radar size in pixels
        static inline ImVec2 m_Position = ImVec2(50, 50);
        static inline float m_Zoom = 1.0f;
        static inline bool m_ShowGrid = true;
        static inline bool m_ShowDistance = true;

    private:
        static void DrawPlayer(ImDrawList* draw, const PlayerData& player, const UnrealEngine::FVector& localPos);
        static void DrawGrid(ImDrawList* draw, const ImVec2& center, float radarSize);
    };
}
