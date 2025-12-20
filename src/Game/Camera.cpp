#include "Camera.h"
#include "../Core/Memory.h"
#include "../Core/Decrypt.h" // Подключаем дешифратор
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace ARC
{
    void Camera::Update()
    {
        std::lock_guard<std::mutex> lock(m_Mutex);

        // Используем Scatter чтение для оптимизации, если возможно,
        // но для дешифровки часто нужны последовательные чтения.

        // 1. GWorld (Decrypt)
        uint64_t gWorld = Decrypt::GetGWorld(UnrealEngine::Offsets::GWorld);
        if (!gWorld) return;

        // 2. GameInstance (Decrypt)
        uint64_t gameInstance = Decrypt::GetGameInstance(gWorld);
        if (!gameInstance) return;

        // 3. LocalPlayers
        uint64_t localPlayersPtr = Memory::Read<uint64_t>(gameInstance + UnrealEngine::Offsets::GameInstance::LocalPlayers);
        if (!localPlayersPtr) return;

        // 4. LocalPlayer[0]
        uint64_t localPlayer = Memory::Read<uint64_t>(localPlayersPtr);
        if (!localPlayer) return;

        // 5. PlayerController
        uint64_t playerController = Memory::Read<uint64_t>(localPlayer + UnrealEngine::Offsets::UPlayer::PlayerController);
        if (!playerController) return;

        // 6. PlayerCameraManager (Decrypt - ВАЖНО!)
        // Старый оффсет +0x0580 или чтение напрямую больше не работают надежно
        uint64_t cameraManager = Decrypt::GetPlayerCameraManager(playerController);

        // Если дешифратор вернул 0 (например, если ключи устарели), можно попробовать старый метод как запасной:
        if (!cameraManager)
            cameraManager = Memory::Read<uint64_t>(playerController + 0x340); // Проверьте оффсет PlayerCameraManager в Types.h (обычно 0x340 или 0x348)

        if (!cameraManager) return;

        // 7. Читаем ViewTarget (POV)
        // Оффсет CameraCachePrivate тоже может меняться (обычно 0x2220 + 0x10 или 0x400)
        // В вашем Types.h это ViewTarget = 0x0400. Проверьте актуальность.
        Memory::ReadBuffer(cameraManager + UnrealEngine::Offsets::APlayerCameraManager::ViewTarget,
            &m_CameraView,
            sizeof(m_CameraView));
    }

    // Остальные функции (GetAxes, WorldToScreen) остаются без изменений,
    // так как это чистая математика.

    void Camera::GetAxes(const UnrealEngine::FRotator& rotation,
        UnrealEngine::FVector* xAxis,
        UnrealEngine::FVector* yAxis,
        UnrealEngine::FVector* zAxis)
    {
        float sp, sy, cp, cy, sr, cr;
        float radPitch = rotation.Pitch * (M_PI / 180.0f);
        float radYaw = rotation.Yaw * (M_PI / 180.0f);
        float radRoll = rotation.Roll * (M_PI / 180.0f);

        sp = sin(radPitch); cp = cos(radPitch);
        sy = sin(radYaw);   cy = cos(radYaw);
        sr = sin(radRoll);  cr = cos(radRoll);

        xAxis->X = cp * cy; xAxis->Y = cp * sy; xAxis->Z = sp;

        UnrealEngine::FRotator tempRot = rotation;
        tempRot.Yaw += 90.0f; tempRot.Pitch = 0.0f;
        float tempYaw = tempRot.Yaw * (M_PI / 180.0f);
        yAxis->X = cos(tempYaw); yAxis->Y = sin(tempYaw); yAxis->Z = 0.0f;

        zAxis->X = -sr * cy - cr * sp * sy;
        zAxis->Y = sr * sy - cr * sp * cy;
        zAxis->Z = cr * cp;
    }

    bool Camera::WorldToScreen(const UnrealEngine::FVector& worldPos, UnrealEngine::FVector2D& screenPos)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);

        UnrealEngine::FVector vAxisX, vAxisY, vAxisZ;
        GetAxes(m_CameraView.POV.Rotation, &vAxisX, &vAxisY, &vAxisZ);

        UnrealEngine::FVector vDelta = worldPos - m_CameraView.POV.Location;
        UnrealEngine::FVector vTransformed;
        vTransformed.X = vDelta.X * vAxisY.X + vDelta.Y * vAxisY.Y + vDelta.Z * vAxisY.Z;
        vTransformed.Y = vDelta.X * vAxisZ.X + vDelta.Y * vAxisZ.Y + vDelta.Z * vAxisZ.Z;
        vTransformed.Z = vDelta.X * vAxisX.X + vDelta.Y * vAxisX.Y + vDelta.Z * vAxisX.Z;

        if (vTransformed.Z < 1.0f) vTransformed.Z = 1.0f;

        float fov = m_CameraView.POV.Fov;
        UnrealEngine::FVector2D center = { m_ScreenSize.X * 0.5f, m_ScreenSize.Y * 0.5f };

        screenPos.X = center.X + vTransformed.X * (center.Y / tanf(fov * M_PI / 360.0f)) / vTransformed.Z;
        screenPos.Y = center.Y - vTransformed.Y * (center.Y / tanf(fov * M_PI / 360.0f)) / vTransformed.Z;

        if (screenPos.X < 0 || screenPos.X > m_ScreenSize.X ||
            screenPos.Y < 0 || screenPos.Y > m_ScreenSize.Y)
            return false;

        return true;
    }

    const UnrealEngine::FCameraCacheEntry& Camera::GetView()
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        return m_CameraView;
    }
}