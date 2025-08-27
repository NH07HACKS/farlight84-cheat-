#pragma once
#include <vector>
#include <string>
#include <utility>

namespace BonesData {
    inline std::vector<std::pair<std::string, std::string>> BonePairs = {
        {"HEAD", "neck_01"},
        {"neck_01", "spine_03"},

        // Cột sống
        {"spine_03", "spine_02"},
        {"spine_02", "spine_01"},
        {"spine_01", "pelvis"},

        // Tay trái
        {"spine_03", "clavicle_l"},
        {"clavicle_l", "upperarm_l"},
        {"upperarm_l", "lowerarm_l"},
        {"lowerarm_l", "hand_l"},

        // Tay phải
        {"spine_03", "clavicle_r"},
        {"clavicle_r", "upperarm_r"},
        {"upperarm_r", "lowerarm_r"},
        {"lowerarm_r", "hand_r"},

      
        {"pelvis", "thigh_l"},
        {"thigh_l", "calf_l"},
        {"calf_l", "foot_l"},

        // Chân phải
        {"pelvis", "thigh_r"},
        {"thigh_r", "calf_r"},
        {"calf_r", "foot_r"},
    };
}
