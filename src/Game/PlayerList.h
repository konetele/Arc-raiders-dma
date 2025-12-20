#pragma once
#include "../Core/Types.h"
#include <vector>
#include <mutex>
#include <chrono>
#include <string>

namespace ARC
{
    struct PlayerData
    {
        uint64_t address{0};
        UnrealEngine::FVector position;
        UnrealEngine::FRotator rotation;
        std::string name;
        int team{0};
        float health{100.0f};
        float maxHealth{100.0f};
        float armor{0.0f};
        float maxArmor{0.0f};
        bool isAlive{true};
        bool isLocalPlayer{false};
        unsigned int entityType{0};
        float distance{0.0f};

        std::chrono::steady_clock::time_point lastUpdate;

        bool IsStale() const
        {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastUpdate).count();
            return elapsed > 5; // Remove if not updated for 5 seconds
        }
    };

    class PlayerList
    {
    public:
        static void Update();
        static void Clear();
        static std::vector<PlayerData> GetPlayers(); // Thread-safe copy

        static const PlayerData* GetLocalPlayer();

    private:
        static inline std::vector<PlayerData> m_Players;
        static inline std::mutex m_Mutex;

        static void RemoveStale();
        static bool ReadPlayerData(uint64_t playerAddress, PlayerData& outData);
    };
}
