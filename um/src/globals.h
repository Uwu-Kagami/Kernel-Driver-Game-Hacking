#pragma once
#include <Windows.h>
#include <cstdint>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx11.h>
#include <imgui/imgui_impl_win32.h>


extern HANDLE driver_handle;
extern uintptr_t client_module;
extern uintptr_t entity_list;
extern bool show_menu;
extern bool esp_enabled;
extern bool aimbot_enabled;
extern float aimbot_fov;
extern float aimbot_speed;
extern int aim_key;
extern bool capturing_key;
extern int bone_aiming;
extern bool draw_fov;
extern bool draw_teamate;
extern bool visibility_aim_check;
extern ImVec4 color_enemy_visible;
extern ImVec4 color_enemy_hidden;
extern ImVec4 color_team_visible;
extern ImVec4 color_team_hidden;
extern ImVec4 color_fov;
