#pragma once
#include "../CppSDK/SDK/Engine_classes.hpp"

namespace SDK {
    class UWorld;
    class ULevel;
    class APlayerController;
    class APawn;
    class ACharacter;
    class ASolarTeamInfo;

}

namespace Globals {
    extern SDK::UWorld* world;
    extern SDK::ULevel* persistentLevel;
    extern SDK::APlayerController* localPlayerController;
    extern SDK::APawn* localPawn;
    extern SDK::ACharacter* playerCharacter;
    extern SDK::ASolarTeamInfo* localTeam;


    bool UpdateInstance();
    void LoopActor();
    inline float rainbowColor[3]{};

}

namespace Menu {
    inline bool bLine = false;
    inline bool bSkeleton = true;
    inline bool bDistance = true;
    inline bool bAimbot = true;
    inline float FOV = 180.0f;
    inline bool dumpbone = false;
    inline bool draw_fov = false;
    inline bool bBox = true;
    inline bool bHealth = true;
    inline bool bNames = false;
    inline bool bAlert360 = true;
    inline float radius = 100.0f;
    inline float maxDistance = 250.0f;
}




namespace Math
{
    bool WorldToScreen(SDK::APlayerController playerController, SDK::FVector& worldPos, SDK::FVector2D* screenPos);
    bool IsVisible(SDK::UWorld* world, SDK::FVector& viewPoint, SDK::FVector& targetWorldPos);
    float Distance3D(const SDK::FVector& point1, const SDK::FVector& point2);
    float Distance2D(SDK::FVector2D point_1, SDK::FVector2D point_2);
};

namespace Color {
    static SDK::FLinearColor red = SDK::FLinearColor(1.f, 0.f, 0.f, 1.f);
    static SDK::FLinearColor green = SDK::FLinearColor(0.f, 1.f, 0.f, 1.f);
    static SDK::FLinearColor white = SDK::FLinearColor(1.f, 1.f, 1.f, 1.f);
    static SDK::FLinearColor yellow = SDK::FLinearColor(1.f, 1.f, 0.f, 1.f);
    static SDK::FLinearColor none = SDK::FLinearColor(0.f, 0.f, 0.f, 0.f);
}

