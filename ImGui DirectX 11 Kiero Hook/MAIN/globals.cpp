#include "globals.h"
#include "../CppSDK/SDK/Engine_classes.hpp"
#include "imgui/imgui.h"
#include "../CppSDK/SDK/Solarland_classes.hpp"
#include "Bones.h"
#include "Windows.h"
#include <unordered_map>
namespace Math {
    bool WorldToScreen(SDK::APlayerController playerController, SDK::FVector& worldPos, SDK::FVector2D* screenPos) {
        return playerController.ProjectWorldLocationToScreen(worldPos, screenPos, true);
    }

    bool IsVisible(SDK::UWorld* world, SDK::FVector& viewPoint, SDK::FVector& targetWorldPos)
    {
        SDK::FHitResult hitResult;
        return (!SDK::UKismetSystemLibrary::LineTraceSingle(
            world, viewPoint, targetWorldPos,
            SDK::ETraceTypeQuery::TraceTypeQuery2, // trace channel
            false,                                 // trace complex
            SDK::TArray<SDK::AActor*>(),           // actors to ignore
            SDK::EDrawDebugTrace::None,
            &hitResult,
            true,
            Color::red, Color::red, NULL
        ));
    }


    float Distance3D(const SDK::FVector& p1, const SDK::FVector& p2) {
        float dx = p1.X - p2.X, dy = p1.Y - p2.Y, dz = p1.Z - p2.Z;
        return SDK::UKismetMathLibrary::Sqrt(dx * dx + dy * dy + dz * dz) / 100.0f;
    }

    float Distance2D(SDK::FVector2D p1, SDK::FVector2D p2) {
        float dx = p1.X - p2.X, dy = p1.Y - p2.Y;
        return SDK::UKismetMathLibrary::Sqrt(dx * dx + dy * dy);
    }



}

namespace Globals {

    SDK::UWorld* world = nullptr;
    SDK::ULevel* persistentLevel = nullptr;
    SDK::APlayerController* localPlayerController = nullptr;
    SDK::APawn* localPawn = nullptr;
    SDK::ACharacter* playerCharacter = nullptr;
    SDK::ASolarTeamInfo* localTeam = nullptr;
    SDK::ASolarFunctionalityCharacter* BestTarget = nullptr;
    SDK::FVector OriginalLocation;
    SDK::FRotator OriginalRotation;
    const SDK::APlayerState* ent{};

    bool UpdateInstance() {
        world = SDK::UWorld::GetWorld();
        if (!world || !world->OwningGameInstance || world->OwningGameInstance->LocalPlayers.Num() == 0)
            return false;

        localPlayerController = world->OwningGameInstance->LocalPlayers[0]->PlayerController;
        if (!localPlayerController) return false;

        localPawn = localPlayerController->AcknowledgedPawn;
        if (!localPawn) return false;

        persistentLevel = world->PersistentLevel;
        if (!persistentLevel) return false;

        playerCharacter = localPawn->IsA(SDK::ACharacter::StaticClass()) ?
            static_cast<SDK::ACharacter*>(localPawn) : nullptr;

        return true;
    }

    inline ImVec2 GetScreenCenter() {
        static float cx = static_cast<float>(GetSystemMetrics(SM_CXSCREEN)) / 2.0f;
        static float cy = static_cast<float>(GetSystemMetrics(SM_CYSCREEN)) / 2.0f;
        return ImVec2(cx, cy);
    }

    void DrawBox(ImDrawList* drawList, ImVec2 topLeft, ImVec2 bottomRight, ImU32 color, float thickness = 1.5f) {
        float lineX = (bottomRight.x - topLeft.x) * 0.3f;
        float lineY = (bottomRight.y - topLeft.y) * 0.3f;

        // Top Left
        drawList->AddLine(topLeft, ImVec2(topLeft.x + lineX, topLeft.y), color, thickness);
        drawList->AddLine(topLeft, ImVec2(topLeft.x, topLeft.y + lineY), color, thickness);
        // Top Right
        drawList->AddLine(ImVec2(bottomRight.x - lineX, topLeft.y), ImVec2(bottomRight.x, topLeft.y), color, thickness);
        drawList->AddLine(ImVec2(bottomRight.x, topLeft.y), ImVec2(bottomRight.x, topLeft.y + lineY), color, thickness);
        // Bottom Left
        drawList->AddLine(ImVec2(topLeft.x, bottomRight.y - lineY), ImVec2(topLeft.x, bottomRight.y), color, thickness);
        drawList->AddLine(ImVec2(topLeft.x, bottomRight.y), ImVec2(topLeft.x + lineX, bottomRight.y), color, thickness);
        // Bottom Right
        drawList->AddLine(ImVec2(bottomRight.x - lineX, bottomRight.y), bottomRight, color, thickness);
        drawList->AddLine(ImVec2(bottomRight.x, bottomRight.y - lineY), bottomRight, color, thickness);
    }

