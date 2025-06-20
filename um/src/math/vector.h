#pragma once

#include <cmath>
#include <numbers>

struct vec4
{
	float w, x, y, z;
};


struct vec3 {
    float x, y, z;

    vec3 operator+(const vec3& other) const {
        return {x + other.x, y + other.y, z + other.z};
    }

    vec3 operator-(const vec3& other) const {
        return {x - other.x, y - other.y, z - other.z};
    }

    float distance_to(const vec3& other) const {
        float dx = x - other.x;
        float dy = y - other.y;
        float dz = z - other.z;
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }

    vec3 RelativeAngle() const {
        return {std::atan2(-z, std::hypot(x, y)) * (180.0f / std::numbers::pi_v<float>),
                std::atan2(y, x) * (180.0f / std::numbers::pi_v<float>), 0.0f};
    }

    float fov_to(const vec3& other) const {
        float deltaPitch = x - other.x;
        float deltaYaw = y - other.y;

        if (deltaYaw > 180.0f)
            deltaYaw -= 360.0f;
        if (deltaYaw < -180.0f)
            deltaYaw += 360.0f;

        return std::sqrt(deltaPitch * deltaPitch + deltaYaw * deltaYaw);
    }

    void clamp() {
        if (x > 89.0f)
            x = 89.0f;
        if (x < -89.0f)
            x = -89.0f;

        while (y > 180.0f)
            y -= 360.0f;
        while (y < -180.0f)
            y += 360.0f;

        z = 0.0f;
    }
};


struct vec2
{
	float x, y;
};

