#pragma once
#include <cstdint>
#include <vector>
#include <string>

namespace UnrealEngine
{
    // ============================================
    //  BASIC TYPES
    // ============================================

    struct FVector2D
    {
        float X, Y;

        FVector2D() : X(0), Y(0) {}
        FVector2D(float x, float y) : X(x), Y(y) {}

        FVector2D operator+(const FVector2D& other) const { return FVector2D{ X + other.X, Y + other.Y }; }
        FVector2D operator-(const FVector2D& other) const { return FVector2D{ X - other.X, Y - other.Y }; }
        FVector2D operator*(float scalar) const { return FVector2D{ X * scalar, Y * scalar }; }

        float Distance(const FVector2D& other) const {
            FVector2D dif = other - *this;
            return sqrt(dif.X * dif.X + dif.Y * dif.Y);
        }
    };

    struct FVector
    {
        double X, Y, Z;

        FVector() : X(0), Y(0), Z(0) {}
        FVector(double x, double y, double z) : X(x), Y(y), Z(z) {}

        FVector operator+(const FVector& other) const { return FVector{ X + other.X, Y + other.Y, Z + other.Z }; }
        FVector operator-(const FVector& other) const { return FVector{ X - other.X, Y - other.Y, Z - other.Z }; }
        FVector operator*(float scalar) const { return FVector{ X * scalar, Y * scalar, Z * scalar }; }

        float Distance(const FVector& other) const {
            auto dist = *this - other;
            return sqrt(dist.X * dist.X + dist.Y * dist.Y + dist.Z * dist.Z) / 100.f; // Convert to meters
        }

        bool IsValid() const { return !(X == 0 && Y == 0 && Z == 0); }
    };

    struct FRotator
    {
        double Pitch, Yaw, Roll;

        FRotator() : Pitch(0), Yaw(0), Roll(0) {}
        FRotator(double p, double y, double r) : Pitch(p), Yaw(y), Roll(r) {}
    };

    struct FQuat
    {
        double X, Y, Z, W;
    };

    struct FTransform
    {
        FQuat Rotation;
        FVector Translation;
        char pad_0038[8];
        FVector Scale3D;
        char pad_0058[8];
    };

    template <class T>
    struct TArray
    {
        T data;
        int count;
        int max;
    };

    struct FString
    {
        __int64 data;
        int count;
        int max;
    };

    struct FMinimalViewInfo
    {
        FVector Location;
        FRotator Rotation;
        float Fov;
        float DesiredFOV;
        float OrthoWidth;
        float OrthoNearClipPlane;
        float OrthoFarClipPlane;
        float PerspectiveNearClipPlane;
        float AspectRatio;
    };

    struct FCameraCacheEntry
    {
        float Timestamp;
        char pad_0004[12];
        FMinimalViewInfo POV;
    };

    // ============================================
    //  ACTOR TYPES
    // ============================================

    enum class EActorType : unsigned __int32
    {
        EACTOR_SPAWN = 0x011F0000,
        EACTOR_PLAYER = 0x010E0000,
        EACTOR_CHEST = 0x00080000,
        EACTOR_ARC_WASP = 0x000A0000,
        EACTOR_LOOT = 0x000C0000,
    };

    // Game offsets (update after patches)
    namespace Offsets
    {
        inline auto GWorld = 0x3A0803A;

        namespace World
        {
            constexpr auto PersistentLevel = 0x00E8;
            constexpr auto Levels = 0x0248;
        }

        namespace Level
        {
            constexpr auto Actors = 0x00D0;
        }

        namespace GameInstance
        {
            constexpr auto LocalPlayers = 0x2A140FF;
        }

        namespace UPlayer
        {
            constexpr auto PlayerController = 0x0058;
        }

        namespace Actor
        {
            constexpr auto RootComponent = 0x01F0;
        }

        namespace Controller
        {
            constexpr auto PlayerState = 0x0338;
            constexpr auto Pawn = 0x0370;
        }

        namespace Pawn
        {
            constexpr auto PlayerState = 0x0350;
            constexpr auto RootComponent = 0x01F0;
        }

        namespace PlayerState
        {
            constexpr auto PlayerNamePrivate = 0x03C8;
            constexpr auto HealthInfo = 0x4b0;
            constexpr auto PlayerStatus = 0x4d8;
        }

        namespace USceneComponent
        {
            constexpr auto RelativeLocation = 0x0190;
            constexpr auto RelativeRotation = 0x01A8;
        }

        namespace APlayerCameraManager
        {
            constexpr auto ViewTarget = 0x30004B0;
        }
    }

    struct FPlayerHealthInfo
    {
        double Health;
        double MaxHealth;
        double Armor;
        double MaxArmor;
        bool bIsDbno;
        char pad_0021[7];
    };
}