    void LoopActor() {

        if (!persistentLevel || !localPlayerController || !localPlayerController->AcknowledgedPawn)
            return;

        SDK::FVector localWorldPos;
        SDK::TArray<SDK::AActor*> actors = persistentLevel->Actors;
        SDK::ASolarFunctionalityCharacter* closestTarget = nullptr;
        SDK::FVector2D targetHeadScreen;
        float closestDist2 = FLT_MAX;
        ImVec2 screenCenter = GetScreenCenter();

        for (auto* actor : actors) {
            if (!actor || !actor->IsA(SDK::EClassCastFlags::Pawn)) continue;

            auto* playerChar = static_cast<SDK::ASolarFunctionalityCharacter*>(actor);
            if (!playerChar || !playerChar->Mesh || !playerChar->PlayerState) continue;

            auto* playerCharState = static_cast<SDK::ASolarPlayerState*>(playerChar->PlayerState);
            if (!playerCharState || !playerCharState->Team) continue;

            // local player
            if (playerChar == localPlayerController->AcknowledgedPawn) {
                localWorldPos = playerChar->K2_GetActorLocation();
                localTeam = playerCharState->Team;
                continue;
            }


            auto* characterBase = static_cast<SDK::ASolarCharacterBase*>(playerChar);
            if (!characterBase || !characterBase->K2_IsAlive()) continue;


            if (playerCharState->Team == localTeam) continue;



            auto* skeletalMesh = playerChar->Mesh;
            SDK::FVector enemyPos = playerChar->K2_GetActorLocation();
            float distanceM = Math::Distance3D(localWorldPos, enemyPos);
            if (distanceM > Menu::maxDistance) continue;


            std::unordered_map<std::string, SDK::FName> BoneMap;
            int boneCount = skeletalMesh->GetNumBones();
            for (int i = 0; i < boneCount; ++i)
                BoneMap[skeletalMesh->GetBoneName(i).ToString()] = skeletalMesh->GetBoneName(i);

            ImDrawList* drawList = ImGui::GetBackgroundDrawList();


            if (Menu::bSkeleton) {
                for (auto& pair : BonesData::BonePairs) {
                    auto itFrom = BoneMap.find(pair.first);
                    auto itTo = BoneMap.find(pair.second);
                    if (itFrom == BoneMap.end() || itTo == BoneMap.end()) continue;

                    SDK::FVector fromPos = skeletalMesh->GetSocketLocation(itFrom->second);
                    SDK::FVector toPos = skeletalMesh->GetSocketLocation(itTo->second);
                    SDK::FVector2D screenFrom, screenTo;

                    if (Math::WorldToScreen(*localPlayerController, fromPos, &screenFrom) &&
                        Math::WorldToScreen(*localPlayerController, toPos, &screenTo)) {

                        bool visible = Math::IsVisible(world, localWorldPos, fromPos) || Math::IsVisible(world, localWorldPos, toPos);
                        ImU32 color = visible ? IM_COL32(0, 255, 0, 255) : IM_COL32(255, 0, 0, 255);
                        drawList->AddLine(ImVec2(screenFrom.X, screenFrom.Y), ImVec2(screenTo.X, screenTo.Y), color, 0.5f);
                    }
                }
            }

            if (Menu::bAlert360) {
                SDK::FRotator camRot = localPlayerController->PlayerCameraManager->GetCameraRotation();
                float yawRad = camRot.Yaw * (3.14159265f / 180.0f);

                SDK::FVector2D forward = { cosf(yawRad), sinf(yawRad) };
                SDK::FVector2D toEnemy = { enemyPos.X - localWorldPos.X, enemyPos.Y - localWorldPos.Y };

                float len = SDK::UKismetMathLibrary::Sqrt(toEnemy.X * toEnemy.X + toEnemy.Y * toEnemy.Y);
                if (len > 1.f) {
                    toEnemy.X /= len;
                    toEnemy.Y /= len;

                    float dot = forward.X * toEnemy.X + forward.Y * toEnemy.Y;
                    float det = forward.X * toEnemy.Y - forward.Y * toEnemy.X;
                    float angle = atan2f(det, dot);

                    ImVec2 enemyPosOnCircle(
                        screenCenter.x + sinf(angle) * Menu::radius,
                        screenCenter.y - cosf(angle) * Menu::radius
                    );

                    bool bVisible = Math::IsVisible(world, localWorldPos, enemyPos);
                    ImU32 color = bVisible ? IM_COL32(0, 255, 0, 255)
                        : IM_COL32(255, 0, 0, 255);

                    drawList->AddCircle(enemyPosOnCircle, 6.0f, color, 64, 2.0f);
                }
            }

            // Box & Health
            SDK::FVector headPos = skeletalMesh->GetSocketLocation(BoneMap["HEAD"]);
            SDK::FVector footPos = skeletalMesh->GetSocketLocation(BoneMap["foot_l"]);
            SDK::FVector footRPos;
            if (BoneMap.find("foot_r") != BoneMap.end()) {
                footRPos = skeletalMesh->GetSocketLocation(BoneMap["foot_r"]);
                if (footRPos.Z < footPos.Z) footPos = footRPos;
            }

            SDK::FVector2D screenHead, screenFoot;
            if (Math::WorldToScreen(*localPlayerController, headPos, &screenHead) &&
                Math::WorldToScreen(*localPlayerController, footPos, &screenFoot)) {

                float boxHeight = (screenFoot.Y - screenHead.Y) + 12.0f;
                float boxWidth = boxHeight / 2.0f + 5.0f;
                ImVec2 topLeft(screenHead.X - boxWidth / 2.0f, screenHead.Y - 6.0f);
                ImVec2 bottomRight(screenHead.X + boxWidth / 2.0f, screenFoot.Y + 6.0f);

                if (Menu::bBox)
                    DrawBox(drawList, topLeft, bottomRight, Math::IsVisible(world, localWorldPos, headPos) ? IM_COL32(0, 255, 0, 255) : IM_COL32(255, 0, 0, 255));

                if (Menu::bHealth) {
                    float healthPerc = characterBase->GetCurrentHealth() / characterBase->GetMaxHealth();
                    ImU32 healthColor = healthPerc > 0.5f ? IM_COL32(0, 255, 0, 255)
                        : healthPerc > 0.25f ? IM_COL32(255, 255, 0, 255)
                        : IM_COL32(255, 0, 0, 255);

                    drawList->AddRectFilled(ImVec2(topLeft.x - 8.0f, topLeft.y),
                        ImVec2(topLeft.x - 5.0f, bottomRight.y),
                        IM_COL32(50, 50, 50, 200));
                    drawList->AddRectFilled(ImVec2(topLeft.x - 8.0f, topLeft.y + (1.0f - healthPerc) * boxHeight),
                        ImVec2(topLeft.x - 5.0f, bottomRight.y),
                        healthColor);
                }
            }

            if (Menu::bLine) {
                SDK::FVector linePos = headPos;
                SDK::FVector2D screenPos;
                if (Math::WorldToScreen(*localPlayerController, linePos, &screenPos)) {


                    ImU32 lineColor = IM_COL32(255, 0, 0, 255);




                    bool visible = Math::IsVisible(world, localWorldPos, linePos);
                    lineColor = visible ? IM_COL32(0, 255, 0, 255) : IM_COL32(255, 0, 0, 255);

                    ImVec2 screenBottom(ImGui::GetIO().DisplaySize.x / 2,
                        ImGui::GetIO().DisplaySize.y);
                    drawList->AddLine(screenBottom,
                        ImVec2(screenPos.X, screenPos.Y),
                        lineColor,
                        0.5f);
                }
            }




            if (Menu::bDistance) {
                SDK::FVector2D screenPos;
                if (Math::WorldToScreen(*localPlayerController, enemyPos, &screenPos)) {
                    ImU32 col = distanceM <= 50.0f ? IM_COL32(0, 255, 0, 255)
                        : distanceM <= 150.0f ? IM_COL32(255, 255, 0, 255)
                        : IM_COL32(255, 100, 0, 255);
                    char buf[64]; sprintf_s(buf, "%.1f m", distanceM);

                    float scale = 1.5f;
                    drawList->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(screenPos.X + 1, screenPos.Y + 1), IM_COL32(0, 0, 0, 150), buf);
                    drawList->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(screenPos.X, screenPos.Y), col, buf);
                }
            }

