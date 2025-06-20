#include "config.hpp"
#include <fstream>
#include "globals.h" 
#include <imgui/imgui.h>

std::ostream& operator<<(std::ostream& os, const ImVec4& v) {
    os << v.x << " " << v.y << " " << v.z << " " << v.w;
    return os;
}

std::istream& operator>>(std::istream& is, ImVec4& v) {
    is >> v.x >> v.y >> v.z >> v.w;
    return is;
}


void save_config(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open())
        return;

    file << esp_enabled << "\n";
    file << aimbot_enabled << "\n";
    file << draw_fov << "\n";
    file << draw_teamate << "\n";
    file << aimbot_fov << "\n";
    file << aimbot_speed << "\n";
    file << bone_aiming << "\n";
    file << aim_key << "\n";
    file << visibility_aim_check << "\n";
    file << color_enemy_visible << "\n";
    file << color_enemy_hidden << "\n";
    file << color_team_visible << "\n";
    file << color_team_hidden << "\n";
    file << color_fov << "\n";

    file.close();
}

void load_config(const std::string& filename) {

    std::ifstream file(filename);
    if (!file.is_open()) {
        save_config(filename); 
        return;
    }

    if (!file.is_open())
        return;

    file >> esp_enabled;
    file >> aimbot_enabled;
    file >> draw_fov;
    file >> draw_teamate;
    file >> aimbot_fov;
    file >> aimbot_speed;
    file >> bone_aiming;
    file >> aim_key;
    file >> visibility_aim_check;
    file >> color_enemy_visible;
    file >> color_enemy_hidden;
    file >> color_team_visible;
    file >> color_team_hidden;
    file >> color_fov;

    file.close();
}
