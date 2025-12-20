#include "PlayerList.h"
#include "Camera.h"
#include "../Core/Memory.h"
#include "../Core/Decrypt.h"
#include <algorithm>
#include <vector>
#include <iostream> // Для std::cout
#include <cstdlib>

namespace ARC
{
    void PlayerList::Update()
    {
        std::lock_guard<std::mutex> lock(m_Mutex);

        // --- ОТЛАДКА: Проверяем GWorld ---
        static bool gWorldLogged = false;
        uint64_t gWorld = Decrypt::GetGWorld(UnrealEngine::Offsets::GWorld);

        if (!gWorld) {
            if (!gWorldLogged) {
                std::cout << "[!] GWorld is 0! Check Decrypt.cpp constants (WORLD_A...)." << std::endl;
                gWorldLogged = true; // Чтобы не спамить
            }
            return;
        }
        else {
            if (gWorldLogged) {
                std::cout << "[+] GWorld found: " << std::hex << gWorld << std::dec << std::endl;
                gWorldLogged = false;
            }
        }

        // --- ОТЛАДКА: Проверяем PersistentLevel ---
        uint64_t persistentLevel = Memory::Read<uint64_t>(gWorld + UnrealEngine::Offsets::World::PersistentLevel);
        if (!persistentLevel) {
            // std::cout << "[!] PersistentLevel is 0" << std::endl; 
            return;
        }

        // --- ОТЛАДКА: Проверяем ActorArray ---
        UnrealEngine::TArray<uint64_t> actorArray;
        Memory::ReadBuffer(persistentLevel + UnrealEngine::Offsets::Level::Actors, &actorArray, sizeof(actorArray));

        if (actorArray.count <= 0) {
            // std::cout << "[!] ActorArray is empty. Count: " << actorArray.count << std::endl;
            return;
        }

        // Если дошли сюда, значит актеры есть
        // std::cout << "[+] Actors found: " << actorArray.count << std::endl;

        // --- ОТЛАДКА: Проверяем GameInstance ---
        uint64_t gameInstance = Decrypt::GetGameInstance(gWorld);
        if (!gameInstance) {
            std::cout << "[!] GameInstance failed to decrypt" << std::endl;
            return;
        }

        // Читаем локального игрока
        uint64_t localPlayersPtr = Memory::Read<uint64_t>(gameInstance + UnrealEngine::Offsets::GameInstance::LocalPlayers);
        if (!localPlayersPtr) {
            std::cout << "[!] LocalPlayersPtr is 0 (Offset: " << std::hex << UnrealEngine::Offsets::GameInstance::LocalPlayers << ")" << std::endl;
            return;
        }

        uint64_t localPlayer = Memory::Read<uint64_t>(localPlayersPtr);
        uint64_t playerController = Memory::Read<uint64_t>(localPlayer + UnrealEngine::Offsets::UPlayer::PlayerController);
        uint64_t localPawn = Memory::Read<uint64_t>(playerController + UnrealEngine::Offsets::Controller::Pawn);

        // Читаем всех актеров
        std::vector<uint64_t> actors(actorArray.count);
        Memory::ReadBuffer(actorArray.data, actors.data(), actorArray.count * sizeof(uint64_t));

        int playersFound = 0;

        for (uint64_t actorAddr : actors)
        {
            if (!actorAddr) continue;

            unsigned int actorType = Memory::Read<unsigned int>(actorAddr + 0x70); // ClassDefaultObject ID offset?
            // Или проверка по имени класса (GNames), если ID меняются

            // ВРЕМЕННО: Считаем всех актеров игроками для теста, если тип не совпадает
            // (Раскомментируйте, если хотите видеть ВСЕХ актеров в списке для проверки координат)
            // bool isPlayer = true; 

            bool isPlayer = (actorType == (unsigned int)UnrealEngine::EActorType::EACTOR_PLAYER);

            if (isPlayer || actorType == (unsigned int)UnrealEngine::EActorType::EACTOR_ARC_WASP)
            {
                playersFound++;
                // ... (код добавления игрока, как был раньше) ...

                // Проверка на дубликаты
                auto it = std::find_if(m_Players.begin(), m_Players.end(),
                    [actorAddr](const PlayerData& p) { return p.address == actorAddr; });

                PlayerData playerData;
                if (it != m_Players.end()) playerData = *it;
                else playerData.address = actorAddr;

                uint64_t rootComponent = Memory::Read<uint64_t>(actorAddr + UnrealEngine::Offsets::Actor::RootComponent);
                if (rootComponent) {
                    Memory::ReadBuffer(rootComponent + UnrealEngine::Offsets::USceneComponent::RelativeLocation,
                        &playerData.position, sizeof(UnrealEngine::FVector));
                }

                playerData.isLocalPlayer = (actorAddr == localPawn);
                playerData.isAlive = true;
                playerData.name = "Player"; // Временное имя, если дешифровка строк сбоит

                // Сохраняем
                if (it != m_Players.end()) *it = playerData;
                else m_Players.push_back(playerData);
            }
        }

        RemoveStale();
    }

    // ... Остальные методы (Clear, GetPlayers...) без изменений ...
    void PlayerList::Clear() { std::lock_guard<std::mutex> lock(m_Mutex); m_Players.clear(); }
    std::vector<PlayerData> PlayerList::GetPlayers() { std::lock_guard<std::mutex> lock(m_Mutex); return m_Players; }
    const PlayerData* PlayerList::GetLocalPlayer() {
        std::lock_guard<std::mutex> lock(m_Mutex);
        auto it = std::find_if(m_Players.begin(), m_Players.end(), [](const PlayerData& p) { return p.isLocalPlayer; });
        return (it != m_Players.end()) ? &(*it) : nullptr;
    }
    void PlayerList::RemoveStale() {
        m_Players.erase(std::remove_if(m_Players.begin(), m_Players.end(), [](const PlayerData& p) { return p.IsStale(); }), m_Players.end());
    }
}