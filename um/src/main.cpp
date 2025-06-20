#include <Windows.h>
#include <dwmapi.h>
#include <d3d11.h>
#include <TlHelp32.h>
#include <iostream>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx11.h>
#include <imgui/imgui_impl_win32.h>
#include <string>
#include <vector>
#include "client_dll.hpp"
#include "offsets.hpp"
#include <algorithm>  
#include "globals.h"
#include <thread>
#include "driver.h"
#include "aim/aimbot.h"
#include "bones/bones.hpp"
#include "config.hpp"
#include "key_utils.hpp"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);


bool show_menu = false;
bool esp_enabled = true;
bool aimbot_enabled = false;
float aimbot_fov = 8.0f;
float aimbot_speed = 2.5f;
int aim_key = VK_LSHIFT;
bool capturing_key = false;
int bone_aiming = 6;
bool draw_fov = false;
bool draw_teamate = false;
bool visibility_aim_check = false;


ImVec4 color_enemy_visible = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);  
ImVec4 color_enemy_hidden = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);   
ImVec4 color_team_visible = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);   
ImVec4 color_team_hidden = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);    
ImVec4 color_fov = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);    



LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param) {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, w_param, l_param))
        return true;

    if (msg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, msg, w_param, l_param);
}


bool is_key_down(int vkey) {
    return (GetAsyncKeyState(vkey) & 0x8000);
}

struct Vector {
    float x, y, z;

    Vector() noexcept : x(), y(), z() {
    }
    Vector(float x, float y, float z) noexcept : x(x), y(y), z(z) {
    }

    Vector operator+(const Vector& v) const noexcept {
        return Vector{x + v.x, y + v.y, z + v.z};
    }
    Vector operator-(const Vector& v) const noexcept {
        return Vector{x - v.x, y - v.y, z - v.z};
    }
};

struct ViewMatrix {
    float data[4][4];

    float* operator[](int index) noexcept {
        return data[index];
    }
    const float* operator[](int index) const noexcept {
        return data[index];
    }
};

static bool world_to_screen(const Vector& world, Vector& screen, const ViewMatrix& vm) noexcept {
    float w = vm[3][0] * world.x + vm[3][1] * world.y + vm[3][2] * world.z + vm[3][3];
    if (w < 0.001f)
        return false;

    float x = vm[0][0] * world.x + vm[0][1] * world.y + vm[0][2] * world.z + vm[0][3];
    float y = vm[1][0] * world.x + vm[1][1] * world.y + vm[1][2] * world.z + vm[1][3];

    float inv_w = 1.0f / w;
    x *= inv_w;
    y *= inv_w;

    ImVec2 size = ImGui::GetIO().DisplaySize;
    screen.x = (size.x / 2.0f) + (x * size.x / 2.0f);
    screen.y = (size.y / 2.0f) - (y * size.y / 2.0f);

    return true;
}

DWORD get_process_id(const wchar_t* name) {
    PROCESSENTRY32W entry = {sizeof(entry)};
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE)
        return 0;

    DWORD pid = 0;
    for (Process32FirstW(snap, &entry); Process32NextW(snap, &entry);) {
        if (!_wcsicmp(entry.szExeFile, name)) {
            pid = entry.th32ProcessID;
            break;
        }
    }
    CloseHandle(snap);
    return pid;
}

uintptr_t get_module_base(DWORD pid, const wchar_t* module_name) {
    MODULEENTRY32W entry = {sizeof(entry)};
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (snap == INVALID_HANDLE_VALUE)
        return 0;

    uintptr_t base = 0;
    for (Module32FirstW(snap, &entry); Module32NextW(snap, &entry);) {
        if (!_wcsicmp(entry.szModule, module_name)) {
            base = reinterpret_cast<uintptr_t>(entry.modBaseAddr);
            break;
        }
    }
    CloseHandle(snap);
    return base;
}


