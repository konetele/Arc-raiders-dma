#include "Radar.h"
#include "../../Game/Camera.h"
#include <algorithm>

namespace ARC
{
    void Radar::Render()
    {
        if (!m_Enabled) return;

        auto players = PlayerList::GetPlayers();

        // Find local player
        auto localIt = std::find_if(players.begin(), players.end(),
            [](const PlayerData& p) { return p.isLocalPlayer; });

        if (localIt == players.end()) return;

        const PlayerData& localPlayer = *localIt;

        ImGui::SetNextWindowPos(m_Position, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(m_Size, m_Size), ImGuiCond_FirstUseEver);

        ImGui::Begin("Radar", &m_Enabled, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);

        auto drawList = ImGui::GetWindowDrawList();
        auto windowPos = ImGui::GetWindowPos();
        auto windowSize = ImGui::GetWindowSize();
        auto center = ImVec2(windowPos.x + windowSize.x / 2, windowPos.y + windowSize.y / 2);

        // Background
        drawList->AddRectFilled(windowPos,
                               ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y),
                               IM_COL32(20, 20, 20, 240));

        // Border
        drawList->AddRect(windowPos,
                         ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y),
                         IM_COL32(255, 255, 255, 100),
                         0.0f, 0, 2.0f);

        // Draw grid
        if (m_ShowGrid)
        {
            DrawGrid(drawList, center, windowSize.x);
        }

        // Draw compass lines
        drawList->AddLine(ImVec2(center.x, windowPos.y + 10), ImVec2(center.x, center.y - 20), IM_COL32(255, 255, 255, 100), 1.0f);
        drawList->AddLine(ImVec2(windowPos.x + 10, center.y), ImVec2(center.x - 20, center.y), IM_COL32(255, 255, 255, 100), 1.0f);
        drawList->AddLine(ImVec2(center.x, windowPos.y + windowSize.y - 10), ImVec2(center.x, center.y + 20), IM_COL32(255, 255, 255, 100), 1.0f);
        drawList->AddLine(ImVec2(windowPos.x + windowSize.x - 10, center.y), ImVec2(center.x + 20, center.y), IM_COL32(255, 255, 255, 100), 1.0f);

        // Draw cardinal directions
        drawList->AddText(ImVec2(center.x - 5, windowPos.y + 15), IM_COL32(255, 255, 255, 255), "N");
        drawList->AddText(ImVec2(windowPos.x + 15, center.y - 10), IM_COL32(255, 255, 255, 255), "W");
        drawList->AddText(ImVec2(center.x - 5, windowPos.y + windowSize.y - 30), IM_COL32(255, 255, 255, 255), "S");
        drawList->AddText(ImVec2(windowPos.x + windowSize.x - 30, center.y - 10), IM_COL32(255, 255, 255, 255), "E");

        // Local player (center)
        drawList->AddCircleFilled(center, 6, IM_COL32(0, 255, 0, 255));
        drawList->AddCircle(center, 6, IM_COL32(255, 255, 255, 255), 0, 2.0f);

        // Draw other players
        for (const auto& player : players)
        {
            if (player.isLocalPlayer) continue;

            DrawPlayer(drawList, player, localPlayer.position);
        }

        // Scale info
        char scaleText[64];
        snprintf(scaleText, sizeof(scaleText), "Scale: %.0fm", m_Scale * m_Zoom);
        drawList->AddText(ImVec2(windowPos.x + 10, windowPos.y + windowSize.y - 25), IM_COL32(255, 255, 255, 200), scaleText);

        ImGui::End();
    }

    void Radar::DrawGrid(ImDrawList* draw, const ImVec2& center, float radarSize)
    {
        int gridLines = 4;
        float cellSize = radarSize / (gridLines * 2);

        for (int i = -gridLines; i <= gridLines; i++)
        {
            float offset = cellSize * i;

            // Vertical lines
            draw->AddLine(ImVec2(center.x + offset, center.y - radarSize / 2),
                         ImVec2(center.x + offset, center.y + radarSize / 2),
                         IM_COL32(50, 50, 50, 100), 1.0f);

            // Horizontal lines
            draw->AddLine(ImVec2(center.x - radarSize / 2, center.y + offset),
                         ImVec2(center.x + radarSize / 2, center.y + offset),
                         IM_COL32(50, 50, 50, 100), 1.0f);
        }
    }

    void Radar::DrawPlayer(ImDrawList* draw, const PlayerData& player, const UnrealEngine::FVector& localPos)
    {
        // Calculate relative position
        auto delta = player.position - localPos;

        // Convert to radar coordinates (scaled)
        float scale = (m_Scale * 100.0f) * m_Zoom; // Convert meters to cm
        float radarX = (delta.X / scale) * (m_Size / 2);
        float radarY = (delta.Y / scale) * (m_Size / 2);

        auto windowPos = ImGui::GetWindowPos();
        auto center = ImVec2(windowPos.x + m_Size / 2, windowPos.y + m_Size / 2);

        ImVec2 screenPos = ImVec2(center.x + radarX, center.y - radarY); // Y inverted

        // Clamp to radar bounds
        if (screenPos.x < windowPos.x || screenPos.x > windowPos.x + m_Size ||
            screenPos.y < windowPos.y || screenPos.y > windowPos.y + m_Size)
        {
            // Draw arrow at edge pointing to player
            float angle = atan2f(-radarY, radarX);
            float edgeX = center.x + cosf(angle) * (m_Size / 2 - 15);
            float edgeY = center.y - sinf(angle) * (m_Size / 2 - 15);

            ImVec2 arrowPos(edgeX, edgeY);

            bool isAI = (player.entityType == (unsigned int)UnrealEngine::EActorType::EACTOR_ARC_WASP);
            ImU32 color = isAI ? IM_COL32(255, 255, 0, 255) : IM_COL32(255, 0, 0, 255);

            draw->AddCircleFilled(arrowPos, 4, color);
            return;
        }

        // Determine color
        bool isAI = (player.entityType == (unsigned int)UnrealEngine::EActorType::EACTOR_ARC_WASP);
        ImU32 color = isAI ? IM_COL32(255, 255, 0, 255) : IM_COL32(255, 0, 0, 255);

        // Draw player dot
        draw->AddCircleFilled(screenPos, 5, color);
        draw->AddCircle(screenPos, 5, IM_COL32(255, 255, 255, 150), 0, 1.5f);

        // Draw distance text
        if (m_ShowDistance)
        {
            char distText[32];
            snprintf(distText, sizeof(distText), "%.0fm", player.distance);

            ImVec2 textSize = ImGui::CalcTextSize(distText);
            ImVec2 textPos(screenPos.x - textSize.x / 2, screenPos.y + 8);

            draw->AddText(textPos, IM_COL32(255, 255, 255, 200), distText);
        }
    }

    void Radar::RenderSettings()
    {
        if (ImGui::CollapsingHeader("Radar Settings"))
        {
            ImGui::Checkbox("Enabled", &m_Enabled);
            ImGui::Checkbox("Show Grid", &m_ShowGrid);
            ImGui::Checkbox("Show Distance", &m_ShowDistance);
            ImGui::SliderFloat("Scale (meters)", &m_Scale, 50.0f, 500.0f);
            ImGui::SliderFloat("Size (pixels)", &m_Size, 200.0f, 600.0f);
            ImGui::SliderFloat("Zoom", &m_Zoom, 0.5f, 3.0f);
        }
    }
}
