#pragma once
#include "../Core/Types.h"
#include <mutex>

namespace ARC
{
    class Camera
    {
    public:
        static void Update();
        static bool WorldToScreen(const UnrealEngine::FVector& worldPos, UnrealEngine::FVector2D& screenPos);
        static const UnrealEngine::FCameraCacheEntry& GetView();
        static UnrealEngine::FVector2D GetScreenSize() { return m_ScreenSize; }

    private:
        static inline UnrealEngine::FCameraCacheEntry m_CameraView{};
        static inline std::mutex m_Mutex;
        static inline UnrealEngine::FVector2D m_ScreenSize{3440.0f, 1440.0f};

        static void GetAxes(const UnrealEngine::FRotator& rotation,
                          UnrealEngine::FVector* xAxis,
                          UnrealEngine::FVector* yAxis,
                          UnrealEngine::FVector* zAxis);
    };
}
