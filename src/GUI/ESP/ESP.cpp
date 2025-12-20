#include "ESP.h"
#include "../../Game/Camera.h"
#include <string>

namespace ARC
{
    void ESP::Render(ImDrawList* drawList)
    {
        if (!m_Enabled) return;

        auto players = PlayerList::GetPlayers();

        for (const auto& player : players)
        {
            if (player.isLocalPlayer) continue;

            // Distance check
            bool isAI = (player.entityType == (unsigned int)UnrealEngine::EActorType::EACTOR_ARC_WASP);
            float maxDist = isAI ? m_MaxDistanceAI : m_MaxDistance;

            if (player.distance > maxDist) continue;

            DrawPlayer(drawList, player);
        }
    }

    void ESP::DrawPlayer(ImDrawList* drawList, const PlayerData& player)
    {
        UnrealEngine::FVector2D screenPos;
        if (!Camera::WorldToScreen(player.position, screenPos))
            return;

        // Draw box
        if (m_DrawBox)
        {
            DrawBox2D(drawList, player);
        }

        // Draw name
        if (m_DrawName)
        {
            DrawName(drawList, player, ImVec2(screenPos.X, screenPos.Y));
        }

        // Draw distance
        if (m_DrawDistance)
        {
            DrawDistance(drawList, player, ImVec2(screenPos.X, screenPos.Y));
        }
    }

    void ESP::DrawBox2D(ImDrawList* drawList, const PlayerData& player)
    {
        // Create simple bounding box
        UnrealEngine::FVector headPos = player.position;
        headPos.Z += 100.0; // Approximate head height

        UnrealEngine::FVector feetPos = player.position;
        feetPos.Z -= 100.0;

        UnrealEngine::FVector2D headScreen, feetScreen;
        if (!Camera::WorldToScreen(headPos, headScreen)) return;
        if (!Camera::WorldToScreen(feetPos, feetScreen)) return;

        float height = feetScreen.Y - headScreen.Y;
        float width = height * 0.4f;

        ImVec2 topLeft(headScreen.X - width / 2, headScreen.Y);
        ImVec2 bottomRight(headScreen.X + width / 2, feetScreen.Y);

        // Choose color
        ImU32 color;
        bool isAI = (player.entityType == (unsigned int)UnrealEngine::EActorType::EACTOR_ARC_WASP);

        if (isAI)
            color = ImGui::ColorConvertFloat4ToU32(m_ColorAI);
        else
            color = ImGui::ColorConvertFloat4ToU32(m_ColorEnemy);

        // Draw box
        drawList->AddRect(topLeft, bottomRight, color, 0.0f, 0, m_BoxThickness);

        // Draw health bar
        if (m_DrawHealth && player.maxHealth > 0)
        {
            float healthPct = player.health / player.maxHealth;
            DrawHealthBar(drawList, topLeft, width, height, healthPct);
        }
    }

    void ESP::DrawHealthBar(ImDrawList* drawList, const ImVec2& topLeft, float width, float height, float healthPct)
    {
        float barWidth = 4.0f;
        float barHeight = height;

        ImVec2 barTopLeft(topLeft.x - barWidth - 4, topLeft.y);
        ImVec2 barBottomRight(topLeft.x - 4, topLeft.y + barHeight);

        // Background
        drawList->AddRectFilled(barTopLeft, barBottomRight, IM_COL32(0, 0, 0, 200));

        // Health
        float healthHeight = barHeight * healthPct;
        ImVec2 healthBottomRight(topLeft.x - 4, topLeft.y + barHeight);
        ImVec2 healthTopLeft(topLeft.x - barWidth - 4, healthBottomRight.y - healthHeight);

        ImU32 healthColor = IM_COL32(
            (1.0f - healthPct) * 255,
            healthPct * 255,
            0,
            255
        );

        drawList->AddRectFilled(healthTopLeft, healthBottomRight, healthColor);
    }

    void ESP::DrawName(ImDrawList* drawList, const PlayerData& player, const ImVec2& screenPos)
    {
        if (player.name.empty()) return;

        ImVec2 textSize = ImGui::CalcTextSize(player.name.c_str());
        ImVec2 textPos(screenPos.x - textSize.x / 2, screenPos.y - 40);

        // Shadow
        drawList->AddText(ImVec2(textPos.x + 1, textPos.y + 1), IM_COL32(0, 0, 0, 255), player.name.c_str());
        // Text
        drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), player.name.c_str());
    }

    void ESP::DrawDistance(ImDrawList* drawList, const PlayerData& player, const ImVec2& screenPos)
    {
        char distText[32];
        snprintf(distText, sizeof(distText), "%.0fm", player.distance);

        ImVec2 textSize = ImGui::CalcTextSize(distText);
        ImVec2 textPos(screenPos.x - textSize.x / 2, screenPos.y - 20);

        // Shadow
        drawList->AddText(ImVec2(textPos.x + 1, textPos.y + 1), IM_COL32(0, 0, 0, 255), distText);
        // Text
        drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), distText);
    }

    void ESP::RenderSettings()
    {
        if (ImGui::CollapsingHeader("ESP Settings"))
        {
            ImGui::Checkbox("Enabled", &m_Enabled);
            ImGui::Checkbox("Draw Box", &m_DrawBox);
            ImGui::Checkbox("Draw Name", &m_DrawName);
            ImGui::Checkbox("Draw Health", &m_DrawHealth);
            ImGui::Checkbox("Draw Distance", &m_DrawDistance);

            ImGui::SliderFloat("Max Distance (Players)", &m_MaxDistance, 50.0f, 1000.0f);
            ImGui::SliderFloat("Max Distance (AI)", &m_MaxDistanceAI, 50.0f, 500.0f);
            ImGui::SliderFloat("Box Thickness", &m_BoxThickness, 1.0f, 5.0f);

            ImGui::ColorEdit4("Enemy Color", (float*)&m_ColorEnemy);
            ImGui::ColorEdit4("Team Color", (float*)&m_ColorTeam);
            ImGui::ColorEdit4("AI Color", (float*)&m_ColorAI);
        }
    }
}