            if (Menu::bNames)
            {
                SDK::FVector2D screenPos;
                if (Math::WorldToScreen(*localPlayerController, playerChar->RootComponent->RelativeLocation, &screenPos))
                {
                    std::string name = playerCharState->GetPlayerName().ToString();

                    ImGui::GetBackgroundDrawList()->AddText(
                        ImVec2(screenPos.X, screenPos.Y - 15),
                        IM_COL32(255, 255, 0, 255),
                        name.c_str()
                    );
                }

            }





            // FOV
            if (Menu::draw_fov)
                drawList->AddCircle(screenCenter, Menu::FOV, IM_COL32(255, 255, 0, 255), 100, 0.5f);

            // Aimbot
            if (Menu::bAimbot && (GetAsyncKeyState(VK_XBUTTON1) & 0x8000)) {
                SDK::FVector2D headScreen;
                if (Math::WorldToScreen(*localPlayerController, headPos, &headScreen)) {
                    float dx = headScreen.X - screenCenter.x;
                    float dy = headScreen.Y - screenCenter.y;
                    float dist2 = dx * dx + dy * dy;
                    if (dist2 <= Menu::FOV * Menu::FOV && dist2 < closestDist2) {
                        closestDist2 = dist2;
                        closestTarget = playerChar;
                        targetHeadScreen = headScreen;
                    }
                }
            }
        }

        if (closestTarget) {
            float dx = (targetHeadScreen.X - screenCenter.x) / 2.0f;
            float dy = (targetHeadScreen.Y - screenCenter.y) / 2.0f;
            mouse_event(MOUSEEVENTF_MOVE, static_cast<LONG>(dx), static_cast<LONG>(dy), 0, 0);
        }


      

      

    }

}

