#pragma once

#include "../offsets.hpp"
#include "../math/vector.h"
#include "../driver.h"
#include "../client_dll.hpp"
#include "../globals.h"
#include "string"



namespace aimbot {

	inline uintptr_t module_base;

	float distance(vec3 p1, vec3 p2);
    void frame(float maxFov, float smoothing, int bone_aiming, bool visibility_aim_check);
    inline bool is_active = false;
    inline std::string debug_status = "Inactive";

}