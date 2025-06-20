#include "aimbot.h"
#include <cfloat>
#include <string>
#include <cmath>
#include "../bones/bones.hpp"

static uintptr_t get_entity(uintptr_t ent_list, int index, HANDLE drv) {
    uintptr_t chunk = driver::read_memory<uintptr_t>(drv, ent_list + 0x8 * (index >> 9) + 0x10);
    if (!chunk)
        return 0;

    return driver::read_memory<uintptr_t>(drv, chunk + 0x78 * (index & 0x1FF));
}

float aimbot::distance(vec3 p1, vec3 p2) {
    return sqrtf((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y) +
                 (p1.z - p2.z) * (p1.z - p2.z));
}

void aimbot::frame(float maxFov, float smoothing, int bone_aiming, bool visibility_aim_check) {
    if (!driver_handle) {
        return;
    }

    uintptr_t ent_list = driver::read_memory<uintptr_t>(
        driver_handle, client_module + cs2_dumper::offsets::client_dll::dwEntityList);
    if (!ent_list) {
        return;
    }

    uintptr_t localPawn = driver::read_memory<uintptr_t>(
        driver_handle, client_module + cs2_dumper::offsets::client_dll::dwLocalPlayerPawn);
    if (!localPawn) {
        return;
    }

    uintptr_t localController = driver::read_memory<uintptr_t>(
        driver_handle, client_module + cs2_dumper::offsets::client_dll::dwLocalPlayerController);
    int localPlayerIndex = localController & 0xFFF;

    BYTE localTeam = driver::read_memory<BYTE>(
        driver_handle, localPawn + cs2_dumper::offsets::client_dll::m_iTeamNum);


    vec3 localEye =
        driver::read_memory<vec3>(driver_handle,
                                  localPawn + cs2_dumper::offsets::client_dll::m_vOldOrigin) +
        driver::read_memory<vec3>(driver_handle,
                                  localPawn + cs2_dumper::offsets::client_dll::m_vecViewOffset);

    float bestDist = FLT_MAX;
    vec3 bestPos = localEye;
    vec3 bestAngle = {};
    is_active = false;

    for (int i = 1; i < 64; ++i) {
        uintptr_t controller = get_entity(ent_list, i, driver_handle);
        if (!controller) {
            continue;
        }

        int pawnHandle = driver::read_memory<int>(
            driver_handle, controller + cs2_dumper::offsets::client_dll::m_hPlayerPawn);
        int pawnIdx = pawnHandle & 0x7FFF;
        if (!pawnIdx) {
            continue;
        }


        uintptr_t pawn = get_entity(ent_list, pawnIdx, driver_handle);
        if (!pawn) {
            continue;
        }
        BYTE team = driver::read_memory<BYTE>(driver_handle,
                                              pawn + cs2_dumper::offsets::client_dll::m_iTeamNum);
        
        if (pawn == localPawn) {
            continue;
        }


        if (team == localTeam) {
            continue;
        }
        

        int hp = driver::read_memory<int>(driver_handle,
                                          pawn + cs2_dumper::offsets::client_dll::m_iHealth);
        if (hp <= 0) {
            continue;
        }

        int spottedMask = driver::read_memory<int>(
            driver_handle, pawn + cs2_dumper::offsets::client_dll::m_entitySpottedState +
                               cs2_dumper::offsets::client_dll::m_bSpottedByMask);
        bool isVisible = (spottedMask & (1 << localPlayerIndex)) != 0;

        //if (visibility_aim_check && (!isVisible )) {
        //    continue;
        //}


        uintptr_t gamescene = driver::read_memory<uintptr_t>(
            driver_handle, pawn + cs2_dumper::offsets::client_dll::m_pGameSceneNode);
        uintptr_t bonearray = driver::read_memory<uintptr_t>(
            driver_handle, gamescene + cs2_dumper::offsets::client_dll::m_modelState + 0x80);

        vec3 head = driver::read_memory<vec3>(driver_handle, bonearray + bone_aiming * 32);

        vec3 angleTo = (head - localEye).RelativeAngle();
        vec3 currentAngle = driver::read_memory<vec3>(
            driver_handle, client_module + cs2_dumper::offsets::client_dll::dwViewAngles);

        float fov = hypotf(angleTo.x - currentAngle.x, angleTo.y - currentAngle.y);
        if (fov > maxFov) {
            continue;
        }

        float dist = distance(localEye, head);
        if (dist < bestDist) {
            bestDist = dist;
            bestPos = head;
            bestAngle = angleTo;
            is_active = true;
        }
    }

    if (!is_active) {
        return;
    }

    vec3 probe = driver::read_memory<vec3>(
        driver_handle, client_module + cs2_dumper::offsets::client_dll::dwViewAngles);
    bool looksLikeAngle = probe.x > -90.f && probe.x < 90.f && probe.y > -181.f && probe.y < 181.f;

    uintptr_t viewAddr =
        looksLikeAngle
            ? (client_module + cs2_dumper::offsets::client_dll::dwViewAngles)
            : driver::read_memory<uintptr_t>(
                  driver_handle, client_module + cs2_dumper::offsets::client_dll::dwViewAngles);

    vec3 currentAngle = driver::read_memory<vec3>(driver_handle, viewAddr);
    vec3 delta = bestAngle - currentAngle;
    delta.x /= smoothing;
    delta.y /= smoothing;

    vec3 smoothed = currentAngle + delta;
    driver::write_memory<vec3>(driver_handle, viewAddr, smoothed);

}
