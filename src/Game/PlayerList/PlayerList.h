#pragma once
#include <vector>
#include <mutex>
#include <string>
#include <chrono>
#include "../Core/Types.h"

namespace ARC
{
    struct PlayerData
    {
        uint64_t address;
        UnrealEngine::FVector position;
        UnrealEngine::FRotator rotation;
        std::string name;
        float health;
        float maxHealth;
        float armor;
        float maxArmor;
        float distance;
        bool isAlive;
        bool isLocalPlayer;
        unsigned int entityType;
        std::chrono::steady_clock::time_point lastUpdate;

        bool IsStale() const
        {
            auto now = std::chrono::steady_clock::now();
            return std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdate).count() > 1000;
        }
    };

    class PlayerList
    {
    public:
        static void Update();
        static void Clear();
        static std::vector<PlayerData> GetPlayers();
        static const PlayerData* GetLocalPlayer();

    private:
        static void RemoveStale();
        static inline std::vector<PlayerData> m_Players;
        static inline std::mutex m_Mutex;
    };
}