INT APIENTRY WinMain(HINSTANCE hInst, HINSTANCE, PSTR, INT) {
    WNDCLASSEXW wc{
        sizeof(wc), CS_HREDRAW | CS_VREDRAW, window_proc, 0, 0, hInst, nullptr, nullptr, nullptr,
        nullptr,    L"OverlayClass",         nullptr};
    RegisterClassExW(&wc);

    int sw = GetSystemMetrics(SM_CXSCREEN), sh = GetSystemMetrics(SM_CYSCREEN);
    HWND hwnd = CreateWindowExW(WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT, wc.lpszClassName,
                                L"Meta Esp", WS_POPUP, 0, 0, sw, sh, nullptr, nullptr, wc.hInstance,
                                nullptr);
    SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
    MARGINS m = {-1};
    DwmExtendFrameIntoClientArea(hwnd, &m);

    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    sd.SampleDesc.Count = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = 2;
    sd.OutputWindow = hwnd;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    IDXGISwapChain* sc = nullptr;
    ID3D11Device* dev = nullptr;
    ID3D11DeviceContext* ctx = nullptr;
    ID3D11RenderTargetView* rtv = nullptr;
    ID3D11Texture2D* bb = nullptr;
    D3D_FEATURE_LEVEL fl;
    if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr,
                                             0, D3D11_SDK_VERSION, &sd, &sc, &dev, &fl, &ctx))) {
        MessageBoxA(0, "Failed to create D3D11 device/swapchain", "Error", MB_ICONERROR);
        return 1;
    }
    sc->GetBuffer(0, IID_PPV_ARGS(&bb));
    dev->CreateRenderTargetView(bb, nullptr, &rtv);
    bb->Release();

    load_config("config.txt");

    ShowWindow(hwnd, SW_SHOW);
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(dev, ctx);

    DWORD pid = get_process_id(L"cs2.exe");
    if (pid == 0) {
        MessageBoxA(0, "cs2.exe not found", "Error", MB_ICONERROR);
        return 1;
    }
    HANDLE drv =
        CreateFileW(L"\\\\.\\SexyDriver", GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    if (drv == INVALID_HANDLE_VALUE) {
        MessageBoxA(0, "Driver not found", "Error", MB_ICONERROR);
        return 1;
    }
    if (!driver::attach_to_process(drv, pid)) {
        MessageBoxA(0, "Failed to attach to process", "Error", MB_ICONERROR);
        return 1;
    }
    uintptr_t client = get_module_base(pid, L"client.dll");
    if (client == 0) {
        MessageBoxA(0, "client.dll not found", "Error", MB_ICONERROR);
        return 1;
    }
    client_module = client;
    driver_handle = drv;

    MSG msg = {};

    while (msg.message != WM_QUIT) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (GetAsyncKeyState(VK_INSERT) & 1)
            show_menu = !show_menu;

        LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
        if (show_menu) {
            SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle & ~WS_EX_TRANSPARENT);  // retirer le flag
        } else {
            SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle | WS_EX_TRANSPARENT);  // remettre le flag
        }



        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        bool skip_frame = false;

        if (show_menu) {
            ImGui::GetIO().MouseDrawCursor = true;

            ImGui::Begin("Meta ESP Menu", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::TextColored(ImVec4(0.2f, 0.7f, 0.9f, 1.0f), "Meta ESP Configuration");
            ImGui::Separator();

            if (ImGui::CollapsingHeader("ESP")) {
            if (ImGui::Checkbox("Enable ESP", &esp_enabled)) save_config("config.txt");
            if (ImGui::Checkbox("Draw TeamMate", &draw_teamate)) save_config("config.txt");
            }


            if (ImGui::CollapsingHeader("Aimbot")) {
            if (ImGui::Checkbox("Enable Aimbot", &aimbot_enabled)) save_config("config.txt");
            if (ImGui::Checkbox("Enable Draw Fov", &draw_fov)) save_config("config.txt");
            if (ImGui::Checkbox("Enable Visibility Check", &visibility_aim_check)) save_config("config.txt");

            if (ImGui::SliderFloat("Aimbot FOV", &aimbot_fov, 1.0f, 100.0f)) save_config("config.txt");
            if (ImGui::SliderFloat("Aimbot Smooth", &aimbot_speed, 1.0f, 100.0f)) save_config("config.txt");

                ImGui::Text("Aimbot Bone Target:");
                const char* bone_names[] = {"Cock", "Spine", "Neck", "Head"};
                if (ImGui::Combo("##BoneSelector", &bone_aiming, bone_names,
                                 IM_ARRAYSIZE(bone_names))) {
                    switch (bone_aiming) {
                        case 0:
                            bone_aiming = 0;
                            break;  // Cock
                        case 1:
                            bone_aiming = 4;
                            break;  // Spine
                        case 2:
                            bone_aiming = 5;
                            break;  // Neck
                        case 3:
                            bone_aiming = 6;
                            break;  // Head
                    }
                }
                save_config("config.txt");
                ImGui::Text("Aimbot Key: ");
                ImGui::SameLine();

                if (!capturing_key) {
                    if (ImGui::Button("Press a Key")) {
                        capturing_key = true;
                        aim_key = 0;
                    }
                } else {
                    ImGui::Text("Press any key... (ESC to cancel)");

                    bool keyCaptured = false;
                    for (int key = 1; key < 256; ++key) {
                        if ((GetAsyncKeyState(key) & 0x8000) && !(GetAsyncKeyState(key) & 1)) {
                            aim_key = key;
                            capturing_key = false;
                            save_config("config.txt");
                            keyCaptured = true;
                            break;
                        }
                    }

                    if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
                        capturing_key = false;
                        aim_key = 0;
                        keyCaptured = true;
                    }

                    if (!keyCaptured) {
                        ImGui::Text("Waiting for key press...");
                    }
                }

                if (aim_key != 0) {
                    ImGui::SameLine();
                    ImGui::Text("Current: %s", GetKeyName(aim_key).c_str());
                    save_config("config.txt");
                }
            }

            if (ImGui::CollapsingHeader("Misc")) {
                if (ImGui::ColorEdit4("Enemy Visible", (float*)&color_enemy_visible)) save_config("config.txt");
                if (ImGui::ColorEdit4("Enemy Hidden", (float*)&color_enemy_hidden)) save_config("config.txt");
                if (ImGui::ColorEdit4("Team Visible", (float*)&color_team_visible)) save_config("config.txt");
                if (ImGui::ColorEdit4("Team Hidden", (float*)&color_team_hidden)) save_config("config.txt");
                if (ImGui::ColorEdit4("Fov", (float*)&color_fov)) save_config("config.txt");

            }

            ImGui::End();
        }


        entity_list = driver::read_memory<uintptr_t>(
            driver_handle, client_module + cs2_dumper::offsets::client_dll::dwEntityList);

        uintptr_t dwEntityListBase = driver::read_memory<uintptr_t>(
            drv, client + cs2_dumper::offsets::client_dll::dwEntityList);
        uintptr_t dwLocalPlayerController = driver::read_memory<uintptr_t>(
            drv, client + cs2_dumper::offsets::client_dll::dwLocalPlayerController);


        if (draw_fov) {
            ImVec2 screen_center =
                ImVec2(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 2);
            float screen_fov = 90.0f;               // FOV horizontal du jeu (à ajuster !)
            float screen_radius = screen_center.x;  // demi-largeur (si horizontal)

            float fov_ratio = tanf(aimbot_fov * 0.5f * (3.14159f / 180.0f)) /
                              tanf(screen_fov * 0.5f * (3.14159f / 180.0f));
            float radius_pixels = screen_radius * fov_ratio;

            ImDrawList* draw_list = ImGui::GetForegroundDrawList();
            draw_list->AddCircle(screen_center, radius_pixels, ImColor(color_fov), 64, 2.0f);
        }

        if (!dwEntityListBase || !dwLocalPlayerController) {
            skip_frame = true;
        }

        uintptr_t dwLocalPlayerPawn = 0;
        uint32_t localTeam = 0;
        if (!skip_frame) {
            uintptr_t pawnHandle = driver::read_memory<uintptr_t>(
                drv, dwLocalPlayerController + cs2_dumper::offsets::client_dll::m_hPlayerPawn);
            uintptr_t pawnListEntry = driver::read_memory<uintptr_t>(
                drv, dwEntityListBase + 0x8 * ((pawnHandle & 0x7FFF) >> 9) + 0x10);
            dwLocalPlayerPawn =
                driver::read_memory<uintptr_t>(drv, pawnListEntry + 120 * (pawnHandle & 0x1FF));
            localTeam = driver::read_memory<uint32_t>(
                drv, dwLocalPlayerPawn + cs2_dumper::offsets::client_dll::m_iTeamNum);
            if (!dwLocalPlayerPawn)
                skip_frame = true;
        }

        uint32_t flags = driver::read_memory<uint32_t>(
            drv, dwLocalPlayerPawn + cs2_dumper::offsets::client_dll::m_fFlags);

        //if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
        //    if (flags & 1) {
        //        uint32_t jumpValue = 65537;
        //        driver::write_memory<uint32_t>(drv, client + cs2_dumper::offsets::client_dll::jump,
        //                                       jumpValue);
        //        Sleep(4);
        //        driver::write_memory<uint32_t>(drv, client + cs2_dumper::offsets::client_dll::jump,
        //                                       0);
        //    }
        //}

        ViewMatrix viewMatrix{};
        if (!skip_frame) {
            viewMatrix = driver::read_memory<ViewMatrix>(
                drv, client + cs2_dumper::offsets::client_dll::dwViewMatrix);
        }

        int found = 0;
        if (!skip_frame) {
            for (int i = 0; i < 64; i++) {
                uintptr_t listEntry = driver::read_memory<uintptr_t>(
                    drv, dwEntityListBase + 0x8 * ((i & 0x7FFF) >> 9) + 0x10);
                if (!listEntry)
                    continue;

                uintptr_t controller =
                    driver::read_memory<uintptr_t>(drv, listEntry + 120 * (i & 0x1FF));
                if (!controller)
                    continue;

                uintptr_t pawnHandle = driver::read_memory<uintptr_t>(
                    drv, controller + cs2_dumper::offsets::client_dll::m_hPlayerPawn);
                if (!pawnHandle)
                    continue;

                uintptr_t pawnListEntry = driver::read_memory<uintptr_t>(
                    drv, dwEntityListBase + 0x8 * ((pawnHandle & 0x7FFF) >> 9) + 0x10);
                if (!pawnListEntry)
                    continue;

                uintptr_t pawn =
                    driver::read_memory<uintptr_t>(drv, pawnListEntry + 120 * (pawnHandle & 0x1FF));
                if (!pawn || pawn == dwLocalPlayerPawn)
                    continue;

                uint32_t health = driver::read_memory<uint32_t>(
                    drv, pawn + cs2_dumper::offsets::client_dll::m_iHealth);
                if (health == 0 || health > 100)
                    continue;

                uintptr_t gamescene = driver::read_memory<uintptr_t>(drv, pawn + cs2_dumper::offsets::client_dll::m_pGameSceneNode);
                uintptr_t bonearray = driver::read_memory<uintptr_t>(drv, gamescene + cs2_dumper::offsets::client_dll::m_modelState + 0x80);



                Vector origin = driver::read_memory<Vector>(
                    drv, pawn + cs2_dumper::offsets::client_dll::m_vOldOrigin);
                Vector head = driver::read_memory<Vector>(drv, bonearray + bones::head * 32);
                head.z += 10.0f;  


                Vector screenOrigin, screenHead;
                if (!world_to_screen(origin, screenOrigin, viewMatrix))
                    continue;
                if (!world_to_screen(head, screenHead, viewMatrix))
                    continue;

                float height = screenOrigin.y - screenHead.y;
                float width = height / 2.0f;
                float x = screenHead.x - width / 2.0f;
                float y = screenHead.y;



                uint32_t flags = driver::read_memory<uint32_t>(
                    drv, dwLocalPlayerPawn + cs2_dumper::offsets::client_dll::m_fFlags);



                if (aimbot_enabled && is_key_down(aim_key)) {
                    aimbot::frame(aimbot_fov, aimbot_speed, bone_aiming, visibility_aim_check);
                }






                if (esp_enabled) {

                uint32_t team = driver::read_memory<uint32_t>(
                    drv, pawn + cs2_dumper::offsets::client_dll::m_iTeamNum);


                bool isVisible = driver::read_memory<bool>(
                    drv, pawn + cs2_dumper::offsets::client_dll::m_entitySpottedState +
                             cs2_dumper::offsets::client_dll::m_bSpottedByMask);

                ImColor color;
                if (team == localTeam) {
                    color = isVisible ? ImColor(color_team_visible) : ImColor(color_team_hidden);
                } else {
                    color = isVisible ? ImColor(color_enemy_visible) : ImColor(color_enemy_hidden);
                }

                float health_ratio = std::clamp(static_cast<float>(health) / 100.0f, 0.0f, 1.0f);

                ImColor health_color;
                if (health_ratio > 0.5f) {
                    float t = (health_ratio - 0.5f) * 2.0f;  
                    health_color = ImColor(static_cast<int>(255 * (1.0f - t)), 
                                           255,                                 
                                           0                                    
                    );
                } else {
                    float t = health_ratio * 2.0f;                     
                    health_color = ImColor(255,                        
                                           static_cast<int>(255 * t),  
                                           0                           
                    );
                }

                float bar_x = x - 6.0f;
                float bar_y = y;
                float bar_height = height;
                float bar_width = 4.0f;

                if (team == localTeam && !draw_teamate)
                    continue;


                ImGui::GetBackgroundDrawList()->AddRectFilled(
                    ImVec2(bar_x, bar_y), ImVec2(bar_x + bar_width, bar_y + bar_height),
                    IM_COL32(40, 40, 40, 200));

                // Barre de vie colorée
                ImGui::GetBackgroundDrawList()->AddRectFilled(
                    ImVec2(bar_x, bar_y + (1.0f - health_ratio) * bar_height),
                    ImVec2(bar_x + bar_width, bar_y + bar_height), health_color);

                // Contour (facultatif)
                ImGui::GetBackgroundDrawList()->AddRect(
                    ImVec2(bar_x - 1, bar_y - 1),
                    ImVec2(bar_x + bar_width + 1, bar_y + bar_height + 1), IM_COL32(0, 0, 0, 255));



                ImGui::GetBackgroundDrawList()->AddRect(ImVec2(x, y), ImVec2(x + width, y + height),
                                                        color, 0, 0, 2.0f);
                ImGui::GetBackgroundDrawList()->AddText(ImVec2(x, y - 16), color,
                                                        (team == localTeam) ? "Teammate" : "Enemy");

                for (int i = 0; i < boneConnectionsCount; ++i) {
                    int bone1 = boneConnections[i].bone1;
                    int bone2 = boneConnections[i].bone2;

                    Vector VectorBone1 = driver::read_memory<Vector>(drv, bonearray + bone1 * 0x20);
                    Vector VectorBone2 = driver::read_memory<Vector>(drv, bonearray + bone2 * 0x20);

                    Vector b1, b2;
                    if (world_to_screen(VectorBone1, b1, viewMatrix) &&
                        world_to_screen(VectorBone2, b2, viewMatrix)) {
                        ImGui::GetBackgroundDrawList()->AddLine(ImVec2(b1.x, b1.y),
                                                                ImVec2(b2.x, b2.y),
                                                                color,  // couleur ImU32
                                                                1.5f    // épaisseur
                        );
                    }
                }


                }

                found++;
            }
        }




        ImGui::EndFrame();
        ImGui::Render();
        ctx->OMSetRenderTargets(1, &rtv, nullptr);
        float clear_color[4] = {0, 0, 0, 0};
        ctx->ClearRenderTargetView(rtv, clear_color);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        sc->Present(1, 0);

        Sleep(1);
    }
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    rtv->Release();
    sc->Release();
    ctx->Release();
    dev->Release();
    DestroyWindow(hwnd);
    UnregisterClassW(wc.lpszClassName, hInst);

    return 0;
}