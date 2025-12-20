#pragma once
#include "../../Game/PlayerList.h"
#include <imgui.h>

namespace ARC
{
    class ESP
    {
    public:
        static void Render(ImDrawList* drawList);
        static void RenderSettings();

        // Settings
        static inline bool m_Enabled = true;
        static inline bool m_DrawBox = true;
        static inline bool m_DrawName = true;
        static inline bool m_DrawHealth = true;
        static inline bool m_DrawDistance = true;
        static inline bool m_DrawSkeleton = false; // For when bones work
        static inline float m_MaxDistance = 350.0f;
        static inline float m_MaxDistanceAI = 100.0f;
        static inline float m_BoxThickness = 2.0f;

        // Colors
        static inline ImVec4 m_ColorEnemy = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
        static inline ImVec4 m_ColorTeam = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);
        static inline ImVec4 m_ColorAI = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);

    private:
        static void DrawPlayer(ImDrawList* drawList, const PlayerData& player);
        static void DrawBox2D(ImDrawList* drawList, const PlayerData& player);
        static void DrawHealthBar(ImDrawList* drawList, const ImVec2& topLeft, float width, float height, float healthPct);
        static void DrawName(ImDrawList* drawList, const PlayerData& player, const ImVec2& screenPos);
        static void DrawDistance(ImDrawList* drawList, const PlayerData& player, const ImVec2& screenPos);
    };
}
